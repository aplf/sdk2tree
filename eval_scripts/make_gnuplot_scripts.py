#!/usr/bin/env python3
__copyright__ = """ Copyright 2020 Miguel E. Coimbra

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. """
__license__ = "Apache 2.0"

###########################################################################
################################# IMPORTS #################################
###########################################################################

import argparse
from argparse import ArgumentError
import copy
import errno
import math
import os
import pprint
import shutil

import sys

from typing import Dict, List, Tuple, Set

import xlsxwriter
from xlsxwriter.utility import xl_rowcol_to_cell
from xlsxwriter import Workbook
from xlsxwriter.worksheet import Worksheet


###########################################################################
################################# GLOBALS #################################
###########################################################################


DMGEN_ORDER: List[str] = ["50000", "100000", "500000", "1000000"]
WEBGRAPH_ORDER: List[str] = ["uk-2007-05@100000", "in-2004",
                             "uk-2014-host", "indochina-2004", "eu-2015-host"]

TSV_SPACE_KEY: str = "tab_separated_space_MB"
SDK2SDSL_SERIALIZED_KT_BITS_KEY: str = "sdk2sdsl_kt_space_bits"
V_COUNT_KEY: str = "|V|"
E_COUNT_KEY: str = "|E|"

DATASET_SIZES = {

    # "TSV_SPACE_KEY" is stored as (BYTES / 1024 / 1024), so it's in MB.
    # SDK2SDSL_SERIALIZED_KT_BITS is stored as (KB * 1024 * 8), so it's bits.

    # Duplication model.
    "50000": {V_COUNT_KEY: 50000, E_COUNT_KEY: 1111410, TSV_SPACE_KEY: 14361658 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 3440 * 1024 * 8},
    "100000": {V_COUNT_KEY: 100000, E_COUNT_KEY: 2593718, TSV_SPACE_KEY: 34395738 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 8616 * 1024 * 8},
    "500000": {V_COUNT_KEY: 500000, E_COUNT_KEY: 11978890, TSV_SPACE_KEY: 178552464 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 48720 * 1024 * 8},
    "1000000": {V_COUNT_KEY: 1000000, E_COUNT_KEY: 27416218, TSV_SPACE_KEY: 418018424 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 118252 * 1024 * 8},

    # Web graphs.
    "uk-2007-05@100000": {V_COUNT_KEY: 100000, E_COUNT_KEY: 3050615, TSV_SPACE_KEY: 41891011 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 1368 * 1024 * 8},
    "in-2004": {V_COUNT_KEY: 1382908, E_COUNT_KEY: 16917053, TSV_SPACE_KEY: 281271862 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 7532 * 1024 * 8},
    "uk-2014-host": {V_COUNT_KEY: 4769354, E_COUNT_KEY: 50829923, TSV_SPACE_KEY: 903075684 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 70856 * 1024 * 8},
    "indochina-2004": {V_COUNT_KEY: 7414866, E_COUNT_KEY: 194109311, TSV_SPACE_KEY: 3461087649 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 71144 * 1024 * 8},
    "eu-2015-host": {V_COUNT_KEY: 11264052, E_COUNT_KEY: 386915963, TSV_SPACE_KEY: 6724801466 / 1024 / 1024, SDK2SDSL_SERIALIZED_KT_BITS_KEY: 318556 * 1024 * 8}
}

SKIP_FIELDS: Set[str] = set(DATASET_SIZES["50000"].keys())

RUN_AVG_STR: str = "run-avg"

CURR_SCRIPT_DIR: str = os.path.realpath(__file__)
CURR_SCRIPT_DIR = CURR_SCRIPT_DIR[:CURR_SCRIPT_DIR.rfind(os.path.sep)]

IMPLEMENTATION_NAMES: List[str] = [
    "dk2tree", "sdk2sdsl", "sdk2tree", "k2trie-1", "k2trie-2", "k2tree"]

###########################################################################
################################ FUNCTIONS ################################
###########################################################################


def put_case_stats(case_data: Dict) -> None:
    """Get the statistics from the k2tree test and puts them in case_data["stats"]."""

    # Holder for the time and memory stats of the current batch.
    stats: Dict = {}

    if len(case_data["files"]) == 0:
        return

    time_file_list: str = [
        f for f in case_data["files"] if f.endswith("-time.tsv")]

    stats["has_time"] = False
    if len(time_file_list) > 0:
        stats["has_time"] = True

        time_file_path: str = time_file_list[0]

        with open(time_file_path, 'r') as t_file:
            lines: List[str] = t_file.readlines()
            headers: List[str] = lines[0].strip().split(";")

            row_vals: Dict[str, float] = {}

            values: List = lines[1].strip().split(";")
            for i in range(0, len(values)):
                if "." in values[i]:
                    row_vals[headers[i]] = float(values[i])
                else:
                    row_vals[headers[i]] = int(values[i])
                if row_vals[headers[i]] > 0 or headers[i] in (("exclusive_save_time")) or headers[i].endswith("_op"):
                    stats[headers[i]] = row_vals[headers[i]]

    memory_file_list: str = [
        f for f in case_data["files"] if f.endswith("-memory.txt")]

    # Check if we have memory information for this execution batch.
    stats["has_memory"] = False

    # If we don't have time available, it means the program exited abnormally, so we will ignore memory even if present.
    if stats["has_time"] and len(memory_file_list) > 0:
        stats["has_memory"] = True

        memory_file_path: str = memory_file_list[0]

        with open(memory_file_path, 'r') as m_file:
            for l in m_file:
                stripped: str = l.strip()
                if "Maximum resident set size (kbytes): " in stripped:
                    tkns: List[str] = stripped.split(": ")
                    max_resident_set_size_kbytes: int = int(tkns[1])
                    stats["max_resident_set_size_kbytes"] = max_resident_set_size_kbytes

    # Get volume
    stats["has_serialization"] = False
    kt_file_list: str = [f for f in case_data["files"] if f.endswith("kt-A")]
    if len(kt_file_list) > 0:
        stats["has_serialization"] = True
        kt_A_path: str = kt_file_list[0]
        impl_kt_sz: int = sum([os.path.getsize(os.path.join(kt_A_path, f)) for f in os.listdir(
            kt_A_path) if os.path.isfile(os.path.join(kt_A_path, f))])
        stats["kt_size_B"] = impl_kt_sz

    case_data["stats"] = stats


def get_result_file_hierarchy(args: argparse.ArgumentParser, data_dirs: List[str]) -> Dict[str, Dict[str, Dict[str, Dict[str, List[str]]]]]:
    """
    Read the k2tree results hierarchy and return it in nested dictionaries.

    Parameters
    ----------
    data_dirs : List[str]
        The list of files in the given batch of k2tree experiment files.

    Returns
    -------
    run_dirs: Dict[str, Dict[str, Dict[str, Dict[str, List[str]]]]]
        A hierarchical dictionary, from highest to lowest: k2tree runs directory, dataset directory, implementation directory and test case.
    """

    run_dirs: Dict[str, Dict[str, Dict[str, Dict[str, List[str]]]]] = {}
    run_dirs[RUN_AVG_STR] = {}

    for r_dir in data_dirs:

        print("> {}".format(r_dir))
        r_dir_name: str = r_dir[r_dir.rfind(os.path.sep) + 1:]
        run_dirs[r_dir_name] = {}

        if os.path.isdir(r_dir):

            dataset_dirs: List[str] = [os.path.join(r_dir, f) for f in os.listdir(
                r_dir) if os.path.isdir(os.path.join(r_dir, f))]
            for d_dir in dataset_dirs:

                d_dir_name: str = d_dir[d_dir.rfind(os.path.sep) + 1:]

                # if d_dir_name in args.skip:
                #    continue

                print(">> {}".format(d_dir))

                run_dirs[r_dir_name][d_dir_name] = {}

                if d_dir_name not in run_dirs[RUN_AVG_STR]:
                    run_dirs[RUN_AVG_STR][d_dir_name] = {}

                if os.path.isdir(d_dir):

                    k2tree_impl_dirs: List[str] = [os.path.join(d_dir, f) for f in os.listdir(
                        d_dir) if os.path.isdir(os.path.join(d_dir, f))]

                    for k2tree_dir in k2tree_impl_dirs:

                        print(">>> {}".format(k2tree_dir))

                        k2tree_dir_name: str = k2tree_dir[k2tree_dir.rfind(
                            os.path.sep) + 1:]

                        test_case_entries: List[str] = [os.path.join(
                            k2tree_dir, f) for f in os.listdir(k2tree_dir)]

                        case_A_data: Dict = {"files": []}
                        case_AD_data: Dict = {"files": []}
                        case_AC_data: Dict = {"files": []}
                        case_AL_data: Dict = {"files": []}

                        case_ALGO_BFS_data: Dict = {"files": []}
                        case_ALGO_DFS_data: Dict = {"files": []}
                        case_ALGO_PR_data: Dict = {"files": []}
                        case_ALGO_CC_data: Dict = {"files": []}
                        case_ALGO_CT_dummy_hash_data: Dict = {"files": []}

                        case_ALGO_CT_dummy_data: Dict = {"files": []}

                        for dir_entry in test_case_entries:
                            if "-A-" in dir_entry or "-A." in dir_entry or dir_entry.endswith("kt-A"):
                                case_A_data["files"].append(dir_entry)
                            elif "-AD-" in dir_entry or "-AD." in dir_entry or dir_entry.endswith("kt-AD"):
                                case_AD_data["files"].append(dir_entry)
                            elif "-AC-" in dir_entry or "-AC." in dir_entry or dir_entry.endswith("kt-AC"):
                                case_AC_data["files"].append(dir_entry)
                            elif "-AL-" in dir_entry or "-AL." in dir_entry or dir_entry.endswith("kt-AL"):
                                case_AL_data["files"].append(dir_entry)
                            elif "ALGO" in dir_entry in dir_entry:
                                if "ALGO-BFS" in dir_entry:
                                    case_ALGO_BFS_data["files"].append(
                                        dir_entry)
                                elif "ALGO-DFS" in dir_entry:
                                    case_ALGO_DFS_data["files"].append(
                                        dir_entry)
                                elif "ALGO-PR" in dir_entry:
                                    case_ALGO_PR_data["files"].append(
                                        dir_entry)
                                elif "ALGO-CC" in dir_entry:
                                    case_ALGO_CC_data["files"].append(
                                        dir_entry)
                                elif "ALGO-CT_dummy_hash" in dir_entry:
                                    case_ALGO_CT_dummy_hash_data["files"].append(
                                        dir_entry)
                                elif "ALGO-CT_dummy" in dir_entry:
                                    case_ALGO_CT_dummy_data["files"].append(
                                        dir_entry)

                        if d_dir_name not in DATASET_SIZES:
                            raise RuntimeError(
                                "Found an unknown dataset: {}. Make sure the dataset's vertex count |V| and edge count |E| are present in 'DATASET_SIZES'".format(d_dir_name))

                        case_files: Dict[str] = {}
                        case_files.update(DATASET_SIZES[d_dir_name])

                        # TODO: for k2tree implementation, for each AC case, get start_time / |E| to get average time per ingested edge and save that as A so we can see the k2tree additions as well.

                        if len(case_A_data) > 0:
                            case_files["A"] = case_A_data
                            put_case_stats(case_A_data)
                        if len(case_AD_data) > 0:
                            case_files["AD"] = case_AD_data
                            put_case_stats(case_AD_data)
                        if len(case_AC_data) > 0:
                            case_files["AC"] = case_AC_data
                            put_case_stats(case_AC_data)

                            # Create "A" operation data for the static k2tree.
                            # Use start_time / |E|.
                            if k2tree_dir_name == "k2tree":
                                stats: Dict = {
                                    "has_time": True,
                                    "has_memory": False,
                                    "start_time": case_files["AC"]["stats"]["start_time"],
                                    "loop_time": case_files["AC"]["stats"]["loop_time"],
                                    "add_op_count": DATASET_SIZES[d_dir_name][E_COUNT_KEY]
                                }
                                stats["add_op_exclusive_time"] = stats["start_time"] / \
                                    stats["add_op_count"]

                                # pprint.pprint(case_files["AC"])

                                case_A_data["stats"] = stats
                                case_files["A"] = case_A_data

                                # pprint.pprint(case_files["A"])

                                # sys.exit(0)

                        if len(case_AL_data) > 0:
                            case_files["AL"] = case_AL_data
                            put_case_stats(case_AL_data)

                        if len(case_ALGO_BFS_data) > 0:
                            case_files["ALGO_BFS"] = case_ALGO_BFS_data
                            put_case_stats(case_ALGO_BFS_data)
                        if len(case_ALGO_DFS_data) > 0:
                            case_files["ALGO_DFS"] = case_ALGO_DFS_data
                            put_case_stats(case_ALGO_DFS_data)
                        if len(case_ALGO_PR_data) > 0:
                            case_files["ALGO_PR"] = case_ALGO_PR_data
                            put_case_stats(case_ALGO_PR_data)
                        if len(case_ALGO_CC_data) > 0:
                            case_files["ALGO_CC"] = case_ALGO_CC_data
                            put_case_stats(case_ALGO_CC_data)
                        if len(case_ALGO_CT_dummy_hash_data) > 0:
                            case_files["ALGO_CT_dummy_hash"] = case_ALGO_CT_dummy_hash_data
                            put_case_stats(case_ALGO_CT_dummy_hash_data)
                        if len(case_ALGO_CT_dummy_data) > 0:
                            case_files["ALGO_CT_dummy"] = case_ALGO_CT_dummy_data
                            put_case_stats(case_ALGO_CT_dummy_data)

                        # Store {run-3: {eu-2015-host: {k2sdsl: {...case_files...}}}}
                        run_dirs[r_dir_name][d_dir_name][k2tree_dir_name] = case_files

                        # Sum the values for the average run result.
                        if k2tree_dir_name not in run_dirs[RUN_AVG_STR][d_dir_name]:
                            deep_copy: Dict = copy.deepcopy(case_files)
                            for k2tree_test, test_value_dict in deep_copy.items():
                                # if k2tree_test != V_COUNT_KEY and k2tree_test != E_COUNT_KEY:
                                if k2tree_test not in SKIP_FIELDS:
                                    print("{}\t{}".format(
                                        k2tree_test, test_value_dict))
                                    test_value_dict["files"].clear()
                            run_dirs[RUN_AVG_STR][d_dir_name][k2tree_dir_name] = deep_copy
                        else:
                            avg_case_files = run_dirs[RUN_AVG_STR][d_dir_name][k2tree_dir_name]
                            for k2tree_test, test_value_dict in case_files.items():
                                # if k2tree_test != V_COUNT_KEY and k2tree_test != E_COUNT_KEY and k2tree_test in avg_case_files and "stats" in test_value_dict:
                                if (k2tree_test not in SKIP_FIELDS) and k2tree_test in avg_case_files and "stats" in test_value_dict:
                                    for stat_name, stat_val in test_value_dict["stats"].items():
                                        if not isinstance(stat_val, bool):
                                            # pprint.pprint(avg_case_files[k2tree_test]["stats"])
                                            if stat_name not in avg_case_files[k2tree_test]["stats"]:
                                                print(
                                                    "Missing stat:\t{}".format(stat_name))
                                                print(
                                                    "################# A ###############")
                                                pprint.pprint(case_A_data)
                                                print(
                                                    "################# AC ###############")
                                                pprint.pprint(case_AC_data)
                                                print(
                                                    "################# test_value_dict ###############")
                                                pprint.pprint(test_value_dict)
                                                print(
                                                    "################# avg_case_files ###############")
                                                # pprint.pprint(avg_case_files[k2tree_test]["stats"])
                                                print("#############> TEST: {}".format(
                                                    k2tree_test))
                                                pprint.pprint(avg_case_files)
                                                exit(0)
                                            avg_case_files[k2tree_test]["stats"][stat_name] = avg_case_files[
                                                k2tree_test]["stats"][stat_name] + stat_val

    # Normalize avg-run values.
    run_dir_count: int = len(run_dirs.keys()) - 1
    if run_dir_count == 0:
        run_dir_count = 1
    for d_dir_name in run_dirs[RUN_AVG_STR].keys():
        for k2tree_dir_name in run_dirs[RUN_AVG_STR][d_dir_name].keys():
            curr_stats: Dict = run_dirs[RUN_AVG_STR][d_dir_name][k2tree_dir_name]
            for k2tree_test, test_value_dict in curr_stats.items():

                if (k2tree_test not in SKIP_FIELDS) and "stats" in test_value_dict:
                    # if k2tree_test != V_COUNT_KEY and k2tree_test != E_COUNT_KEY and "stats" in test_value_dict:
                    for stat_name in test_value_dict["stats"].keys():
                        stat_val = test_value_dict["stats"][stat_name]
                        if not isinstance(stat_val, bool):
                            if isinstance(stat_val, int):
                                test_value_dict["stats"][stat_name] = int(
                                    stat_val / run_dir_count)
                            elif isinstance(stat_val, float):
                                test_value_dict["stats"][stat_name] = stat_val / \
                                    run_dir_count

    return run_dirs


def copyanything(src, dst):
    """
        Make a recursive copy of a directory.
    """
    try:
        shutil.copytree(src, dst)
    except OSError as exc:  # python >2.5
        if exc.errno == errno.ENOTDIR:
            shutil.copy(src, dst)
        else:
            raise


def normalize_k2trie(args: argparse.ArgumentParser, data_dirs: List[str]) -> None:

    run_dirs: Dict[str, Dict[str, Dict[str, Dict[str, List[str]]]]] = {}

    for r_dir in data_dirs:

        #print("> {}".format(r_dir))

        r_dir_name: str = r_dir[r_dir.rfind(os.path.sep) + 1:]
        run_dirs[r_dir_name] = {}

        if os.path.isdir(r_dir):

            dataset_dirs: List[str] = [os.path.join(r_dir, f) for f in os.listdir(
                r_dir) if os.path.isdir(os.path.join(r_dir, f))]

            for d_dir in dataset_dirs:

                #print(">> {}".format(d_dir))

                d_dir_name: str = d_dir[d_dir.rfind(os.path.sep) + 1:]
                run_dirs[r_dir_name][d_dir_name] = {}

                if os.path.isdir(d_dir):

                    k2tree_impl_dirs: List[str] = [os.path.join(d_dir, f) for f in os.listdir(
                        d_dir) if f == "k2trie" and os.path.isdir(os.path.join(d_dir, f))]

                    for k2trie_dir in k2tree_impl_dirs:

                        print(">>> {}".format(k2trie_dir))

                        k2trie_dir_name: str = k2trie_dir[k2trie_dir.rfind(
                            os.path.sep) + 1:]

                        test_case_entries: List[str] = [os.path.join(
                            k2trie_dir, f) for f in os.listdir(k2trie_dir)]

                        # Create "k2trie-2" dir in 'd_dir'
                        k2trie_2_dir: str = os.path.join(d_dir, "k2trie-2")
                        if not os.path.exists(k2trie_2_dir):
                            os.makedirs(k2trie_2_dir)

                        for f in test_case_entries:
                            fbase: str = f[f.rfind(os.path.sep) + 1:]
                            if "k2trie_2-" in fbase:
                                # Move files with "k2trie_2" to "k2trie-2" dir.
                                k2trie_2_path: str = os.path.join(
                                    k2trie_2_dir, fbase)
                                os.rename(f, k2trie_2_path)
                            elif fbase.endswith("-A") or fbase.endswith("-AC"):
                                k2trie_2_path: str = os.path.join(
                                    k2trie_2_dir, "kt" + fbase.replace("k2trie", ""))
                                # Copy dir "kt-A" and "kt-AC" to "k2trie-2/kt-2-A" and "k2trie-2/kt-2-AC".
                                copyanything(f, k2trie_2_path)

                                # Rename dir kt-A" to "kt-1-A" and dir kt-AC" to "kt-1-AC"
                                k2trie_1_rename_path: str = os.path.join(
                                    k2trie_dir, "kt" + fbase.replace("k2trie", ""))
                                os.rename(f, k2trie_1_rename_path)

                        # Rename "k2trie" dir into "k2trie-1".
                        os.rename(k2trie_dir, k2trie_dir + "-1")

                        # for k2trie_file in test_case_entries:
                        #    print("\t{}".format(k2trie_file))


def rename_implementation_files(args: argparse.ArgumentParser, data_dirs: List[str], impl_changes: Dict[str, str]) -> None:

    run_dirs: Dict[str, Dict[str, Dict[str, Dict[str, List[str]]]]] = {}

    for r_dir in data_dirs:

        r_dir_name: str = r_dir[r_dir.rfind(os.path.sep) + 1:]
        run_dirs[r_dir_name] = {}

        if os.path.isdir(r_dir):

            dataset_dirs: List[str] = [os.path.join(r_dir, f) for f in os.listdir(
                r_dir) if os.path.isdir(os.path.join(r_dir, f))]

            for d_dir in dataset_dirs:

                d_dir_name: str = d_dir[d_dir.rfind(os.path.sep) + 1:]
                run_dirs[r_dir_name][d_dir_name] = {}

                if os.path.isdir(d_dir):

                    k2tree_impl_dirs: List[str] = [os.path.join(d_dir, f) for f in os.listdir(
                        d_dir) if f in impl_changes and os.path.isdir(os.path.join(d_dir, f))]

                    for curr_imple_dir in k2tree_impl_dirs:

                        print("\n> Processing:\t{}".format(curr_imple_dir))
                        print("> Renaming files first.")

                        test_case_entries: List[str] = [os.path.join(
                            curr_imple_dir, f) for f in os.listdir(curr_imple_dir)]

                        old_impl_name: str = ""
                        new_impl_name: str = ""
                        for impl in impl_changes.keys():
                            if impl in curr_imple_dir:
                                old_impl_name = impl
                                new_impl_name = impl_changes[impl]
                                break

                        for f in test_case_entries:
                            fbase: str = f[f.rfind(os.path.sep) + 1:]

                            separate_keyword: str = "-{}-".format(
                                old_impl_name)

                            if not separate_keyword in fbase:
                                continue

                            new_path: str = os.path.join(
                                curr_imple_dir, fbase.replace(old_impl_name, new_impl_name))

                            #print("\tOld base:\t{}".format(fbase))
                            print("> Renaming file:")
                            print("\tOld:\t{}".format(f))
                            print("\tNew:\t{}\n".format(new_path))

                            os.rename(f, new_path)

                        print("> Renaming files finished.")

                        # Rename the directory.
                        dir_path: str = curr_imple_dir[: curr_imple_dir.rfind(
                            os.path.sep)]
                        old_dir_base: str = curr_imple_dir[curr_imple_dir.rfind(
                            os.path.sep) + 1:]
                        new_dir_path: str = os.path.join(
                            dir_path, new_impl_name)

                        print("\n> Renaming directory:")
                        print("\tOld:\t{}".format(curr_imple_dir))
                        print("\tNew:\t{}".format(new_dir_path))

                        os.rename(curr_imple_dir, new_dir_path)


def normalize_k2trie_2(args: argparse.ArgumentParser, data_dirs: List[str]) -> None:

    run_dirs: Dict[str, Dict[str, Dict[str, Dict[str, List[str]]]]] = {}

    for r_dir in data_dirs:

        #print("> {}".format(r_dir))

        r_dir_name: str = r_dir[r_dir.rfind(os.path.sep) + 1:]
        run_dirs[r_dir_name] = {}

        if os.path.isdir(r_dir):

            dataset_dirs: List[str] = [os.path.join(r_dir, f) for f in os.listdir(
                r_dir) if os.path.isdir(os.path.join(r_dir, f))]

            for d_dir in dataset_dirs:

                #print(">> {}".format(d_dir))

                d_dir_name: str = d_dir[d_dir.rfind(os.path.sep) + 1:]
                run_dirs[r_dir_name][d_dir_name] = {}

                if os.path.isdir(d_dir):

                    k2tree_impl_dirs: List[str] = [os.path.join(d_dir, f) for f in os.listdir(
                        d_dir) if f.startswith("k2trie") and os.path.isdir(os.path.join(d_dir, f))]

                    for k2trie_dir in k2tree_impl_dirs:

                        print(">>> {}".format(k2trie_dir))

                        k2trie_dir_name: str = k2trie_dir[k2trie_dir.rfind(
                            os.path.sep) + 1:]

                        test_case_entries: List[str] = [os.path.join(
                            k2trie_dir, f) for f in os.listdir(k2trie_dir)]

                        for f in test_case_entries:
                            fbase: str = f[f.rfind(os.path.sep) + 1:]
                            if fbase.endswith("-A") or fbase.endswith("-AC"):
                                # Rename dir kt-A" to "kt-1-A" and dir kt-AC" to "kt-1-AC"
                                new_path: str = os.path.join(
                                    k2trie_dir, "kt-" + fbase[5:])

                                print("old_name:\t{}".format(f))
                                print("new_name:\t{}\n\n".format(new_path))
                                os.rename(f, new_path)


def print_impl_x_case_to_tsv(args: argparse.ArgumentParser, dataset_type: str, k2tree_implementation_name: str, k2tree_case: str, data_dict: Dict, run_out_dir: str):

    out_file_base: str = "{}-{}-{}.tsv".format(
        k2tree_implementation_name, k2tree_case, dataset_type)
    out_file_path: str = os.path.join(run_out_dir, out_file_base)
    with open(out_file_path, 'w') as f:

        if k2tree_case == "A":
            if k2tree_implementation_name == "k2tree":
                f.write("|V|\t|E|\tloop_time\tadd_op_count\tadd_op_exclusive_time\n")
            else:
                f.write(
                    "|V|\t|E|\tloop_time\tmax_resident_set_size_kbytes\tadd_op_count\tadd_op_exclusive_time\n")
            specific_op_fields = []
        elif k2tree_case == "AD":
            f.write("|V|\t|E|\tloop_time\tmax_resident_set_size_kbytes\tadd_op_count\tadd_op_exclusive_time\trem_op_count\trem_op_exclusive_time\n")
            specific_op_fields = ["rem_op_count", "rem_op_exclusive_time"]
        elif k2tree_case == "AC":
            if k2tree_implementation_name == "k2tree":
                f.write(
                    "|V|\t|E|\tloop_time\tmax_resident_set_size_kbytes\tcheck_op_count\tcheck_op_exclusive_time\n")
            else:
                f.write("|V|\t|E|\tloop_time\tmax_resident_set_size_kbytes\tadd_op_count\tadd_op_exclusive_time\tcheck_op_count\tcheck_op_exclusive_time\n")
            specific_op_fields = ["check_op_count", "check_op_exclusive_time"]
        elif k2tree_case == "AL":
            if k2tree_implementation_name == "k2tree":
                f.write(
                    "|V|\t|E|\tloop_time\tmax_resident_set_size_kbytes\tlist_op_count\tlist_op_exclusive_time\n")
            else:
                f.write(
                    "|V|\t|E|\tloop_time\tmax_resident_set_size_kbytes\tadd_op_count\tadd_op_exclusive_time\tlist_op_count\tlist_op_exclusive_time\n")
            specific_op_fields = ["list_op_count", "list_op_exclusive_time"]
        elif "ALGO" in k2tree_case:
            f.write(
                "|V|\t|E|\tloop_time\tmax_resident_set_size_kbytes\tadd_op_count\tadd_op_exclusive_time\texclusive_algo_time\n")
            specific_op_fields = ["exclusive_algo_time"]
        else:
            raise ArgumentError(
                "'k2tree_case' must be one of 'A', 'AD', 'AC', 'AL' or must be an 'ALGO' variant.")

        if dataset_type == "dmgen":
            target_ds_list = []
            for dmgen_ds in DMGEN_ORDER:
                if dmgen_ds not in args.skip:
                    target_ds_list.append(("dmgen", dmgen_ds))
        elif dataset_type == "webgraph":
            target_ds_list = []
            for webgraph_ds in WEBGRAPH_ORDER:
                if webgraph_ds not in args.skip and ("ALGO" not in k2tree_case):
                    target_ds_list.append(("webgraph", webgraph_ds))
        elif dataset_type == "all-datasets":
            target_ds_list = []
            for dmgen_ds in DMGEN_ORDER:
                if dmgen_ds not in args.skip:
                    target_ds_list.append(("dmgen", dmgen_ds))
            for webgraph_ds in WEBGRAPH_ORDER:
                if webgraph_ds not in args.skip and ("ALGO" not in k2tree_case):
                    target_ds_list.append(("webgraph", webgraph_ds))
        else:
            raise ArgumentError(
                "'dataset_type' must be one of 'dmgen', 'webgraph' or 'all-datasets'.")

        for dataset_type, ds in target_ds_list:

            if ds not in data_dict[dataset_type]:
                continue

            if ds in args.skip:
                continue

            stats: Dict = data_dict[dataset_type][ds]["stats"]

            print("{}/{}/{}/{}".format(run_out_dir,
                                       k2tree_implementation_name, ds, k2tree_case))

            if (not stats["has_memory"]) and (not stats["has_time"]):
                continue

            specific_op_str = ""
            if len(specific_op_fields) > 0:
                specific_op_vals = []
                for sof in specific_op_fields:
                    if sof in stats:
                        specific_op_vals.append(str(stats[sof]))
                    else:
                        continue

                specific_op_str = "\t" + "\t".join(specific_op_vals)

            # if k2tree_implementation_name == "k2tree":
            #     if "loop_time" in stats:
            #         target_str = "{}\t{}\t{}\t{}{}\n".format(data_dict[dataset_type][ds][V_COUNT_KEY], data_dict[dataset_type][ds][E_COUNT_KEY], stats["loop_time"], stats["max_resident_set_size_kbytes"], specific_op_str)
            #     else:
            #         continue
            # else:
            #     target_str = "{}\t{}\t{}\t{}\t{}\t{}{}\n".format(data_dict[dataset_type][ds][V_COUNT_KEY], data_dict[dataset_type][ds][E_COUNT_KEY], stats["loop_time"], stats["max_resident_set_size_kbytes"], stats["add_op_count"], stats["add_op_exclusive_time"], specific_op_str)

            if k2tree_implementation_name == "k2tree":
                if k2tree_case == "A":
                    target_str = "{}\t{}\t{}\t{}\t{}{}\n".format(data_dict[dataset_type][ds][V_COUNT_KEY], data_dict[dataset_type]
                                                                 [ds][E_COUNT_KEY], stats["loop_time"], stats["add_op_count"], stats["add_op_exclusive_time"], specific_op_str)

                # elif "loop_time" in stats:
                else:
                    target_str = "{}\t{}\t{}\t{}{}\n".format(data_dict[dataset_type][ds][V_COUNT_KEY], data_dict[dataset_type]
                                                             [ds][E_COUNT_KEY], stats["loop_time"], stats["max_resident_set_size_kbytes"], specific_op_str)
                # else:
                #     pprint.pprint(stats)
                #     sys.exit(0)
                #     continue
            else:
                target_str = "{}\t{}\t{}\t{}\t{}\t{}{}\n".format(data_dict[dataset_type][ds][V_COUNT_KEY], data_dict[dataset_type][ds][E_COUNT_KEY],
                                                                 stats["loop_time"], stats["max_resident_set_size_kbytes"], stats["add_op_count"], stats["add_op_exclusive_time"], specific_op_str)
            f.write(target_str)


def print_dataset_type_values(d_dir_name: str, impl_result_values: List, implementation_order: str, stat_type: str, case_to_stat_divisor_name_map: Dict[str, str], k2tree_case: str) -> str:
    dataset_values_per_implementation: str = d_dir_name + "\t" + \
        str(impl_result_values["sdk2sdsl"][V_COUNT_KEY]) + \
        "\t" + str(impl_result_values["sdk2sdsl"][E_COUNT_KEY])

    # Iterate implementations: dk2tree, k2sdsl, k2tree, k2trie-1, k2trie-2, sdk2tree
    for impl_name in implementation_order:
        if impl_name in impl_result_values:
            print("########### V {} E {}".format(
                impl_result_values[impl_name][V_COUNT_KEY], impl_result_values[impl_name][E_COUNT_KEY]))
            print("########### {}/{}/{}".format(k2tree_case, d_dir_name, impl_name))
            implementation_stats: Dict = impl_result_values[impl_name]["stats"]

            if impl_name == "k2tree" and stat_name not in implementation_stats:
                continue

            if not implementation_stats["has_" + stat_type]:
                continue

            if impl_name == "k2tree" and k2tree_case == "A" and stat_type == "time":
                dataset_values_per_implementation = dataset_values_per_implementation + \
                    "\t" + str(implementation_stats[stat_name])
            elif "algo" in stat_name or stat_type == "memory":
                dataset_values_per_implementation = dataset_values_per_implementation + \
                    "\t" + str(implementation_stats[stat_name])
            else:
                divisor_name: str = case_to_stat_divisor_name_map[k2tree_case]
                print(stat_type)
                pprint.pprint(implementation_stats)
                pprint.pprint(divisor_name)
                dataset_values_per_implementation = dataset_values_per_implementation + "\t" + \
                    str(implementation_stats[stat_name] /
                        implementation_stats[divisor_name])

    return dataset_values_per_implementation


def print_impls_x_datasets_for_dataset_type(global_stats_lines: Dict[str, List[str]], test_case_x_impl_x_dataset: Dict, k2tree_case: str, dataset_type: str, stat_type: str, implementation_order: List, case_to_stat_divisor_name_map: Dict[str, str]):

    if "ALGO" in k2tree_case:

        if k2tree_case == "ALGO_CT_dummy" and (dataset_type in (("all", "webgraph"))):
            return

        #base_name: str = "k2sdsl-{}-{}-{}.tsv".format(k2tree_case, stat_type, dataset_type)
        base_name: str = "sdk2sdsl-{}-{}-{}.tsv".format(
            k2tree_case, stat_type, dataset_type)
    else:
        base_name: str = "all-impls-{}-{}-{}.tsv".format(
            k2tree_case, stat_type, dataset_type)

    out_path: str = os.path.join(run_out_dir, base_name)
    with open(out_path, 'w') as out:

        # Check which implementation effectively has results.
        present_headers: Set = set()
        if dataset_type == "all":
            if "dmgen" in test_case_x_impl_x_dataset[k2tree_case]:
                for d_dir_name in test_case_x_impl_x_dataset[k2tree_case]["dmgen"]:
                    for impl_name in test_case_x_impl_x_dataset[k2tree_case]["dmgen"][d_dir_name]:
                        present_headers.add(impl_name)
            if "webgraph" in test_case_x_impl_x_dataset[k2tree_case]:
                for d_dir_name in test_case_x_impl_x_dataset[k2tree_case]["webgraph"]:
                    if "ALGO" in k2tree_case and d_dir_name in args.skip:  # to skip indochina-2004
                        continue
                    for impl_name in test_case_x_impl_x_dataset[k2tree_case]["webgraph"][d_dir_name]:
                        present_headers.add(impl_name)
        else:
            if dataset_type in test_case_x_impl_x_dataset[k2tree_case]:
                for d_dir_name in test_case_x_impl_x_dataset[k2tree_case][dataset_type]:
                    if "ALGO" in k2tree_case and d_dir_name in args.skip:  # to skip indochina-2004
                        continue
                    for impl_name in test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name]:
                        present_headers.add(impl_name)

        # Move 'k2trie-1', 'k2trie-2 and 'k2tree' to the end, as they are not present in all test cases.
        header_order: List = sorted(present_headers)
        if "k2trie-1" in header_order:
            header_order.remove("k2trie-1")
            header_order.append("k2trie-1")
        if "k2trie-2" in header_order:
            header_order.remove("k2trie-2")
            header_order.append("k2trie-2")
        if "k2tree" in header_order:
            header_order.remove("k2tree")
            header_order.append("k2tree")

        tsv_header: str = "# ds/impl\t|V|\t|E|\t{}".format(
            "\t".join(header_order))

        out.write("{}\n".format(tsv_header))

        if dataset_type == "all":
            # Outputing both dataset types to .the tsv file.
            for dataset_type in (("dmgen", "webgraph")):

                # Order datasets of type 'dataset_type' by increasing number of vertices.
                v_count_to_ds: Dict[int, str] = {}
                if dataset_type in test_case_x_impl_x_dataset[k2tree_case]:
                    for d_dir_name in test_case_x_impl_x_dataset[k2tree_case][dataset_type]:

                        if "ALGO" in k2tree_case and d_dir_name in args.skip:
                            continue

                        # Get the lowest and highest values

                        # All implementations have the same |V| and |E| fields. Get the |V| from the first one we find.
                        first_impl_key: str = list(
                            test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name].keys())[0]
                        d_v_count: int = test_case_x_impl_x_dataset[k2tree_case][
                            dataset_type][d_dir_name][first_impl_key][V_COUNT_KEY]

                        v_count_to_ds[d_v_count] = d_dir_name
                ordered_v_count: List[int] = sorted(list(v_count_to_ds.keys()))

                for curr_v_count in ordered_v_count:
                    d_dir_name: str = v_count_to_ds[curr_v_count]
                    impl_result_values = test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name]
                    dataset_values_per_implementation: str = print_dataset_type_values(
                        d_dir_name, impl_result_values, implementation_order, stat_type, case_to_stat_divisor_name_map, k2tree_case)
                    # Write stats line.
                    out.write("{}\n".format(dataset_values_per_implementation))
        else:
            xlsx_ws_name: str = "{}-{}-{}".format(
                dataset_type, k2tree_case, stat_type)
            if xlsx_ws_name not in global_stats_lines:
                global_stats_lines[xlsx_ws_name] = [r_dir_name]
                global_stats_lines[xlsx_ws_name].append(tsv_header)
            else:
                global_stats_lines[xlsx_ws_name].append(r_dir_name)
                global_stats_lines[xlsx_ws_name].append(tsv_header)

            # Outputing only one of 'dmgen' or 'webgraph' dataset types to .the tsv file.
            v_count_to_ds: Dict[int, str] = {}

            if dataset_type in test_case_x_impl_x_dataset[k2tree_case]:
                for d_dir_name in test_case_x_impl_x_dataset[k2tree_case][dataset_type]:
                    if "ALGO" in k2tree_case and d_dir_name in args.skip:
                        continue

                    first_impl_key: str = list(
                        test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name].keys())[0]
                    d_v_count: int = test_case_x_impl_x_dataset[k2tree_case][
                        dataset_type][d_dir_name][first_impl_key][V_COUNT_KEY]
                    v_count_to_ds[d_v_count] = d_dir_name
            ordered_v_count = sorted(list(v_count_to_ds.keys()))

            for curr_v_count in ordered_v_count:
                d_dir_name: str = v_count_to_ds[curr_v_count]
                impl_result_values = test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name]
                dataset_values_per_implementation: str = print_dataset_type_values(
                    d_dir_name, impl_result_values, implementation_order, stat_type, case_to_stat_divisor_name_map, k2tree_case)
                # Write stats line.

                # TODO COMMENT OR DELETE THIS:
                # if k2tree_case == "A" and stat_type == "memory":
                #     print(dataset_values_per_implementation)
                #     sys.exit(0)

                out.write("{}\n".format(dataset_values_per_implementation))
                global_stats_lines[xlsx_ws_name].append(
                    dataset_values_per_implementation)

    # Now generate .gnuplot file.
    if "ALGO" in k2tree_case:
        # Get the appropriate gnuplot template file.
        gnuplot_template_path: str = os.path.join(
            CURR_SCRIPT_DIR, "gnuplot-templates", "gp-sdk2sdsl-ALGO-TEST.gnuplot")
        gp_lines: List[str] = []

        # Ingest its contents into a single 'str'.
        with open(gnuplot_template_path, 'r') as gp_file:
            for l in gp_file:
                #stripped: str = l.strip()
                gp_lines.append(l)

            gp_content: str = "".join(gp_lines).replace(
                "TEST_CASE", k2tree_case)

            # Write to a new gnuplot template.
            gnuplot_out_path: str = os.path.join(
                run_out_dir, "sdk2sdsl-{}-{}.gnuplot".format(k2tree_case, stat_type))

            if stat_type == "time":
                #gp_content = gp_content.replace("STAT_DIVISOR", "1").replace("Y_UNIT", 't (μs)')
                gp_content = gp_content.replace(
                    "MULTIPLIER", "1000*1000").replace("Y_UNIT", 't (μs)')
                if "BFS" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "500") \
                        .replace("REAL_Y_TOP", "2250") \
                        .replace("TEST_NAME", "Breadth-First Seach (BFS)") \
                        .replace("COMPLEXITY_BODY", "(V * sqrt(E) + E)") \
                        .replace("X_UNIT", 'n sqrt(m) + m') \
                        .replace("X_FORMULA", "$2*sqrt($3)+$3")
                elif "DFS" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "500") \
                        .replace("REAL_Y_TOP", "2250") \
                        .replace("TEST_NAME", "Depth-First Seach (DFS)") \
                        .replace("COMPLEXITY_BODY", "(V * sqrt(E) + E)") \
                        .replace("X_UNIT", 'n sqrt(m) + m') \
                        .replace("X_FORMULA", "$2*sqrt($3)+$3")
                elif "PR" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "400") \
                        .replace("REAL_Y_TOP", "2250") \
                        .replace("TEST_NAME", "PageRank") \
                        .replace("X_UNIT", "n sqrt(m) + m") \
                        .replace("X_FORMULA", "$2*sqrt($3)+$3")
                    # .replace("X_UNIT", 'n + m') \
                    # .replace("X_FORMULA", "$2+$3")

                    #
                elif "CC" in k2tree_case:
                    #gp_content = gp_content.replace("DMGEN_Y_TOP", "800").replace("REAL_Y_TOP", "3750").replace("TEST_NAME", "Clustering Coefficient (CC)").replace("COMPLEXITY_BODY", "(E + V * sqrt(E) + E * V * (log(V)/log(2)))")
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "800") \
                        .replace("REAL_Y_TOP", "3750") \
                        .replace("TEST_NAME", "Clustering Coefficient (CC)") \
                        .replace("X_UNIT", "m sqrt(m)") \
                        .replace("X_FORMULA", "$3*sqrt($3)") \
                        .replace("#set WEBGRAPH-xtics 0,INCR,MAX", "set xtics 0,2e12,8e12") \
                        .replace("#set DMGEN-xtics 0,INCR,MAX", "set xtics 0,0.4e11,1.7e11")
                elif "CT_dummy_hash" in k2tree_case:
                    #gp_content = gp_content.replace("DMGEN_Y_TOP", "1100").replace("REAL_Y_TOP", "2550").replace("TEST_NAME", "Counting Triangles").replace("COMPLEXITY_BODY", "(E + V * sqrt(E) + (E * V * (log(V)/log(2))))")
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "1100") \
                        .replace("REAL_Y_TOP", "2550") \
                        .replace("TEST_NAME", "Counting Triangles (hash)") \
                        .replace("X_UNIT", "m sqrt(m)") \
                        .replace("X_FORMULA", "$3*sqrt($3)") \
                        .replace("#set WEBGRAPH-xtics 0,INCR,MAX", "set xtics 0,2e12,8e12") \
                        .replace("#set DMGEN-xtics 0,INCR,MAX", "set xtics 0,0.4e11,1.7e11")
                elif "CT_dummy" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "1100") \
                        .replace("REAL_Y_TOP", "2550") \
                        .replace("TEST_NAME", "Counting Triangles") \
                        .replace("X_UNIT", "m sqrt(m) log_k(n) log(m)") \
                        .replace("X_FORMULA", "$3*sqrt($3)*log($2)/log(2)*log($3)") \
                        .replace("#set DMGEN-xtics 0,INCR,MAX", "set xtics 0,1.0e13,5.5e13")
                    # .replace("X_UNIT", "m * sqrt(m) * log(n)/log(2)*log(m)") \

            else:
                #gp_content = gp_content.replace("STAT_DIVISOR", "1000").replace("Y_UNIT", 'memory (MB)')
                gp_content = gp_content \
                    .replace("*MULTIPLIER", "/1000") \
                    .replace("Y_UNIT", 'memory (MB)') \
                    .replace("X_UNIT", 'n + m') \
                    .replace("X_FORMULA", "$2+$3") \
                    .replace("#set xtics 0,INCR,MAX", "set xtics 0,1.5e12,8e12") \
                    .replace("#set DMGEN-xtics 0,INCR,MAX", "set xtics 0,0.5e7,3e7") \
                    .replace("#set WEBGRAPH-xtics 0,INCR,MAX", "set xtics 0,0.75e8,8e8")
                if "BFS" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "325") \
                        .replace("REAL_Y_TOP", "1300") \
                        .replace("TEST_NAME", "Breadth-First Seach (BFS)") \
                        .replace("COMPLEXITY_BODY", "V")
                elif "DFS" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "325") \
                        .replace("REAL_Y_TOP", "1100") \
                        .replace("TEST_NAME", "Depth-First Seach (DFS)") \
                        .replace("COMPLEXITY_BODY", "V")
                elif "PR" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "500") \
                        .replace("REAL_Y_TOP", "4500") \
                        .replace("TEST_NAME", "PageRank") \
                        .replace("COMPLEXITY_BODY", "E")
                elif "CC" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "2100") \
                        .replace("REAL_Y_TOP", "14000") \
                        .replace("TEST_NAME", "Clustering Coefficient (CC)") \
                        .replace("COMPLEXITY_BODY", "V + E")

                elif "CT_dummy_hash" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "2000") \
                        .replace("REAL_Y_TOP", "13500") \
                        .replace("TEST_NAME", "Counting Triangles (hash)") \
                        .replace("COMPLEXITY_BODY", "E")
                elif "CT_dummy" in k2tree_case:
                    gp_content = gp_content \
                        .replace("DMGEN_Y_TOP", "2000") \
                        .replace("REAL_Y_TOP", "13500") \
                        .replace("TEST_NAME", "Counting Triangles") \
                        .replace("COMPLEXITY_BODY", "1")

            temp: str = gp_content.replace("", "").replace("STAT", stat_type)

            with open(gnuplot_out_path, 'w') as custom_gnuplot:
                custom_gnuplot.write(temp)
    else:

        # Get the appropriate gnuplot template file.
        gnuplot_template_path: str = os.path.join(
            CURR_SCRIPT_DIR, "gnuplot-templates", "gp-TEST_CASE.gnuplot")
        gp_lines: List[str] = []

        # Ingest its contents into a single 'str'.
        with open(gnuplot_template_path, 'r') as gp_file:
            for l in gp_file:
                stripped: str = l.strip()
                if k2tree_case == "A" and stat_type == "memory":
                    if "title 'k2tree'" in stripped:
                        continue
                if k2tree_case == "AD":
                    if "title 'k2tree'" in stripped or "title 'k2trie1'" in stripped or "'k2trie2'" in stripped:
                        continue
                    # elif "title 'sdk2tree'" in stripped:
                    #    l = l.replace("$8", "$6")
                if k2tree_case == "AL":
                    if "title 'k2trie1'" in stripped or "title 'k2trie2'" in stripped:
                        continue
                    # elif "title 'sdk2tree'" in stripped:
                    #    l = l.replace("$8", "$6")
                    elif "title 'k2tree'" in stripped:
                        l = l.replace("$9", "$7")
                    # or stripped.endswith("'k2trie1'"):
                gp_lines.append(l)

        repl_map: Dict[str, str] = {
            "A": "ADD",
            "AC": "CHECK",
            "AD": "DEL",
            "AL": "LIST"
        }

        gp_content: str = "".join(gp_lines).replace("TEST_CASE", k2tree_case).replace(
            "TEST_NAME", repl_map[k2tree_case]).replace("STAT", stat_type)

        if stat_type == "time":
            gp_content = gp_content \
                .replace("MULTIPLIER", "1000*1000") \
                .replace("Y_UNIT", 't (μs)')
        else:
            gp_content = gp_content \
                .replace("log($2)/log(2)*log($3)", "$2+$3") \
                .replace("log_k(n) log(m)", "n + m") \
                .replace("*MULTIPLIER", "/1000") \
                .replace("Y_UNIT", 'memory (MB)') \
                .replace("#set DMGEN-xtics 0,INCR,MAX", "set xtics 0,0.5e7,3e7") \
                .replace("#set WEBGRAPH-xtics 0,INCR,MAX", "set xtics 0,0.75e8,8e8")

        if k2tree_case == "AD":
            gp_content = gp_content.replace(
                "DMGEN_Y_TOP", "15").replace("REAL_Y_TOP", "10")
        elif k2tree_case == "AL":
            gp_content = gp_content.replace(
                "DMGEN_Y_TOP", "1500").replace("REAL_Y_TOP", "1250")
            if stat_type == "time":
                gp_content = gp_content \
                    .replace("log_k(n) log(m)", "sqrt(m)") \
                    .replace("log($2)/log(2)*log($3)", "sqrt($3)") \
                    .replace("#set DMGEN-xtics 0,INCR,MAX", "set xtics 0,1000,5500") \
                    .replace("#set WEBGRAPH-xtics 0,INCR,MAX", "set xtics 0,4000,21000")
            # if stat_type == "memory":
                #gp_content = gp_content.replace("set grid", "set grid\nset logscale y")
        elif k2tree_case == "AC":
            gp_content = gp_content.replace(
                "DMGEN_Y_TOP", "4").replace("REAL_Y_TOP", "4")
        elif k2tree_case == "A":
            gp_content = gp_content.replace(
                "DMGEN_Y_TOP", "5").replace("REAL_Y_TOP", "4.5")
        else:
            gp_content = gp_content.replace("Y_TOP", "9")

        # Write to a new gnuplot template.
        gnuplot_out_path: str = os.path.join(
            run_out_dir, "all-impls-{}-{}.gnuplot".format(k2tree_case, stat_type))

        with open(gnuplot_out_path, 'w') as custom_gnuplot:
            custom_gnuplot.write(gp_content)


###########################################################################
################################ ARGUMENTS ################################
###########################################################################
# The class argparse.RawTextHelpFormatter is used to keep new lines in help text.
parser: argparse.ArgumentParser = argparse.ArgumentParser(
    description="k2tree gnuplot script generator.", formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument("-i", "--input-dir",
                    help="path to directory with k2tree implementation results.", required=True, type=str)
parser.add_argument("-o", "--output-dir",
                    help="directory where to store gnuplot script files.", required=False, type=str)
parser.add_argument("--normalize-k2trie", help="fix k2trie files that were left in the same directory from a previous execution.",
                    required=False, action="store_true")
parser.add_argument("--rename-implementations",
                    help="rename implementation name on file and directory names.", required=False, action="store_true")
parser.add_argument("--print-kt-sizes", help="print the sizes of the implementations on disk.",
                    required=False, action="store_true")

parser.add_argument('--skip', nargs='+',
                    help='Skip the following datasets.', required=False, default=[])

args: argparse.Namespace = parser.parse_args()


assert(len(args.input_dir) > 0 and os.path.isdir(args.input_dir)
       ), "> '--input-dir' must be a path to an existing directory. Exiting."
#assert((args.normalize_k2trie or args.rename_implementations) or (hasattr(args, "output_dir") and len(args.output_dir)) > 0), "> '--output-dir' is required if '--normalize-k2trie' is not provided. Exiting."
assert((args.normalize_k2trie or args.rename_implementations or args.print_kt_sizes) or args.output_dir !=
       None), "> '--output-dir' is required if neither '--normalize-k2trie' or '--rename-implementation' or '--print-kt-sizes' are provided. Exiting."
assert(not (args.rename_implementations and args.normalize_k2trie)
       ), "> '--normalize-k2trie"

###########################################################################
################################## LOGIC ##################################
###########################################################################

# Read all result files into a 'Dict' hierarchy.
aggregated_results_suffix: str = "aggregated"
gnuplot_suffix: str = "gnuplot"
figures_suffix: str = "figures"
data_dirs: List[str] = [os.path.join(args.input_dir, f) for f in os.listdir(args.input_dir) if not (f.endswith(
    aggregated_results_suffix) or f.endswith(gnuplot_suffix) or f.endswith(figures_suffix)) and os.path.isdir(os.path.join(args.input_dir, f))]

# If we are updating the hierarchy of files that followed an old version's format.
if args.normalize_k2trie:
    normalize_k2trie_2(args, data_dirs)
    sys.exit(0)
elif args.rename_implementations:
    rename_implementation_files(args, data_dirs, {"k2sdsl": "sdk2sdsl"})
    sys.exit(0)


run_dirs: Dict[str, Dict[str, Dict[str, Dict[str, List[str]]]]
               ] = get_result_file_hierarchy(args, data_dirs)


# pprint.pprint(run_dirs)

# for r in run_dirs:
#     for d in run_dirs[r]:
#         print("{} : {}".format(r, d))
# sys.exit(0)


if args.print_kt_sizes:
    for d_dir_name in run_dirs[RUN_AVG_STR].keys():
        for k2tree_dir_name in run_dirs[RUN_AVG_STR][d_dir_name].keys():
            curr_stats: Dict = run_dirs[RUN_AVG_STR][d_dir_name][k2tree_dir_name]

            for k2tree_test, test_value_dict in curr_stats.items():
                if k2tree_test == "A" and "stats" in test_value_dict and "kt_size_B" in test_value_dict["stats"]:
                    if k2tree_dir_name == "sdk2sdsl":
                        kt_sz_bits: int = test_value_dict["stats"]["kt_size_B"] * 8
                        kt_sz_MB: int = kt_sz_bits / (8 * 1024 * 1024)
                        bits_per_edge: float = kt_sz_bits / \
                            int(curr_stats[E_COUNT_KEY])
                        print(
                            "> Dataset:\t{}\t{}(bits/edge)".format(d_dir_name, bits_per_edge))

    sys.exit(0)


# Print individual
global_stats_lines: Dict[str, List[str]] = {}
for r_dir_name in run_dirs.keys():
    run_out_dir: str = os.path.join(
        args.output_dir, "{}-{}".format(r_dir_name, aggregated_results_suffix))

    if not os.path.isdir(run_out_dir):
        os.makedirs(run_out_dir)

    curr_run_data: Dict = {}

    test_case_x_impl_x_dataset: Dict = {}

    implementation_order: List[str] = []

    # Get the desired data in another structure.
    for d_dir_name in run_dirs[r_dir_name].keys():

        # if d_dir_name in args.skip:
        #    continue

        for k2tree_implementation_name in run_dirs[r_dir_name][d_dir_name].keys():

            if k2tree_implementation_name not in implementation_order:
                implementation_order.append(k2tree_implementation_name)

            if k2tree_implementation_name not in curr_run_data:
                curr_run_data[k2tree_implementation_name] = {}

            test_case_dict: Dict = run_dirs[r_dir_name][d_dir_name][k2tree_implementation_name]
            for k2tree_case in test_case_dict.keys():
                print(k2tree_case)

                if k2tree_case in ((V_COUNT_KEY, E_COUNT_KEY, TSV_SPACE_KEY, SDK2SDSL_SERIALIZED_KT_BITS_KEY)):
                    continue
                elif "stats" not in test_case_dict[k2tree_case]:
                    continue

                # if k2tree_case != V_COUNT_KEY and k2tree_case != E_COUNT_KEY and k2tree_case != TSV_SPACE_KEY and "stats" in test_case_dict[k2tree_case]:
                if k2tree_case not in curr_run_data[k2tree_implementation_name]:
                    holder: Dict = {
                        "dmgen": {},
                        "webgraph": {}
                    }
                    curr_run_data[k2tree_implementation_name][k2tree_case] = holder
                else:
                    holder: Dict = curr_run_data[k2tree_implementation_name][k2tree_case]

                if d_dir_name in (("50000", "100000", "500000", "1000000")):
                    dataset_type: str = "dmgen"
                else:
                    dataset_type: str = "webgraph"

                holder[dataset_type][d_dir_name] = test_case_dict[k2tree_case]
                test_case_dict[k2tree_case][V_COUNT_KEY] = test_case_dict[V_COUNT_KEY]
                test_case_dict[k2tree_case][E_COUNT_KEY] = test_case_dict[E_COUNT_KEY]

                # Prepare additional structure to write aggregated results per k2tree test case.
                if k2tree_case not in test_case_x_impl_x_dataset:
                    test_case_x_impl_x_dataset[k2tree_case] = {}

                if dataset_type not in test_case_x_impl_x_dataset[k2tree_case]:
                    test_case_x_impl_x_dataset[k2tree_case][dataset_type] = {}

                if d_dir_name not in test_case_x_impl_x_dataset[k2tree_case][dataset_type]:
                    test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name] = {
                    }

                if k2tree_implementation_name not in test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name]:
                    test_case_x_impl_x_dataset[k2tree_case][dataset_type][d_dir_name][
                        k2tree_implementation_name] = run_dirs[r_dir_name][d_dir_name][k2tree_implementation_name][k2tree_case]


# out_file_base
    # Print from the new structure.

    append_list: List[str] = []

    if "k2trie-1" in implementation_order:
        implementation_order.remove("k2trie-1")
        append_list.append("k2trie-1")

    if "k2trie-2" in implementation_order:
        implementation_order.remove("k2trie-2")
        append_list.append("k2trie-2")

    if "k2tree" in implementation_order:
        implementation_order.remove("k2tree")
        append_list.append("k2tree")

    implementation_order = sorted(implementation_order)
    implementation_order.extend(append_list)

    case_to_stat_name_map: Dict[str, str] = {
        "A": "add_op_exclusive_time",
        "AD": "rem_op_exclusive_time",
        "AL": "list_op_exclusive_time",
        "AC": "check_op_exclusive_time",
        "ALGO_BFS": "exclusive_algo_time",
        "ALGO_DFS": "exclusive_algo_time",
        "ALGO_CC": "exclusive_algo_time",
        "ALGO_PR": "exclusive_algo_time",
        "ALGO_CT_dummy_hash": "exclusive_algo_time",
        "ALGO_CT_dummy": "exclusive_algo_time"
    }

    case_to_stat_divisor_name_map: Dict[str, str] = {
        "A": "add_op_count",
        "AD": "rem_op_count",
        "AL": "list_op_count",
        "AC": "check_op_count",
    }

    # Iterate cases: A, AL, AC, AD, ALGO-BFS, etc.
    for k2tree_case in test_case_x_impl_x_dataset:

        impl_headers: str = "# ds/impl"
        for impl_name in implementation_order:
            impl_headers = impl_headers + "\t" + impl_name

        stat_name: str = case_to_stat_name_map[k2tree_case]
        print_impls_x_datasets_for_dataset_type(global_stats_lines, test_case_x_impl_x_dataset,
                                                k2tree_case, "dmgen", "time", implementation_order, case_to_stat_divisor_name_map)

        # if k2tree_case != "ALGO-CT_dummy":
        print_impls_x_datasets_for_dataset_type(global_stats_lines, test_case_x_impl_x_dataset,
                                                k2tree_case, "webgraph", "time", implementation_order, case_to_stat_divisor_name_map)
        print_impls_x_datasets_for_dataset_type(global_stats_lines, test_case_x_impl_x_dataset,
                                                k2tree_case, "all", "time", implementation_order, case_to_stat_divisor_name_map)

        stat_name: str = "max_resident_set_size_kbytes"
        print_impls_x_datasets_for_dataset_type(global_stats_lines, test_case_x_impl_x_dataset,
                                                k2tree_case, "dmgen", "memory", implementation_order, case_to_stat_divisor_name_map)
        print_impls_x_datasets_for_dataset_type(global_stats_lines, test_case_x_impl_x_dataset,
                                                k2tree_case, "webgraph", "memory", implementation_order, case_to_stat_divisor_name_map)
        print_impls_x_datasets_for_dataset_type(global_stats_lines, test_case_x_impl_x_dataset,
                                                k2tree_case, "all", "memory", implementation_order, case_to_stat_divisor_name_map)

    for k2tree_implementation_name in curr_run_data.keys():

        for k2tree_case in curr_run_data[k2tree_implementation_name].keys():

            # if k2tree_case.endswith("CT_dummy"):
            #    continue

            data_dict: Dict = curr_run_data[k2tree_implementation_name][k2tree_case]

            #print("IMPL:{}\tCASE:{}".format(k2tree_implementation_name, k2tree_case))
            print_impl_x_case_to_tsv(
                args, "dmgen", k2tree_implementation_name, k2tree_case, data_dict, run_out_dir)

            print_impl_x_case_to_tsv(
                args, "webgraph", k2tree_implementation_name, k2tree_case, data_dict, run_out_dir)

            #print_impl_x_case_to_tsv(args, "all-datasets", k2tree_implementation_name, k2tree_case, data_dict, run_out_dir)

# Write globals .tsv file.
global_keys: List[str] = list(global_stats_lines.keys())
global_keys = sorted(global_keys)
global_xlsx_base: str = "all-runs.tsv"
global_xlsx_path: str = os.path.join(
    args.output_dir, "{}".format(global_xlsx_base))
workbook: Workbook = xlsxwriter.Workbook(global_xlsx_path)

workbook.encoding = "utf-8"

# ds, algo, ratio val
sdk2sdsl_algo_time_ratios: Dict[str, Dict[str, float]] = {}
sdk2sdsl_algo_memory_ratios: Dict[str, Dict[str, float]] = {}


print("> Created notebook:\n\t{}".format(global_xlsx_path))
for ws_name in global_keys:

    ws_name_shortened: str = ws_name.replace("webgraph", "web")

    tsv_path: str = os.path.join(args.output_dir, ws_name + ".tsv")
    worksheet: Worksheet = workbook.add_worksheet(ws_name_shortened)
    print("\tWorksheet:\t{}".format(ws_name_shortened))
    line_ctr: int = 0

    #ret_val = worksheet.write(0, 0, "TEST")
    # print(ret_val)
    # workbook.close()

    with open(tsv_path, 'w') as ws_tsv_out:

        on_avg: bool = True

        for line in global_stats_lines[ws_name]:
            ws_tsv_out.write(line + "\n")
            col_ctr: int = 0

            # print(line)

            if "run-" in line and not line.endswith("avg"):
                on_avg = False
            elif line.endswith("avg"):
                on_avg = True

            tkns: List[str] = line.split("\t")

            # If we are iterating the actual |V|, |E| and sdk2sdsl algorithm values.
            if "ALGO" in ws_name_shortened and len(tkns) > 1 and (tkns[1].strip() != "|V|"):
                v_count: int = int(tkns[1])
                e_count: int = int(tkns[2])
                algo_val: float = float(tkns[3])

                # We want to turn something like 'dmgen-ALGO_BFS-time' or 'dmgen-ALGO_BFS-memory' into 'BFS'.
                latex_algo_name: str = ws_name_shortened.replace("-memory", "").replace("-time", "")
                latex_algo_name = latex_algo_name[latex_algo_name.find(
                    "ALGO_") + 5:]

                if latex_algo_name == "CT_dummy_hash":
                    latex_algo_name = "CT (hash)"
                elif latex_algo_name == "CT_dummy":
                    latex_algo_name = "CT (neighbour)"

                if tkns[0] == "50000":
                    latex_ds_name = "dm50K"
                elif tkns[0] == "100000":
                    latex_ds_name = "dm100K"
                elif tkns[0] == "500000":
                    latex_ds_name = "dm500K"
                elif tkns[0] == "1000000":
                    latex_ds_name = "dm1M"
                else:
                    latex_ds_name = tkns[0].replace("@100000", "").strip()

                if ws_name_shortened.endswith("-memory"):
                    # algo_val is in KB, multiply by 1000 to convert to Bytes.
                    extra_val: float = float(algo_val / (v_count + e_count)) * 1000 #/ 1000
                    tkns.append(extra_val)

                    if on_avg:
                        if latex_ds_name not in sdk2sdsl_algo_memory_ratios:
                            sdk2sdsl_algo_memory_ratios[latex_ds_name] = {}
                        if latex_algo_name not in sdk2sdsl_algo_memory_ratios[latex_ds_name]:
                            sdk2sdsl_algo_memory_ratios[latex_ds_name][latex_algo_name] = extra_val

                else:
                    should_store_latex_val: bool = False

                    #time_unit: int = 1000_000

                    if "BFS" in ws_name_shortened:
                        extra_val: float = 1000 * float(
                            1000 * 1000 * algo_val / ((v_count * math.sqrt(e_count)) + e_count)) 
                        tkns.append(extra_val)
                        should_store_latex_val = True

                        #if "BFS" not in
                    if "DFS" in ws_name_shortened:
                        extra_val: float = 1000 * float(
                            1000 * 1000 * algo_val / ((v_count * math.sqrt(e_count)) + e_count))
                        tkns.append(extra_val)
                        should_store_latex_val = True
                    if "CC" in ws_name_shortened:
                        extra_val: float = 1000 * float(
                            1000 * 1000 * algo_val / (e_count * math.sqrt(e_count)))
                        tkns.append(extra_val)
                        should_store_latex_val = True
                    if "CT_dummy_hash" in ws_name_shortened:
                        extra_val: float = 1000 * float(
                            1000 * 1000 * algo_val / (e_count * math.sqrt(e_count)))
                        tkns.append(extra_val)
                        should_store_latex_val = True
                    if "CT_dummy" in ws_name_shortened:
                        extra_val: float = 1000 * float(
                            1000 * 1000 * algo_val / ((e_count * math.sqrt(e_count)) * math.log(v_count, 2) * (math.log(e_count))))
                        tkns.append(extra_val)
                        should_store_latex_val = True

                    if on_avg and should_store_latex_val:
                        if latex_ds_name not in sdk2sdsl_algo_time_ratios:
                            sdk2sdsl_algo_time_ratios[latex_ds_name] = {}
                        if latex_algo_name not in sdk2sdsl_algo_time_ratios[latex_ds_name]:
                            sdk2sdsl_algo_time_ratios[latex_ds_name][latex_algo_name] = extra_val

            for cell_val in tkns:
                worksheet.write(line_ctr, col_ctr, str(cell_val))
                col_ctr += 1

            line_ctr += 1


workbook.close()


# Print sdk2sdsl graph algorithm values for LaTeX.
latex_ds_order: List[str] = ["dm50K", "dm100K", "dm500K", "dm1M", "uk-2007-05", "in-2004", "uk-2014-host", "eu-2015-host"]
latex_algo_order: List[str] = ["BFS", "DFS",
                               "CC", "CT (hash)", "CT (neighbour)"]

for ds in latex_ds_order:
#for ds in sdk2sdsl_algo_time_ratios:
    #pprint.pprint(sdk2sdsl_algo_time_ratios[ds])

    latex_line: str = "\\texttt{" + ds + "}\t"
    for algo in latex_algo_order:
        if algo in sdk2sdsl_algo_time_ratios[ds]:
            latex_line += "&\t{0:.4f} ".format(sdk2sdsl_algo_time_ratios[ds][algo])
            #latex_line += "&\t{} ".format(sdk2sdsl_algo_time_ratios[ds][algo])
        else:
            latex_line += "&\tN/A "
        
    if ds == "dm1M":
        latex_line += " \\\\ \\hline"
    elif ds != "eu-2015-host":
        latex_line += " \\\\"
    print(latex_line)

for ds in latex_ds_order:
#for ds in sdk2sdsl_algo_memory_ratios:
    latex_line: str = "\\texttt{" + ds + "}\t"
    
    for algo in latex_algo_order:
        if algo in sdk2sdsl_algo_memory_ratios[ds]:
            latex_line += "&\t{0:.3f} ".format(sdk2sdsl_algo_memory_ratios[ds][algo])
            #latex_line += "&\t{} ".format(sdk2sdsl_algo_memory_ratios[ds][algo])
        else:
            latex_line += "&\tN/A "

    if ds == "dm1M":
        latex_line += " \\\\ \\hline"
    elif ds != "eu-2015-host":
        latex_line += " \\\\"
    print(latex_line)
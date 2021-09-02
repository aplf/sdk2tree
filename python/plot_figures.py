#!/usr/bin/env python3
__copyright__ = """ Copyright 2019 Miguel E. Coimbra

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

# PEP 8 Style Guide for imports: https://www.python.org/dev/peps/pep-0008/#imports
# 1. standard library imports
import argparse
from decimal import Decimal
from functools import cmp_to_key
import logging
from math import log
import os
import random
import sys
from typing import Dict, List


# 2. other imports
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib import rc
from matplotlib import rcParams
from matplotlib import style
from matplotlib.patches import Rectangle


# https://stackoverflow.com/questions/15814635/prettier-default-plot-colors-in-matplotlib
# http://seaborn.pydata.org/tutorial/color_palettes.html
# import seaborn as sns

###########################################################################
########################### MATPLOTLIB CONFIGS ############################
###########################################################################

# http://tonysyu.github.io/mpltools/auto_examples/style/plot_ggplot.html
#style.use('ggplot')

# http://sbillaudelle.de/2015/02/20/matplotlib-with-style.html
# http://sbillaudelle.de/2015/02/23/seamlessly-embedding-matplotlib-output-into-latex.html

# Activate latex text rendering
plt.rc('text', usetex=True)

# Controlling font size from matplotlib: https://stackoverflow.com/questions/3899980/how-to-change-the-font-size-on-a-matplotlib-plot
#SMALL_SIZE = 8
#MEDIUM_SIZE = 11
#BIGGER_SIZE = 12

SMALL_SIZE = 17
INNER_PLOT_FONT_SIZE = 13
MEDIUM_SIZE = 20
BIGGER_SIZE = 21

plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
#plt.rc('axes', titlesize=SMALL_SIZE)     # fontsize of the axes title
plt.rc('axes', titlesize=MEDIUM_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=MEDIUM_SIZE)   # fontsize of the tick labels
plt.rc('ytick', labelsize=MEDIUM_SIZE)   # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)   # legend fontsize
plt.rc('legend', loc="best")
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title

# Disable automatic layout resize. This way the plot rectangle will always be the same size.
plt.rcParams['figure.autolayout'] = False
plt.rcParams['legend.loc'] = "best"

plt.rcParams['lines.markersize'] = 7
plt.rcParams['lines.linestyle'] = "--"
plt.rcParams['lines.linewidth'] = 0.65
plt.rcParams['lines.marker'] = "^"
plt.rcParams['lines.color'] = "black"

plt.rcParams['figure.figsize'] = (8,6)

PLOT_ALPHA = 0.45

# List of matplotlib markers: https://matplotlib.org/api/markers_api.html
#colors = ('b', 'g', 'r', 'c', 'm', 'y', 'k')

# Convert the cycler of rcParams to a list of colors strings.
# https://matplotlib.org/examples/color/color_cycle_demo.html
# https://matplotlib.org/gallery/color/color_cycler.html
colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

# Set plot styles for group plots.
styles = ["o","+","*","x","D", "<"]

# Set linestyles.
linestyles = ['_', '-', '--', ':']
markers = []
# List of matplotlib markers: https://matplotlib.org/api/markers_api.html
for m in Line2D.markers:
    try:
        if len(m) == 1 and m != ' ':
            markers.append(m)
    except TypeError:
        pass

###########################################################################
################################ FUNCTIONS ################################
###########################################################################

# See https://thispointer.com/python-how-to-get-list-of-files-in-directory-and-sub-directories/
def getListOfFiles(dirName: str) -> List[str]:
    # create a list of file and sub directories 
    # names in the given directory 
    listOfFile = os.listdir(dirName)
    allFiles = list()
    # Iterate over all the entries
    for entry in listOfFile:
        # Create full path
        fullPath = os.path.join(dirName, entry)
        # If entry is a directory then get the list of files in this directory 
        if os.path.isdir(fullPath):
            allFiles = allFiles + getListOfFiles(fullPath)
        else:
            allFiles.append(fullPath)
                
    return allFiles  

def get_file_list(startpath: str) -> List[str]:
	for root, _, files in os.walk(startpath):
		root.replace(startpath, '').count(os.sep)
		file_list = []
		
		for f in files:
			full_name = '{}{}{}'.format(root, os.path.sep, f)
			file_list.append(full_name)
			print("\t{}".format(full_name))
		return file_list

def list_files(startpath: str) -> None:
	for root, _, files in os.walk(startpath):
		level = root.replace(startpath, '').count(os.sep)
		indent = ' ' * 4 * (level)
		print('{}{}/'.format(indent, os.path.basename(root)))
		subindent = ' ' * 4 * (level + 1)

		for f in files:
			print('{}{}'.format(subindent, f))
		

def get_k2_sizes(path_list: List[str]) -> Dict:
	version_sizes = {}
	for p in path_list:
		if "kt-" in p:

			p_tkns = p.split(os.path.sep)

			version_index = 0
			i = 0
			for tkn in p_tkns:
				if "dyn-" in tkn:
					version = tkn
					version_index = i
				i = i + 1
			dataset = p_tkns[version_index - 1]

			# Are we iterating a .kt file from a k2-tree version and dataset that we've seen already?
			if version in version_sizes and dataset in version_sizes[version]:
				continue

			if not version in version_sizes:
				version_sizes[version] = {}

			if not dataset in version_sizes[version]:
				file_dir = p[:p.rfind(os.path.sep)]
				#print(file_dir)
				k2_size = 0
				for f in os.listdir(file_dir):
					f_path = os.path.join(file_dir, f)
					if os.path.isfile(f_path):
						#print(f_path)
						k2_size = k2_size + os.path.getsize(f_path)
				if "inesc" in version and ("kt-AD" in p or "kt-AL" in p):
					k2_size = int(k2_size/2)
				version_sizes[version][dataset] = k2_size
	return version_sizes


def get_stats(df_dict: Dict, filepath: str, debugging: bool = False)-> None:

	# Are we processing a time or memory file?

	file_dic = {}

	if filepath.endswith("-time.tsv"):
		file_type = "time"
		with open(filepath, 'r') as tf:
			tf_lines = tf.readlines()

			if len(tf_lines) > 2:
				print("> Time file is supposed to have 2 lines ({} found).".format(len(tf_lines)))
				print("> File:\t{}".format(filepath))
				print("> Exiting.")
				exit(1)

			if tf_lines[0].strip().endswith(";"):
				col_names = tf_lines[0][:-2].split(";")
			else:
				col_names = tf_lines[0].split(";")

			col_vals = tf_lines[1].split(";")

			#if debugging:
			#	print("{} names / {} values".format(len(col_names), len(col_vals)))

			for i in range(0, len(col_names)):
				#if debugging:
				#	print("{} = {}".format(col_names[i], col_vals[i]))
				if col_names[i].endswith("count") or col_vals[i] == -1:
					file_dic[col_names[i]] = int(col_vals[i])
				else:
					file_dic[col_names[i]] = float(col_vals[i])

			
	elif filepath.endswith("-memory.txt"):
		file_type = "memory"
		with open(filepath, 'r') as tf:
			tf_lines = tf.readlines()
			for l in tf_lines:
				l_stripped = l.strip()
				if l_stripped.startswith("Maximum resident set size (kbytes): "):
					mem_ind = l_stripped.find(":")
					max_resident_set_size = l_stripped[mem_ind + 2:]
					file_dic["max_resident_set_size"] = float(max_resident_set_size) / 1000
	else:
		return

	#if debugging:
	#	print("> Dictionary - {}\n\t{}".format(filepath, file_dic))

	name_sans_path = filepath[filepath.rfind(os.path.sep)+1:]

	# Check k2-tree version.
	if "static" in name_sans_path:
		version = "static"
	elif "dyn-inesc" in name_sans_path:
		version = "dyn-inesc"
		pass
	elif "dyn-upc" in name_sans_path:
		version = "dyn-upc"
		pass
	else:
		print("> Unexpected file name with unknown k2-tree version:\t{}. Exiting.".format(name_sans_path))
		exit(1)

	# Check execution scenario (A: additions only; AD: additions and deletions; AL: additions and listings).
	if "-A-" in name_sans_path:
		scenario = "A"
	elif "-AD-" in name_sans_path:
		scenario = "AD"
		pass
	elif "-AL-" in name_sans_path or version == "static":
		scenario = "AL"
		pass
	else:
		print("> Unexpected file name with unknown type of operation:\t{}. Exiting.".format(name_sans_path))
		exit(1)


	dataset = name_sans_path[:name_sans_path.find(version) - 1]
	new_dic_key = dataset + "_" + version + "_" + scenario

	if not new_dic_key in df_dict:
		df_dict[new_dic_key] = {}

	# Columns: scenario (A, AD, AL), version (static, dyn-inesc, dyn-upc), dataset, peak memory (MB), 
	

	rlim = name_sans_path.find(version) - 1
	dataset = name_sans_path[:rlim]
	
#start_time;loop_time;exclusive_save_time;time_per_add_op;time_per_rem_op;time_per_list_op;add_op_count;add_op_exclusive_time;rem_op_count;rem_op_exclusive_time;list_op_count;list_op_exclusive_time

	file_dic["scenario"] = scenario
	file_dic["version"] = version
	file_dic["dataset"] = dataset

	df_dict[new_dic_key][file_type] = file_dic

def version_plot(df: pd.DataFrame, index: str, columns: str, values: str, ylabel: str, title: str, is_log: bool, file_path: str, debugging: bool = False) -> None:

	# How to sort the pandas pivot DataFrame:
	# https://pandas.pydata.org/pandas-docs/version/0.19/generated/pandas.DataFrame.sort_values.html#pandas.DataFrame.sort_values
	order = ["100-K", "uk-2007-05@100000", "in-2004", "1-M", "10-M", "eu-2015-host"]
	sorted_pivot_df = df.pivot(index, columns, values).reindex(order, axis=0).dropna(thresh=2)

	bar_val_list = []

	if "memory" in ylabel:
		if debugging:
			print("\n\n\n> File:\t{}".format(file_path))
		
		# Iterating the rows of a pandas DataFrame.
		# https://www.geeksforgeeks.org/iterating-over-rows-and-columns-in-pandas-dataframe/
		for _, j in sorted_pivot_df.iterrows(): 
		
			# Iterating a pandas DataFrame Series:
			# https://stackoverflow.com/questions/38387529/how-to-iterate-over-pandas-series-generated-from-groupby-size
			for series_item in j:
				bar_val_list.append(str(series_item) + " MB")
				if debugging:
					print(str(series_item) + "\t")

	ax = sorted_pivot_df.plot(kind='bar', rot=45, log=is_log, alpha=PLOT_ALPHA)

	ax.xaxis.label.set_visible(False)

	ax.set_ylabel(ylabel)
	ax.set_title(title)
	prepare_op_legend(ylabel)

	# On adding text notes with rotation:
	# https://matplotlib.org/3.1.0/gallery/text_labels_and_annotations/text_rotation.html
	if "memory" in ylabel:
		bar_val_index = 0
		ax.patches.sort(key=Rectangle.get_x)
		#if debugging:
		#	print(ax.patches)
		font_size = INNER_PLOT_FONT_SIZE # 11


		curr_ylim = plt.ylim()
		yy_resolution_limit = int(abs(curr_ylim[1] - curr_ylim[0]) * 0.10)
		yy_resolution_bump = int(abs(curr_ylim[1] - curr_ylim[0]) * 0.075)

		#curr_xlim = plt.xlim()
		#xx_resolution_limit = int()
		

		if debugging:
			print("> plt.ylim():\t{}".format(plt.ylim()))
			print("> yy_resolution_limit:\t{}".format(yy_resolution_limit))
			print(yy_resolution_limit)
		list_len = len(bar_val_list)
		patch_text_rot = 90 # 45
		xx_resolution_bump = 0
		for p in ax.patches:
			xx_resolution_bump = p.get_width() / 4	
			if bar_val_index < list_len and (not "nan" in bar_val_list[bar_val_index]):
				curr_height = p.get_height()
				if curr_height < yy_resolution_limit:
					if "-log" in file_path:
						ax.text(p.get_x() + xx_resolution_bump, curr_height + log(yy_resolution_bump), bar_val_list[bar_val_index], rotation=patch_text_rot, fontsize=font_size)
					else:
						ax.text(p.get_x() + xx_resolution_bump, curr_height + yy_resolution_bump, bar_val_list[bar_val_index], rotation=patch_text_rot, fontsize=font_size)
					if debugging:
						print("height:\t{}\tval:{}".format(curr_height, bar_val_list[bar_val_index]))
				else:
					ax.text(p.get_x() + xx_resolution_bump, curr_height - 0.40, bar_val_list[bar_val_index], rotation=patch_text_rot, fontsize=font_size)
			bar_val_index = bar_val_index + 1
	plt.tight_layout()

	plt.savefig(file_path, transparent = True)

	if debugging:
		print("> Saving:\t{}".format(file_path))
	
	# plt.savefig(file_path.replace(".png", ".pgf"), transparent = True)
	# 	ValueError: Error processing 'Listing operation for all \(\displaystyle \it{k^{2}}\)-tree implementations'
	# LaTeX Output:
	# ! Undefined control sequence.
	# <argument> ...eration for all \(\displaystyle \it 
	#                                                   {k^{2}}\)-tree implementat...
	# <*> ...laystyle \it{k^{2}}\)-tree implementations}

	plt.close(ax.get_figure())

def plot_version_comparison(k2_tree_df: Dict, out_dir: str, scenario: str, debugging: bool = False)-> None:

	if scenario == 'AL':
		exclusive_time_key = "list_op_exclusive_time"
		op_count_key = "list_op_count"
	elif scenario == 'A':
		exclusive_time_key = "add_op_exclusive_time"
		op_count_key = "add_op_count"
	elif scenario == 'AD':
		exclusive_time_key = "rem_op_exclusive_time"
		op_count_key = "rem_op_count"
	else:
		print("> Unknown scenario specified: {}. Exiting.".format(scenario))
		exit(1)

	# Filter rows by column value:
	# https://stackoverflow.com/questions/17071871/select-rows-from-a-dataframe-based-on-values-in-a-column-in-pandas
	df = k2_tree_df.loc[k2_tree_df["scenario"] == scenario]

	# Dealing with pandas SettingWithCopyWarning:
	# https://www.dataquest.io/blog/settingwithcopywarning/
	df = df.copy()

	# Create new column based on division of other columns.
	# https://stackoverflow.com/questions/35439613/python-pandas-dividing-column-by-another-column
	df["avg_scenario_op"] = df[exclusive_time_key]/df[op_count_key]

	# Selecting sub-columns for plotting.
	# https://medium.com/dunder-data/selecting-subsets-of-data-in-pandas-6fcd0170be9c
	df = df[["dataset", "|V|", "version", "avg_scenario_op", exclusive_time_key, op_count_key, "max_resident_set_size", "k2_size"]]

	# Sorting pandas DataFrame by columns.
	# https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DataFrame.sort_values.html
	#df = df.sort_values(by=["dataset", "version"])
	df = df.sort_values(by=["|V|", "dataset", "version"], ascending=True)


	# Filter datasets for which there was only a single system executing.
	dups_table = df.pivot_table(index=['dataset'], aggfunc='size')
	my_colnames = []
	for dataset, cnt in dups_table.to_frame().to_records():
		if cnt == 1:
			my_colnames.append(str(dataset))
	df = df.loc[np.logical_not(df["dataset"].isin(my_colnames))]

	df_resident_mem_and_k2_disk_sz = df[(df['version'] == "static")]

	#print("STATIC FILTER")
	#with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
	#	print(df_resident_mem_and_k2_disk_sz[["version", "dataset", "max_resident_set_size", "k2_size"]])

	

	df_resident_mem_and_k2_disk_sz = df_resident_mem_and_k2_disk_sz.assign(version="static-on-disk")


	df_resident_mem_and_k2_disk_sz.loc[df_resident_mem_and_k2_disk_sz["version"] == "static-on-disk", "max_resident_set_size"] = df_resident_mem_and_k2_disk_sz["k2_size"] / (1024 * 1024)



	#print("MAX REPLACE")
	#with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
	#	print(df_resident_mem_and_k2_disk_sz[["version", "dataset", "max_resident_set_size", "k2_size"]])


	



	df_memory_disk_concat = pd.concat([df, df_resident_mem_and_k2_disk_sz]).round(3)


	#with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
	#	print(df_memory_disk_concat)




	# Plot memory usage.
	ylabel = "Peak resident memory (MB)"
	title = ""
	if(scenario == "A"):
		title = r"Edge addition operation for dynamic $\it{k^{2}}$-tree implementations"
	elif(scenario == "AL"):
		title = r"Listing operation for all $\it{k^{2}}$-tree implementations"
	elif(scenario == "AD"):
		title = r"Edge removal operation for all dynamic $\it{k^{2}}$-tree implementations"

	op_memory_fig_path = os.path.join(out_dir, "{}-memory.png".format(scenario))


	version_plot(df_memory_disk_concat, "dataset", "version", "max_resident_set_size", ylabel, title, False, op_memory_fig_path, debugging)
	#version_plot(df, "dataset", "version", "max_resident_set_size", ylabel, title, False, op_memory_fig_path)


	

	# Plot memory usage in log scale.
	ylabel = "Log-scale peak resident memory (MB)"
	op_memory_fig_path = os.path.join(out_dir, "{}-memory-log.png".format(scenario))
	version_plot(df_memory_disk_concat, "dataset", "version", "max_resident_set_size", ylabel, title, True, op_memory_fig_path, debugging)
	#version_plot(df, "dataset", "version", "max_resident_set_size", ylabel, title, True, op_memory_fig_path)
	
	# Plot total time for each operation.
	if(scenario == "A"):
		#ylabel = "Total time per edge addition (s)"
		ylabel = "Addition total time (s)"
	elif(scenario == "AL"):
		#ylabel = "Total time per neighbor listing (s)"
		ylabel = "Neighbor listing total time (s)"
	elif(scenario == "AD"):
		#ylabel = "Total time per edge removal (s)"
		ylabel = "Edge removal total time (s)"

	op_times_fig_path = os.path.join(out_dir, "{}.png".format(scenario))
	version_plot(df, "dataset", "version", exclusive_time_key, ylabel, title, False, op_times_fig_path)
	
	# Plot total time for each operation using log scale.
	ylabel = ylabel.replace("time", "log-time")
	op_times_fig_path = os.path.join(out_dir, "{}-log.png".format(scenario))
	version_plot(df, "dataset", "version", exclusive_time_key, ylabel, title, True, op_times_fig_path)

	# Plot time per operation.
	#ylabel = ylabel.replace("Total log-time", "Time").replace("(s)", "operation (s)")
	ylabel = ylabel.replace("log-time", "time").replace("total", "per-operation")
	time_per_op_fig_path = os.path.join(out_dir, "{}-ops.png".format(scenario))
	version_plot(df, "dataset", "version", "avg_scenario_op", ylabel, title, False, time_per_op_fig_path)

	# Plot time per operation using log scale.
	ylabel = ylabel.replace("time", "log-time")
	time_per_op_fig_path = os.path.join(out_dir, "{}-ops-log.png".format(scenario))
	version_plot(df, "dataset", "version", "avg_scenario_op", ylabel, title, True, time_per_op_fig_path)


	
def prepare_op_legend(ylabel: str) -> None:
	# Based on https://stackoverflow.com/questions/23037548/change-main-plot-legend-label-text
	#L=plt.legend()

	# Transparency on axis legend.
	# https://matplotlib.org/gallery/recipes/transparent_legends.html
	#L=plt.legend(framealpha=0.1, bbox_to_anchor=(0,1.05,1,0.2), loc="lower left", ncol=2, frameon=True, facecolor="red", mode="expand", fontsize=14)

	# Note: set frameon=True to see the area of the legend box.
	L=plt.legend(framealpha=0.1, bbox_to_anchor=(0,1.05,1,0.2), loc="lower left", ncol=2, frameon=False, facecolor="red", mode="expand", fontsize=14)

	

	texts = L.get_texts()	

	for i in range(0, len(texts)):
		curr_legend = texts[i].get_text()
		if curr_legend == "A":
			texts[i].set_text("Addition")
		if curr_legend == "AL":
			texts[i].set_text("Listing")
		if curr_legend == "AD":
			texts[i].set_text("Removal")
		if curr_legend == "static" and "memory" in ylabel:
			texts[i].set_text("static - (inc. full reconstruction from disk)")


def plot_scenario_comparison_times(k2_tree_df: Dict, out_dir: str, version: str, debugging: bool = False) -> None: 

	if not (version == "dyn-inesc" or version == "dyn-upc"):
		print("> Unknown k2-tree version specified: {}. Exiting.".format(version))
		exit(1)

	# Filter rows by column value:
	# https://stackoverflow.com/questions/17071871/select-rows-from-a-dataframe-based-on-values-in-a-column-in-pandas
	df = k2_tree_df.loc[k2_tree_df["version"] == version]

	# Create new column based on division of other columns.
	# https://stackoverflow.com/questions/35439613/python-pandas-dividing-column-by-another-column
	df = df.copy()
	df["avg_list_op"] = df["list_op_exclusive_time"]/df["list_op_count"]
	df = df.copy()
	df["avg_add_op"] = df["add_op_exclusive_time"]/df["add_op_count"]

	df = df.assign(avg_time_per_op = -1.0)
	df = df.assign(exclusive_time = -1.0)
	df = df.assign(total_time = -1.0)

	df.loc[df.scenario == "AL", ["avg_time_per_op"]] = df['avg_list_op']
	df.loc[df.scenario == "A", ["avg_time_per_op"]] = df['avg_add_op']

	# if version == 'dyn-inesc':
	if version == 'dyn-inesc' or version == "dyn-upc":
		df = df.copy()
		df["avg_rem_op"] = df["rem_op_exclusive_time"]/df["rem_op_count"]
		df.loc[df.scenario == "AD", ["avg_time_per_op"]] = df['avg_rem_op']
		df.loc[df.scenario == "AD", ["exclusive_time"]] = df['rem_op_exclusive_time']
		df.loc[df.scenario == "AD", ["total_time"]] = df['add_op_exclusive_time'] + df['rem_op_exclusive_time']	

	df.loc[df.scenario == "AL", ["exclusive_time"]] = df['list_op_exclusive_time']
	df.loc[df.scenario == "AL", ["total_time"]] = df['add_op_exclusive_time'] + df['list_op_exclusive_time']

	df.loc[df.scenario == "A", ["exclusive_time"]] = df['add_op_exclusive_time']
	df.loc[df.scenario == "A", ["total_time"]] = df['add_op_exclusive_time']

	# Selecting sub-columns for plotting.
	# https://medium.com/dunder-data/selecting-subsets-of-data-in-pandas-6fcd0170be9c
	df = df[["dataset", "|V|", "scenario", "avg_time_per_op", "exclusive_time", "total_time", "max_resident_set_size"]]

	# Sorting pandas DataFrame by columns.
	# https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DataFrame.sort_values.html
	#df = df.sort_values(by=["dataset", "scenario"])
	df = df.sort_values(by=["|V|", "dataset", "scenario"], ascending=True)
	
	# Filter datasets for which there was only a single system executing.
	dups_table = df.pivot_table(index=['dataset'], aggfunc='size')
	my_colnames = []
	for dataset, cnt in dups_table.to_frame().to_records():
		#if (cnt < 3 and version == "dyn-inesc") or (cnt < 2 and version == "dyn-upc"):
		if (cnt < 3 and version == "dyn-inesc") or (cnt < 3 and version == "dyn-upc"):
			my_colnames.append(str(dataset))
	df = df.loc[np.logical_not(df["dataset"].isin(my_colnames))]

	if args.debugging and args.verbose:
		with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
			print(df)


	# Plot peak resident memory for each of the current system's dataset.
	ylabel = "Peak resident memory (MB)"
	title = "{} ".format(version) + r"$\it{k^{2}}$-tree implementation"
	op_mem_fig_path = os.path.join(out_dir, "{}-memory.png".format(version))
	version_plot(df, "dataset", "scenario", "max_resident_set_size", ylabel, title, False, op_mem_fig_path, debugging)

	# Plot peak resident memory log scale for each of the current system's dataset.
	ylabel = "Log-scale peak resident memory (MB)"
	op_mem_fig_path = os.path.join(out_dir, "{}-memory-log.png".format(version))
	version_plot(df, "dataset", "scenario", "max_resident_set_size", ylabel, title, True, op_mem_fig_path, debugging)

	# Pivot to plot specific columns and plot the sum of the time the operation took each time it executed.
	ylabel = "Total time per operation (s)"
	op_times_fig_path = os.path.join(out_dir, "{}-summed-time-per-operation.png".format(version))
	version_plot(df, "dataset", "scenario", "exclusive_time", ylabel, title, False, op_times_fig_path, debugging)

	# Plot log scale.
	ylabel = ylabel.replace("time", "log-time")
	op_times_fig_path = os.path.join(out_dir, "{}-summed-time-per-operation-log.png".format(version))
	version_plot(df, "dataset", "scenario", "exclusive_time", ylabel, title, True, op_times_fig_path, debugging)

	# Pivot to plot specific columns and plot the time per operation, calculated from total time.
	ylabel = "Time per operation (s)"
	time_per_op_fig_path = os.path.join(out_dir, "{}-time-per-operation.png".format(version))
	version_plot(df, "dataset", "scenario", "avg_time_per_op", ylabel, title, False, time_per_op_fig_path, debugging)

	# Plot log scale.
	ylabel = ylabel.replace("Time", "Log-time")
	time_per_op_fig_path = os.path.join(out_dir, "{}-time-per-operation-log.png".format(version))
	version_plot(df, "dataset", "scenario", "avg_time_per_op", ylabel, title, True, time_per_op_fig_path, debugging)

	if not (version == "dyn-inesc" or version == "dyn-upc"):
		return

	# Show only results for scenario "A" and "AD"
	df = df.loc[df["scenario"].isin(["A", "AD"])]

	# Pivot to plot specific columns and plot the sum of the time the operation took each time it executed.
	ylabel = "Total time per operation (s)"
	op_times_fig_path = os.path.join(out_dir, "{}-A_AD_summed-time-per-operation.png".format(version))
	version_plot(df, "dataset", "scenario", "exclusive_time", ylabel, title, False, op_times_fig_path, debugging)

	# Plot log scale.
	ylabel = ylabel.replace("time", "log-time")
	op_times_fig_path = os.path.join(out_dir, "{}-A_AD_summed-time-per-operation-log.png".format(version))
	version_plot(df, "dataset", "scenario", "exclusive_time", ylabel, title, True, op_times_fig_path, debugging)

	# Pivot to plot specific columns and plot the time per operation, calculated from total time.
	ylabel = "Time per operation (s)"
	time_per_op_fig_path = os.path.join(out_dir, "{}-A_AD_time-per-operation.png".format(version))
	version_plot(df, "dataset", "scenario", "avg_time_per_op", ylabel, title, False, time_per_op_fig_path, debugging)

	# Plot log scale.
	ylabel = "Log-time per operation (s)"
	time_per_op_fig_path = os.path.join(out_dir, "{}-A_AD_time-per-operation-log.png".format(version))
	version_plot(df, "dataset", "scenario", "avg_time_per_op", ylabel, title, True, time_per_op_fig_path, debugging)

###########################################################################
################################## MAIN ###################################
###########################################################################

# The class argparse.RawTextHelpFormatter is used to keep new lines in help text.
DESCRIPTION_TEXT = "Plotter for comparing k2-tree version results."
parser = argparse.ArgumentParser(description=DESCRIPTION_TEXT, formatter_class=argparse.RawTextHelpFormatter)
parser.add_argument('-i', '--input-dir', help="results directorys.", required=True, type=str)
parser.add_argument('-o', '--out-dir', help="output directory.", required=False, type=str)
parser.add_argument('-d', '--debugging', help="enable debugging.", required=False, action="store_true")
parser.add_argument('-v', '--verbose', help="enable verbose output.", required=False, action="store_true")

args = parser.parse_args()

### Sanitize arguments.

# Output directory checks.
out_dir = None
if (not args.out_dir is None):
	if len(args.out_dir) == 0:
		print("> -out-dir must be a non-empty string. Exiting.")
		exit(1)
	elif not (os.path.exists(args.out_dir) and os.path.isdir(args.out_dir)):
		print("> Provided output directory does not exist: {}. Creating.".format(args.out_dir))
		os.mkdir(args.out_dir)
		print("> Created:\t{}".format(args.out_dir))

# Input directory checks.
if not (os.path.exists(args.input_dir) and os.path.isdir(args.input_dir)):
	print("> Provided input directory does not exist: {}. Exiting.".format(args.input_dir))
	exit(1)


print("> Debugging:\t{}".format(args.debugging))

# Build a list of file paths from the string input directory.
files = getListOfFiles(args.input_dir)

#if args.debugging:
#	list_files(args.input_dir)




# List of dictionaries to build a Pandas dataframe.
# See: https://stackoverflow.com/questions/20638006/convert-list-of-dictionaries-to-a-pandas-dataframe
dataframe_feed = {}
# Columns: scenario (A, AD, AL), version (static, dyn-inesc, dyn-upc), dataset, time, peak memory (MB)




# Separate by k2-tree version using list comprehension.
if args.verbose:
	print("> Static k2-tree files:")
static_k2_files = [f for f in files if "static" in f]
for static_k2 in static_k2_files:
	if args.verbose:
		print("\t{}".format(static_k2))
	get_stats(dataframe_feed, static_k2, args.debugging)

if args.verbose:
	print("\n> Dynamic INESC-ID k2-tree files:")
dyn_inesc_k2_files = [f for f in files if "dyn-inesc" in f]
for dyn_inesc_k2 in dyn_inesc_k2_files:
	if args.verbose:
		print("\t{}".format(dyn_inesc_k2))
	get_stats(dataframe_feed, dyn_inesc_k2, args.debugging)


if args.verbose:
	print("\n> Dynamic UPC k2-tree files:")
dyn_upc_k2_files = [f for f in files if "dyn-upc" in f]
for dyn_upc_k2 in dyn_upc_k2_files:
	if args.verbose:
		print("\t{}".format(dyn_upc_k2))
	get_stats(dataframe_feed, dyn_upc_k2, args.debugging)


# On merging python dicts (Python 3.5+):
# https://stackoverflow.com/questions/38987/how-to-merge-two-dictionaries-in-a-single-expression
merged_dataframe_feed = []
for key, dic in dataframe_feed.items():
	curr_keys = dic.keys()
	if "time" in curr_keys and "memory" in curr_keys:
		merged_dataframe_feed.append({**dic["memory"], **dic["time"]})

# Merge all the dictionaries into pandas dataframe.
k2_tree_df = pd.DataFrame(merged_dataframe_feed)


# Replace NaN values with integer -1.
k2_tree_df = k2_tree_df.replace(np.nan, -1.0)

# Note: pandas apparently converts integer fields to float without requiring such a transformation.
# Here we convert back to int.
# See: https://stackoverflow.com/questions/55455010/how-to-stop-pandas-dataframe-from-converting-int-to-float-for-no-reason
# Converting data type: https://stackoverflow.com/questions/15891038/change-data-type-of-columns-in-pandas
k2_tree_df = k2_tree_df.astype({"add_op_count": int, "rem_op_count": int, "list_op_count": int})


# Get k2-tree file sizes for the dyn-inesc and dyn-upc versions.
k2_tree_df['k2_size'] = 0

inesc_sizes = get_k2_sizes(dyn_inesc_k2_files)
#print(inesc_sizes)
for k_dataset, v_dataset_sz in inesc_sizes["dyn-inesc"].items():
	#print(k_dataset, v_dataset_sz)
	#print(type(k_dataset), type(v_dataset_sz))
	k2_tree_df.loc[(k2_tree_df["dataset"] == k_dataset) & (k2_tree_df["version"] == "dyn-inesc"), 'k2_size'] = v_dataset_sz

upc_sizes = get_k2_sizes(dyn_upc_k2_files)
#print(upc_sizes)
for k_dataset, v_dataset_sz in upc_sizes["dyn-upc"].items():
	#print(k_dataset, v_dataset_sz)
	k2_tree_df.loc[(k2_tree_df["dataset"] == k_dataset) & (k2_tree_df["version"] == "dyn-upc"), 'k2_size'] = v_dataset_sz



# Make synthetic dataset names easily-readable.
k2_tree_df = k2_tree_df.replace("10000000", "10-M")
k2_tree_df = k2_tree_df.replace("1000000", "1-M")
k2_tree_df = k2_tree_df.replace("100000", "100-K")




# 273143316 Jun 28 15:20 prepared_datasets/dmgen/10000000/10000000.kt
# 101033696 Jun 28 13:31 prepared_datasets/dmgen/1000000/1000000.kt
# 7347248 Jun 28 13:23 prepared_datasets/dmgen/100000/100000.kt
# 271446792 Jun 28 02:30 prepared_datasets/webgraph/eu-2015-host/eu-2015-host.kt
# 6329672 Jun 27 23:54 prepared_datasets/webgraph/in-2004/in-2004.kt
# 1136540 Jun 27 23:48 prepared_datasets/webgraph/uk-2007-05@100000/uk-2007-05@100000.kt

# Prepare dataset vertex count |V|, edge count |E|.
k2_tree_df['|V|'] = 0
k2_tree_df['|E|'] = 0


# The field 'k2_size' is filled for all datasets with the size of the static k2-tree format.
# Later in this script, we scan the results directory for the k2-tree formats of dyn-inesc
# and dyn-upc.
k2_tree_df.loc[k2_tree_df['dataset'] == '10-M', '|V|'] = 10000000
k2_tree_df.loc[k2_tree_df['dataset'] == '10-M', '|E|'] = 310748104
k2_tree_df.loc[(k2_tree_df['dataset'] == '10-M') & (k2_tree_df["version"] == "static"), 'k2_size'] = 273143316

k2_tree_df.loc[k2_tree_df['dataset'] == '1-M', '|V|'] = 1000000
k2_tree_df.loc[k2_tree_df['dataset'] == '1-M', '|E|'] = 27416218
k2_tree_df.loc[(k2_tree_df['dataset'] == '1-M') & (k2_tree_df["version"] == "static"), 'k2_size'] = 101033696

k2_tree_df.loc[k2_tree_df['dataset'] == '100-K', '|V|'] = 100000
k2_tree_df.loc[k2_tree_df['dataset'] == '100-K', '|E|'] = 2593718
k2_tree_df.loc[(k2_tree_df['dataset'] == '100-K') & (k2_tree_df["version"] == "static"), 'k2_size'] = 7347248

k2_tree_df.loc[k2_tree_df['dataset'] == 'eu-2015-host', '|V|'] = 11264052
k2_tree_df.loc[k2_tree_df['dataset'] == 'eu-2015-host', '|E|'] = 386915963
k2_tree_df.loc[(k2_tree_df['dataset'] == 'eu-2015-host') & (k2_tree_df["version"] == "static"), 'k2_size'] = 271446792

k2_tree_df.loc[k2_tree_df['dataset'] == 'in-2004', '|V|'] = 1382908
k2_tree_df.loc[k2_tree_df['dataset'] == 'in-2004', '|E|'] = 16917053
k2_tree_df.loc[(k2_tree_df['dataset'] == 'in-2004') & (k2_tree_df["version"] == "static"), 'k2_size'] = 6329672

k2_tree_df.loc[k2_tree_df['dataset'] == 'uk-2007-05@100000', '|V|'] = 100000
k2_tree_df.loc[k2_tree_df['dataset'] == 'uk-2007-05@100000', '|E|'] = 3050615
k2_tree_df.loc[(k2_tree_df['dataset'] == 'uk-2007-05@100000') & (k2_tree_df["version"] == "static"), 'k2_size'] = 1136540

# Printing a full pandas DataFrame:
# https://stackoverflow.com/questions/19124601/pretty-print-an-entire-pandas-series-dataframe
if args.debugging and args.verbose:
	with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
		df = k2_tree_df[["dataset", "scenario", "version", "k2_size"]]
		print(df.sort_values(by=["version", "dataset"]))


def make_dataset_latex_table(df: pd.DataFrame, out_dir: str, debugging: bool = False) -> None:
	pass
 

def plot_version_disk_k2_size(df: pd.DataFrame, out_dir: str, debugging: bool = False) -> None:
	
	
	# Selecting sub-columns for plotting.
	# https://medium.com/dunder-data/selecting-subsets-of-data-in-pandas-6fcd0170be9c
	df = df[["dataset", "|V|", "version", "max_resident_set_size", "k2_size"]]

	# Sorting pandas DataFrame by columns.
	# https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DataFrame.sort_values.html
	#df = df.sort_values(by=["dataset", "version"])
	df = df.sort_values(by=["|V|", "dataset", "version"], ascending=True)


	# Filter datasets for which there was only a single system executing.
	dups_table = df.pivot_table(index=['dataset'], aggfunc='size')
	my_colnames = []
	for dataset, cnt in dups_table.to_frame().to_records():
		if cnt == 1:
			my_colnames.append(str(dataset))
	df = df.loc[np.logical_not(df["dataset"].isin(my_colnames))]

	df_resident_mem_and_k2_disk_sz = df[(df['version'] == "static")]

	#print("STATIC FILTER")
	#with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
	#	print(df_resident_mem_and_k2_disk_sz[["version", "dataset", "max_resident_set_size", "k2_size"]])

	

	df_resident_mem_and_k2_disk_sz = df_resident_mem_and_k2_disk_sz.assign(version="static-on-disk")


	df_resident_mem_and_k2_disk_sz.loc[df_resident_mem_and_k2_disk_sz["version"] == "static-on-disk", "max_resident_set_size"] = df_resident_mem_and_k2_disk_sz["k2_size"] / (1024 * 1024)



	#print("MAX REPLACE")
	#with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
	#	print(df_resident_mem_and_k2_disk_sz[["version", "dataset", "max_resident_set_size", "k2_size"]])


	



	#df_memory_disk_concat = pd.concat([df, df_resident_mem_and_k2_disk_sz]).round(3)
	df_memory_disk_concat = pd.concat([df, df_resident_mem_and_k2_disk_sz]).round(3)

	df_memory_disk_concat = df_memory_disk_concat.sort_values(by=["|V|", "dataset", "version"], ascending=True)


	df_memory_disk_concat = df_memory_disk_concat.drop_duplicates(subset=["dataset", "version", "k2_size"])


	df_memory_disk_concat = df_memory_disk_concat[(df_memory_disk_concat['version']!="static-on-disk")]

	with pd.option_context("display.max_rows", None, "display.max_columns", None):  # more options can be specified also
		print(df_memory_disk_concat)





	# Plot memory usage.
	ylabel = "Disk size (MB)"
	title = r"$\it{k^{2}}$-tree disk size for all versions"

	k2_disk_size_path = os.path.join(out_dir, "k2-disk-sizes.png")
	version_plot(df_memory_disk_concat, "dataset", "version", "k2_size", ylabel, title, False, k2_disk_size_path, debugging)

	ylabel = "Disk log size (MB)"
	k2_disk_size_path = os.path.join(out_dir, "k2-disk-sizes-log.png")
	version_plot(df_memory_disk_concat, "dataset", "version", "k2_size", ylabel, title, True, k2_disk_size_path, debugging)

make_dataset_latex_table(k2_tree_df, args.out_dir, args.debugging)

plot_version_comparison(k2_tree_df, args.out_dir, "AL", args.debugging)
plot_version_comparison(k2_tree_df, args.out_dir, "A", args.debugging)

# NOTE: we are not executing this one because the dynamic implementation of UPC crashes when
# trying to delete edges - we have nothing to compare our version against.
plot_version_comparison(k2_tree_df, args.out_dir, "AD", args.debugging)

plot_scenario_comparison_times(k2_tree_df, args.out_dir, "dyn-inesc", args.debugging)
plot_scenario_comparison_times(k2_tree_df, args.out_dir, "dyn-upc", args.debugging)


plot_version_disk_k2_size(k2_tree_df, args.out_dir, args.debugging)


# Write to Excel.
excel_out_path = os.path.join(args.out_dir, "k2-tree.xlsx")
k2_tree_df.to_excel(excel_out_path)


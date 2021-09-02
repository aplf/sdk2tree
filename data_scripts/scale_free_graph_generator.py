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
import datetime
import os
import pathlib # https://docs.python.org/3.6/library/pathlib.html#pathlib.Path.mkdir
import sys


# 2. related third party imports
import networkx as nx

# 3. custom local imports
# NONE

###########################################################################
############################# READ ARGUMENTS ##############################
###########################################################################


# The class argparse.RawTextHelpFormatter is used to keep new lines in help text.
DESCRIPTION_TEXT = "Scale-free graph generator to evaluate dynamic k2-trees"
parser = argparse.ArgumentParser(description=DESCRIPTION_TEXT, formatter_class=argparse.RawTextHelpFormatter)


# GraphBolt/PageRank-specific parameters.
#parser.add_argument("-i", "--input-file", help="dataset name.", required=True, type=str)
#parser.add_argument("-cache-dir", help="cache directory name.", required=False, type=str, default="")
#parser.add_argument("-temp-dir", help="temporary directory name.", required=False, type=str, default="")

parser.add_argument("-out-dir", help="base output directory where directories for statistics, RBO results, logging, evaluation and figures will be created.", required=True, type=str, default="")
parser.add_argument("-n", help="set desired vertex count", required=True, type=int, default=100)

#parser.add_argument("-damp", "--dampening", help="set desired PageRank dampening factor.", required=False, type=float, default=0.85)
#parser.add_argument("-iterations", help="set desired PageRank power-method iteration count.", required=False, type=int, default=30)

#parser.add_argument("-delete-edges", help="should edge deletions be sent in the stream?", required=False, action="store_true") # if ommited, default value is false

args = parser.parse_args()

# Sanitize arguments and exit on invalid values.

""" if args.iterations <= 0 or not isinstance(args.iterations, int):
    print("> '-iterations' must be a positive integer. Exiting.")
    sys.exit(1)
if args.dampening <= 0 or not isinstance(args.dampening, float):
    print("> '-dampening' must be a positive float in ]0; 1[. Exiting.")
    sys.exit(1) """

#if (args.size <= 0 and args.size_percent < 0) or not isinstance(args.size, int):
if args.n <= 0 or not isinstance(args.n, int):
    print("> '-n' must be a positive integer. Exiting.")
    sys.exit(1)

if args.out_dir.startswith('~'):
    args.out_dir = os.path.expanduser(args.out_dir).replace('\\', '/')
if len(args.out_dir) == 0:
    print("> '-out-dir' must be a non-empty string. Exiting")
    sys.exit(1)


print("> Arguments:\t\t\t{}".format(args))


# Create output directory if it doesn't exist.

if not os.path.exists(args.out_dir):
    print("> Creating output directory:\t\t'{}'".format(args.out_dir))
    pathlib.Path(args.out_dir).mkdir(parents=True, exist_ok=True)
else:
    print("> Output directory found:\t'{}'".format(args.out_dir))


#############



#st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
#print st
#2012-12-15 01:21:05

G = nx.scale_free_graph(args.n)

file_stamp = str(datetime.datetime.now()).split('.')[0].replace(' ', '_').replace(':', '-')
target_file_name = str(args.n) + "V_" + str(G.number_of_edges()) + "E_"  + file_stamp + ".tsv"
target_file_path = os.path.join(args.out_dir, target_file_name)

print("> Target edge file:\t\t{}".format(target_file_path))

with open(target_file_path, 'wb') as f:
    nx.write_edgelist(G, f, data = False)

#print(target_file_name)

#print(G.edges)
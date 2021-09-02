#!/usr/bin/env python3
__copyright__ = """ Copyright 2018 Miguel E. Coimbra

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

# Shuffle a given file's lines.
# Used to shuffle edges in .tsv files.

###########################################################################
################################# IMPORTS #################################
###########################################################################

# PEP 8 Style Guide for imports: https://www.python.org/dev/peps/pep-0008/#imports
# 1. standard library imports
import argparse
import os
import random
import sys
import typing

# 2. related third party imports
# 3. custom local imports
def rreplace(s: str, old: str, new: str, occurrence: str) -> str:
    li = s.rsplit(old, occurrence)
    return new.join(li)

###########################################################################
############################# READ ARGUMENTS ##############################
###########################################################################

# The class argparse.RawTextHelpFormatter is used to keep new lines in help text.
DESCRIPTION_TEXT = "K2Tree edge removal generator. Take a stream of edge additions and generate a sequence of removals for x% of the added edges."
parser = argparse.ArgumentParser(description=DESCRIPTION_TEXT, formatter_class=argparse.RawTextHelpFormatter)
parser.add_argument('-i', '--input-file', help="dataset name.", required=True, type=str)
parser.add_argument('-o', '--out-dir', help="output directory.", required=False, type=str)
parser.add_argument('-r', '--remove-edges', help="generate edge removals.", required=False, action="store_true")
parser.add_argument('-l', '--list-neighbours', help="generate vertex neighbour listing.", required=False, action="store_true")
parser.add_argument('-c', '--check-edges', help="generate edge existence checks.", required=False, action="store_true")
parser.add_argument('-d', '--debug', help='debug mode outputs helpful information.', required=False, action="store_true")

args = parser.parse_args()

### Sanitize arguments.

# Output directory checks.
out_dir = None
if (not args.out_dir is None):
	if len(args.out_dir) == 0:
		print("> -out-dir must be a non-empty string. Exiting.")
		sys.exit(1)
	elif not (os.path.exists(args.out_dir) and os.path.isdir(args.out_dir)):
		print("> Provided output directory does not exist: {}. Exiting.".format(args.out_dir))
		sys.exit(1)
	out_dir = args.out_dir

# Check input file.
if len(args.input_file) == 0:
	print("> -input-file must be a non-empty string. Exiting.")
	sys.exit(1)
elif not os.path.exists(args.input_file):
	print("> Provided input file does not exist: {}. Exiting.".format(args.input_file))
	sys.exit(1)
elif os.path.isdir(args.input_file):
	print("> Provided input file was a directory but should be a file: {}. Exiting.".format(args.input_file))
	sys.exit(1)

# Check generator arguments.
if args.remove_edges is False and args.list_neighbours is False and args.check_edges is False:
	print("> At least one of the flags '-r/-remove-edges' or '-l/-list-neighbours' or '-c/-check-edges' must be provided. Exiting.")
	sys.exit(1)


# If not provided, output directory should be the one
# where the input file is located.
if out_dir is None:
	path_sep_index = args.input_file.rfind(os.path.sep)

	if path_sep_index == -1:
		out_dir = "."
	else:
		out_dir =  args.input_file[0:path_sep_index]

# Base file name to use for the output files.
file_base = args.input_file[args.input_file.rfind(os.path.sep) + 1:]
print("> File base name:\t{}".format(file_base))

# Check filetype.
file_type = args.input_file[args.input_file.rfind('.'):]
print("> File type:\t\t{}".format(file_type))

with open(args.input_file, 'r') as dataset:
	edges = dataset.readlines()

	additions_list = [e for e in edges if e.startswith('a')]

	v_map = set()
	for add_str in additions_list:
		_, src, dst = add_str.strip().split()
		v_map.add(int(src))
		v_map.add(int(dst))

	add_ctr = len(additions_list)

	print('> Input file had {} edge additions.'.format(add_ctr))

	candidates_list = random.sample(additions_list, int(add_ctr/2))
	
	# Write edge removals file if requested.
	if args.remove_edges:
		removals_name = rreplace(file_base, file_type, '-deletions' + file_type, 1)
		removals_out_path = os.path.join(out_dir, removals_name)
		print('\n> Removals file:\t{}'.format(removals_out_path))
		print('> #removals = {}'.format(int(add_ctr/2)))
		with open(removals_out_path, 'w') as out_file:
			out_file.write(''.join([c.replace('a', 'd') for c in candidates_list]))

	# Write edge checks file if requested.
	if args.check_edges:
		checks_name = rreplace(file_base, file_type, '-checks' + file_type, 1)
		checks_out_path = os.path.join(out_dir, checks_name)
		print('\n> Checks file:\t{}'.format(checks_out_path))
		
		print('> #checks = {} (sampled from additions)'.format(int(add_ctr/2)))

		# Generate random non-sampled edge checks.
		max_v = max(v_map)
		min_v = min(v_map)

		print('> min_v = {}'.format(str(min_v)))
		print('> max_v = {}'.format(str(max_v)))


		for i in range(int(add_ctr/2)):

			src_id = random.randint(min_v, max_v)
			dst_id = random.randint(min_v, max_v)

			#print('> check: ({}, {})'.format(src_id, dst_id))

			candidates_list.append('l {} {}\n'.format(
				src_id, 
				dst_id))

		print('> #checks = {} (sampled + random)'.format(len(candidates_list)))

		with open(checks_out_path, 'w') as out_file:
			out_file.write(''.join([c.replace('a', 'l') for c in candidates_list]))


	v_ctr = len(v_map)
	candidates_list = random.sample(additions_list, int(v_ctr))
	listing_count = int(v_ctr)
	#candidates_list = random.sample(additions_list, int(add_ctr/2))

	#print(v_map)

	# Write vertex listings file if requested.
	if args.list_neighbours:
		lists_name = rreplace(file_base, file_type, '-lists' + file_type, 1)
		lists_out_path = os.path.join(out_dir, lists_name)
		print('\n> Lists file:\t\t{}'.format(lists_out_path))
		print('> #listings = {}'.format(listing_count))

		with open(lists_out_path, 'w') as out_file:
			l_list = ['\t'.join(c.replace('a', 'n').split()[:-1]) for c in candidates_list]
			out_file.write('\n'.join(l_list) + '\n')
#!/bin/python

# Description: Creates a trivial package file from a list of files
#
# The package file contains the source files concatenated, with headers that allows their
# easy extraction. The files starts with a number (long int) indicating the number of the individual
# files contained in the package. Following this number is a set of headers, each header describes
# one file in the package, and comprises of a filename, a file size and flags (reserved).
# After the headers, the source files are concatenated one after another.
#
# Invocation: pack.py [-o pakfile] file1 file2 ....

import sys, getopt
import struct, os
import os.path

if sys.argv[1] == '-o':
	out_file = file(sys.argv[2], 'wb')
	in_filename_list = sys.argv[3:]
else:
	out_file = sys.stdout
	in_filename_list = sys.argv[1:]

def create_header(filename, size, flags):
	h = struct.pack("24sII", filename, size, flags)
	return h

# Write the file number
num_files = len(in_filename_list)
out_file.write(struct.pack("I", num_files))
print("There are %d files in the package" % num_files)

# Write the headers
file_list = []
for ifn in in_filename_list:
	# If the filename contains colon, the written filename is different
	# from the source name
	
	if ":" in ifn:
		(source_fn, dest_fn) = ifn.split(':')
	else:
		source_fn = ifn
		dest_fn = os.path.split(ifn)[1] # Remove the dir name
	
	size = os.stat(source_fn).st_size
	header = create_header(dest_fn, size, 0)
	out_file.write(header)
	
	file_list.append({
		'source_fn': source_fn,
		'dest_fn': dest_fn,
		'size': size
		})

	print("Wrote header for %s (as %s), size %d" % (source_fn, dest_fn, size))

# Copy the source file content
for fe in file_list:
	filename = fe['source_fn']
	data = file(filename, 'rb').read()
	out_file.write(data)
	print("File %s has been written" % (filename,))



#!/usr/bin/env python
# author: Ivan Senin 

from __future__ import division
import sys
import os
import string as s
from random import choice
from random import randint

def usage():
	filename = sys.argv[0]
	print "Program generates file of exact size with multiple lines of characters"
	print filename, ": usage:"
	print filename, " <output file> <size[G|K|B(default)]>"
	print "\nExample: ", filename, " input.txt 1G\t# Creates 1 Gb file"

def parse_size(size_str):
	number = int(filter(lambda c: c.isdigit(), size_str))
	qualifier = size_str[-1].upper()

	multiplicator_pow = {'G' : 3, 'M' : 2, 'K' : 1 }
	if qualifier in multiplicator_pow:
		pow_d = multiplicator_pow[qualifier]
		number *= 1024**pow_d
	return number

def main():
	argc = len(sys.argv)
	if argc < 3:
		usage()
		return

	out = open(sys.argv[1], "w")
	data_size = parse_size(sys.argv[2])

	alphabet = s.digits + s.ascii_letters + s.punctuation + ' ' 
	# alphabet = s.digits + s.ascii_letters
	max_line_len = min(1000, int(data_size / 10)) + 1
	assert data_size >= 0, "set valid data size"
	assert not out.closed, "cannot create output file"
	counter = 0
	generated_size = 0
	print "Start file generation..."
	while generated_size + 1 < data_size:
		line_len = min(data_size - generated_size, randint(1, max_line_len)) - 1
		line = (''.join(choice(alphabet) for i in range(line_len)))
		out.write(line)
		out.write('\n')

		generated_size += line_len + 1
		counter += 1
		if 10**4 == counter:
			print "Generating in progress:\t{:.1f}".format(generated_size / data_size * 100), " %"
			counter = 0


	print "Generating done."

	out.close()

main()
#!/bin/python

from socket import *
import struct
import sys

PORT = 24605

if len(sys.argv)==2:
	threshold = int(sys.argv[1])
else:
	threshold = 0

s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', PORT))

n = 0
while(True):
	t = s.recv(4)
	n += 1
	value, = struct.unpack("i", t)
	if value > threshold:
		print "%10d %d" % (n, value)


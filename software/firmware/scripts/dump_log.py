#!/bin/python

from socket import *

PORT = 24600

s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', PORT))

while(True):
	t, addr = s.recvfrom(200)
	print "[%s] %s" % (addr[0], t.strip())



#!/bin/python

from socket import *

PORT = 24600

s = socket(AF_INET, SOCK_DGRAM)
s.bind(('', PORT))

while(True):
	t = s.recv(200)
	print t.strip()


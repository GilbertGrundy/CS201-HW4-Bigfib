#!/usr/bin/python3

from sys import argv

b1 = int(argv[1], 16)
b2 = int(argv[2], 16)

print("{:x}".format(b1 + b2))

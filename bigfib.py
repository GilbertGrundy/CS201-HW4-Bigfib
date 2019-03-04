#!/usr/bin/python3

from sys import argv

n = int(argv[1])

b1 = 1
b2 = 1
for _ in range(2, n):
    b = b1 + b2
    b1 = b2
    b2 = b
print("{:x}".format(b2))

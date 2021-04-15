#!/usr/bin/env python3

# coding: utf-8
import sys

if len(sys.argv) != 3:
    print("usage: %s TEMPLATE OUTPUT" % sys.argv[0])
    sys.exit()

with open(sys.argv[1], "r") as f:
    data = f.read().strip().split("\n")

offsets = {"i_pt": 0, "i_key": 64}
    
data_ = []
for d in data:
    ds = d.split(":")
    off = offsets.get(ds[0][:-1])
    res = d
    if off != None:
        res = ":".join(ds[:-1]) + ": share " + str(int(ds[1]) + off)
    elif ds[0] == "i_r":
        res = ":".join(ds[:-1]) + ": random"
    data_.append(res)
        
with open(sys.argv[2], "w+") as f:
    for d in data_:
        f.write(d + "\n")

# coding: utf-8
import sys

if len(sys.argv) != 3:
    print("usage: %s TEMPLATE OUTPUT" % sys.argv[0])
    sys.exit()

with open(sys.argv[1], "r") as f:
    data = f.read().strip().split("\n")

offsets = {"PTxDI": 0, "KxDI": 128}
randoms = ("Zmul", "Zinv", "Bmul", "Binv")
    
data_ = []
for d in data:
    ds = d.split(":")
    off = offsets.get(ds[0])
    res = d
    if off != None:
        adjust = -8 if ((int(ds[1]) // 8) % 2 == 1) else 0
        base = int(ds[1]) // 16 * 8
        disp = int(ds[1]) % 16 + adjust
        res = ":".join(ds[:-1]) + ": share " + str(base + disp + off)
    elif any(map(lambda x: ds[0].startswith(x), randoms)):
        res = ":".join(ds[:-1]) + ": random"
    data_.append(res)
        
with open(sys.argv[2], "w+") as f:
    for d in data_:
        f.write(d + "\n")

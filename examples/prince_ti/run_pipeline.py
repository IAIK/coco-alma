#!/usr/bin/env python3

import os, sys
import subprocess as sp

SECURE = False
if len(sys.argv) == 2:
    SECURE = (sys.argv[1] == "secure")

PRINCE_DIR = os.path.dirname(os.path.realpath(__file__))
DESIGN_DIR = PRINCE_DIR + "/design"
ALMA_DIR = "/".join(PRINCE_DIR.split("/")[:-2])
TMP_DIR = ALMA_DIR + "/tmp"

##### PARSING

design_files = set(os.listdir(DESIGN_DIR))
if not SECURE:
    try: design_files.remove("top_module_d11_secure.sv")
    except KeyError: pass
else:
    try: design_files.remove("top_module_d11.sv")
    except KeyError: pass
    
file_list = "\n    ".join([DESIGN_DIR + "/" + x for x in design_files])

parse = \
"""
python3 %s/parse.py --log-yosys 
    --top-module top_module_d11 
    --source 
    %s
""" % (ALMA_DIR, file_list)

print(parse)
res = sp.call(parse.split())

if res:
    print("Parsing failed")
    sys.exit(res)

##### TRACING

trace = \
"""
python3 %s/trace.py 
    --testbench %s/verilator_tb%s.cpp 
    --netlist %s/circuit.v
""" % (ALMA_DIR, PRINCE_DIR, ("_secure" if SECURE else ""), TMP_DIR)

print(trace)
res = sp.call(trace.split())

if res:
    print("Tracing failed")
    sys.exit(res)

##### LABELING

labels = \
"""
python3 %s/generate_labels.py %s/labels.txt %s/my-labels.txt
""" % (PRINCE_DIR, TMP_DIR, TMP_DIR)

print(labels)
res = sp.call(labels.split())

if res:
    print("Labeling failed")
    sys.exit(res)

##### VERIFICATION

verify = \
"""
python3 %s/verify.py 
    --json %s/circuit.json 
    --vcd %s/tmp.vcd
    --label %s/my-labels.txt 
    --rst-name i_reset 
    --cycles 3 
    --mode transient 
    --probe-duration once 
    --num-leaks 1
""" % (ALMA_DIR, TMP_DIR, TMP_DIR, TMP_DIR)
# --trace-stable

print(verify)
res = sp.call(verify.split())

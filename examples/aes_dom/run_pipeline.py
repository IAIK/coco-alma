import os, sys
import subprocess as sp

AES_DIR = os.path.dirname(os.path.realpath(__file__))
DESIGN_DIR = AES_DIR + "/design"
ALMA_DIR = "/".join(AES_DIR.split("/")[:-2])
TMP_DIR = ALMA_DIR + "/tmp"

##### PARSING

parse = \
"""
python3 %s/parse.py --log-yosys 
    --top-module aes_wrapper 
    --source %s %s
""" % (ALMA_DIR, AES_DIR + "/tmp/circuit.v", AES_DIR + "/aes_dom_wrapper.v")

print(parse)
res = sp.call(parse.split())

if res:
    print("Parsing failed")
    sys.exit(res)

##### TRACING

trace = \
"""
python3 %s/trace.py 
    --testbench %s/verilator_tb.cpp 
    --netlist %s/circuit.v
""" % (ALMA_DIR, AES_DIR, TMP_DIR)

print(trace)
res = sp.call(trace.split())

if res:
    print("Tracing failed")
    sys.exit(res)

##### VERIFICATION

verify = \
"""
python3 %s/verify.py 
    --top-module aes_wrapper 
    --json %s/circuit.json 
    --vcd %s/tmp.vcd
    --label %s/labels.txt 
    --rst-name RstxBI
    --rst-phase 0
    --cycles 23
    --mode transient 
    --probe-duration always
""" % (ALMA_DIR, TMP_DIR, TMP_DIR, AES_DIR)

print(verify)
res = sp.call(verify.split())

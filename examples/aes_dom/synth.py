import os, sys
import subprocess as sp

AES_DIR = os.path.dirname(os.path.realpath(__file__))
DESIGN_DIR = AES_DIR + "/design"
ALMA_DIR = "/".join(AES_DIR.split("/")[:-2])
TMP_DIR = AES_DIR + "/tmp"
TEMPLATE_DIR = ALMA_DIR + "/templates/"

SYNTH_FILE_PATH = TMP_DIR + "/yosys_synth.ys"
TEMPLATE_FILE_PATH = TEMPLATE_DIR + "/yosys_synth_template.txt"

yosys_cmd = \
"""
docker run --rm -t 
  -v %s:/src
  -v %s:/tmp 
  -w /src 
  hdlc/ghdl:yosys 
  yosys -m ghdl /tmp/yosys_synth.ys
""" % (DESIGN_DIR, TMP_DIR)

parse = \
"""
python3 %s/parse.py
    --synthesis-file yosys_synth.ys
    --top-module aes_top
    --yosys
""" % (ALMA_DIR)

design_files = os.listdir(DESIGN_DIR)
design_files = [d for d in design_files if d.endswith(".vhdl")]

def create_yosys_script():
    yosys_script = ""
    with open(TEMPLATE_FILE_PATH) as template_file:
        yosys_script += template_file.read()
    assert("{READ_FILES}" in yosys_script)
    read_verilog_commands = "ghdl %s -e aes_top ;\n\n" % " ".join(design_files)
    yosys_script = yosys_script.replace("{READ_FILES}", read_verilog_commands)
    assert("{TOP_MODULE}" in yosys_script)
    yosys_script = yosys_script.replace("{TOP_MODULE}", "aes_top")
    assert("{JSON_FILE_PATH}" in yosys_script)
    yosys_script = yosys_script.replace("{JSON_FILE_PATH}", "/tmp/circuit.json")
    assert("{NETLIST_FILE_PATH}" in yosys_script)
    yosys_script = yosys_script.replace("{NETLIST_FILE_PATH}", "/tmp/circuit.v")
    with open(SYNTH_FILE_PATH, "w") as f:
        f.write(yosys_script)

if __name__ == "__main__":
    create_yosys_script()
    yosys_cmd = yosys_cmd.split()
    res = sp.run(args=yosys_cmd, capture_output=True)
    if res.returncode:
        print("Parsing failed:")
        print(res.stdout.decode("utf-8"))
        sys.exit(res.returncode)
    

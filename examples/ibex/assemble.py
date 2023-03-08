import argparse
import subprocess as sp
import os, time, re, sys, json
import binascii as ba

IBEX_CFG_DIR = os.path.dirname(os.path.realpath(__file__))
TMP_DIR = "/".join(IBEX_CFG_DIR.split("/")[:-2]) + "/tmp"
CONFIG = IBEX_CFG_DIR + "/config.json"
# parsed automatically
ASM_CMD = None
OBJDUMP_CMD = None
SECURE_MEM = None      
MEM_WIDTH = 1
MEM_MODULE = None
INSTR_LIMIT = None
TOP_MODULE = None

def check_file_exists(file_path):
    if file_path == None: return None
    if not os.path.isfile(file_path):
        raise argparse.ArgumentTypeError("File '%s' does not exist" % file_path)
    return file_path

def check_dir_exists(dir_path):
    if not os.path.isdir(dir_path):
        print("ERROR: Directory %s does not exist" % dir_path)

try:
    with open(CONFIG, "r") as f:
        opts = json.load(f)
        OBJDUMP_CMD = opts.get("objdump")
        ASM_CMD = opts.get("asm")
except FileNotFoundError as e:
    print(e)

if not isinstance(ASM_CMD, str) or not isinstance(OBJDUMP_CMD, str):
    print("Invalid config.json file contents")
    sys.exit(1)

ASM_CMD = ASM_CMD.split()
OBJDUMP_CMD = OBJDUMP_CMD.split()
check_file_exists(ASM_CMD[0])
check_file_exists(OBJDUMP_CMD[0])

def parse_arguments():
    global SECURE_MEM, MEM_WIDTH, MEM_MODULE, INSTR_LIMIT, TOP_MODULE
    parser = argparse.ArgumentParser(description="Assemble.py for ibex")
    parser.add_argument("--program", dest="program_path", required=True)
    parser.add_argument('--init-file', dest='init_file_path', required=False, default=None)
    parser.add_argument("--build-dir", dest="build_dir_path", required=False, default=TMP_DIR)
    parser.add_argument("--netlist", dest="netlist_path", required=True)
    parser.add_argument('--uppercase', dest='uppercase', default=False, action='store_true')
    args = parser.parse_args()
    check_file_exists(args.program_path)
    check_file_exists(args.init_file_path)
    check_dir_exists(args.build_dir_path)
    check_file_exists(args.netlist_path)
    
    with open(args.netlist_path, "r") as f:
        verilog_txt = f.read()
        mod = re.compile("module (\S+)\(")
        res = mod.search(verilog_txt)
        if res is None:
            print("Could not find top module, aborting")
            sys.exit(-1)
        TOP_MODULE = res.group(1)
        SECURE_MEM = "instr_rom" in verilog_txt
        MEM_WIDTH = 4 if SECURE_MEM else 1
        MEM_MODULE = "instr_rom" if SECURE_MEM else "u_ram"
        rax = re.compile("%s.mem\[([0-9]+)\]" % MEM_MODULE)
        INSTR_LIMIT = max([int(x) for x in rax.findall(verilog_txt)]) + 1
    return args


def read_objfile(args):
    p = sp.Popen(OBJDUMP_CMD + ["-s", "-j", ".text", args.build_dir_path + "/program.o"],
                 stdout=sp.PIPE, stderr=sp.PIPE)
    p.wait()
    data = p.stdout.read().decode("ascii").strip().split("\n")
    curr = 1
    while("section" not in data[curr - 1]): curr += 1
    data = [d.strip()[:d.find("  ")].split()[1:] for d in data[curr:]]
    data = "".join(["".join(d) for d in data])
    return ba.unhexlify(data)
    

def create_raminit_header(args):
    p = sp.Popen(ASM_CMD + [args.program_path, "-o", args.build_dir_path + "/program.o"],
                 stdout=sp.PIPE, stderr=sp.PIPE)
    p.wait()
    print((p.stdout.read() + p.stderr.read()).decode("ascii"))

    code = read_objfile(args)
    if len(code) > INSTR_LIMIT * MEM_WIDTH:
        print(".text section is too large (> %d bytes)" % (INSTR_LIMIT * MEM_WIDTH))
        sys.exit(-1)
    
    header = open(args.build_dir_path + "/ram_init.h", "w")
    header.write("void load_prog(Testbench<Vcircuit>* tb) {\n")

    mangle = "__%03X" if args.uppercase else "__%03x"
    mgl = lambda x: (mangle % ord(x))

    for i in range(0, len(code), MEM_WIDTH):
        x = "0x" + ba.hexlify(code[i:i+MEM_WIDTH][::-1]).decode("ascii")
        header.write(f"  tb->m_core->{TOP_MODULE}__DOT__{MEM_MODULE}{mgl('.')}"
                     f"mem{mgl('[')}{i // MEM_WIDTH}{mgl(']')} = {x};\n")
    
    # parse data init file with format addr/reg ; value    
    reg, mem = [], []
    if args.init_file_path is not None:
        data = None
        with open(args.init_file_path, "r") as f:
            data = f.read().strip().split("\n")
        data = [d.split(";") for d in data]
        reg = [d for d in data if d[0].startswith("x")]
        mem = [d for d in data if not d[0].startswith("x")]
    
    for m in mem:
        addr, val = int(m[0], 0) // MEM_WIDTH, m[1]
        header.write(f"  tb->m_core->{TOP_MODULE}__DOT__u_ram{mgl('.')}mem{mgl('[')}{addr}{mgl(']')} = {val};\n")
    header.write("  tb->reset();\n")
    for r in reg:
        addr, val = r[0][1:], r[1]
        header.write(f"  tb->m_core->{TOP_MODULE}__DOT__u_core{mgl('.')}register_file_i{mgl('.')}"
                     f"rf_reg_tmp{mgl('[')}{addr}{mgl(']')} = {val};\n")
        
    header.write("}\n")
    header.close()

    
def create_verilator_testbench(args):
    with open(IBEX_CFG_DIR + "/verilator_tb_template.txt", "r") as f:
        template = f.read()
    
    tb_path = args.build_dir_path + "/verilator_tb.c"
    vcd_path = args.build_dir_path + "/circuit.vcd"
    
    template = template.replace("{VCD_PATH}", vcd_path)
    with open(tb_path, "w+") as f: f.write(template)
    
    print("Wrote verilator testbench to %s" % tb_path)
    print("It produces output VCD at %s" % vcd_path)
    

def main():
    args = parse_arguments()
    print("Using program file: ", args.program_path)
    print("Using initialization file: ", args.init_file_path)
    print("Using build directory: %s" % args.build_dir_path)
    print("Using netlist path: %s" % args.netlist_path)
    
    # Create raminit.h
    create_raminit_header(args)

    # Create verilator testbench
    create_verilator_testbench(args)

if __name__ == "__main__": 
    main()



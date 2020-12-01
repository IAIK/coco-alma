# IBEX masking verification

This directory contains a short tutorial on how to verify masked implementations on the IBEX RISC-V processor.
It should work out-of-the-box for the provided sample programs. If you encounter any problems, feel free to 
open an issue.

## Coco-Ibex

Coco-Ibex is the hardened RISC-V processor discussed in the original paper. For this tutorial you will need
to clone its [code](https://github.com/IAIK/coco-ibex), and synthesize it using a modified synthesis flow.
The Yosys synthesis flow is documented [here](https://ibex-core.readthedocs.io/en/latest/) and is the same for
the hardened processor. Below, we only show a brief summary of what you need to do.

**1. Clone the coco-ibex repository somewhere**
``` bash
git clone https://github.com/IAIK/coco-ibex.git
```
**2. Go into the synthesis directory and run the synthesis**
```
cd coco-ibex/syn/ && ./syn_yosys.sh
```
This will first generate Verilog versions of all necessary modules and then build the netlist in the directory
`syn_out/ibex_current_datetime/generated`. The interesting netlist has the name `ibex_core_netlist.v`. This is
what you will need later on for the parsing and tracing step of Alma. The file `rtl/secure.sv` controls what security
features are enabled or disabled, and might change depending on what you need, see the documentation.

## Prerequesites

Since we are running programs on the synthesised core, we have to first compile them. This requires you to have the
[RISC-V developement toolchain](https://github.com/riscv/riscv-tools). The most important tools you need there
are the `as` RISC-V assembler for generating binaries, and the `objdump` ELF utility for reading them.

In theory, this itself is enough to manually write programs, compile them and load them with a testbench,
however, it is a very slow process when done by hand. This is why we automate it with the `assemble.py` script.
To use it, you have to specify the assembler and objdump commands as installed on your system in `config.json`.

## Verification Flow

The verification flow is mostly the same as described in the root directory. However, there are additional steps
you need to perform before you can trace the execution of the circuit. In the following, we only briefly describe
the main steps.

* Synthesise the `ibex_top` circuit, which includes the coco-ibex core, and memory modules.
```bash
python3 path_to/parse.py -t ibex_top -s \
    path_to/coco-ibex/rtl/secure.sv \
    path_to/coco-ibex/shared/rtl/ibex_top.v \
    path_to/coco-ibex/syn/syn_out/ibex_current_datetime/generated/ibex_core_netlist.v
```
* Compile RISC-V Program and create the header for the testbench.
```bash
python3 path_to/assemble.py \
    --program program.S \
    --netlist path_to/coco-alma/tmp/circuit.v
```
* Label the memory and registers as explained in the root directory.
* Generate the execution trace for the given program.
```bash
python3 path_to/trace.py \
    --testbench path_to/coco-alma/tmp/verilator_tb.c \
    --netlist path_to/coco-alma/circuit.v \
    --output-bin path_to/coco-alma/circuit.elf
```
* Run the verification script.
```bash
python3 verify.py \
    --json path_to/coco-alma/tmp/circuit.json \
    --label path_to/coco-alma/tmp/labels.txt \
    --vcd path_to/coco-alma/tmp/circuit.vcd \
    --cycles 5 \
    --mode transient \
    --rst-name rst_sys_n \
    --rst-cycles 4 \
    --rst-phase 0 \
```

You can customize most of the artefact file locations generated during this process and then need to
additionally specify them as command line parameters. For more information on what options are supported,
please see the help menus available through `python parse.py --help` and similar.

# Hardware masking verification

This directory contains a short tutorial on how to verify a DOM-masked implementation of the Keccak S-box.

## Keccak-DOM S-box

We provide a Verilog design of a first-order masked Keccak S-box implementation using DOM in the `design` directory.
In order to verify this design, follow the steps described below.

## Verification Flow

In the following, we briefly describe the main steps.
For a simplified procedure, without any custom settings, please run `run_pipeline.py` from the virtual environment.

For manual execution, please run the following steps from the root directory of the repository.
By default, all files will be created and looked for under the coco-alma temporary directory `tmp`.

1. Synthesize the `keccak_sbox` circuit, which is the top module of the Keccak S-box design.
```bash
python3 parse.py --log-yosys \
    --top-module keccak_sbox \
    --source examples/keccak_dom/design/keccak_sbox.v
    --netlist tmp/circuit.v
```

2. Create your own Verilator testbench or use the one provided to generate a trace.
If you use the provided one, the VCD trace file `tmp.vcd` is created in the same directory as the executable of the Verilator simulation.
```bash
python3 trace.py \
    --testbench examples/keccak_dom/verilator_tb.cpp \
    --netlist tmp/circuit.v
```
3. Adapt the label file in `tmp/labels.txt` accordingly or use the one provided.
If you use the provided one, copy it to the `tmp` directory.


4. Run the verification script. 
```bash
python3 verify.py \
    --json tmp/circuit.json \
    --label tmp/my-labels.txt \
    --vcd tmp/tmp.vcd \
    --cycles 3 \
    --mode transient \
    --rst-name i_reset \
    --probing-model time-constrained
```

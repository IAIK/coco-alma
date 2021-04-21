# Hardware masking verification

This directory contains a short tutorial on how to verify a DOM-masked implementation of the AES cipher.
It should work out-of-the-box with the provided testbench. If you encounter any problems, feel free to open an issue.

## AES-DOM

AES-DOM, originally developed by Hannes Gross, is a masked implementation of the AES cipher. For this tutorial we include a fork of the original implementation in the `design` directory.
You can obtain the code by running:
```bash
cd design
git submodule init
git submodule update
```
In this tutorial, we will synthesize the circuit, generate a trace using test-vectors and then attempt to verify the probing resistance. Below, we only show a brief summary of what you need to do.

## Verification Flow

The verification flow is mostly the same as described in the root directory. However, there are additional steps
you need to perform before you can trace the execution of the circuit. In the following, we only briefly describe
the main steps.

For a simplified procedure, without any custom settings, please run `run_pipeline.py` from the virtual environment. The provided pipeline executes all the steps every time although it might not be necessary, so you can adapt it or write your own! For manual execution, please follow the following steps:

* Synthesise the `aes_top` circuit, which is the top module of the AES-DOM design. 
  Since the design was written in VHDL, which Yosys does not support natively, we have to perform a workaround. The GHDL project provides a [plugin for Yosys](https://github.com/ghdl/ghdl-yosys-plugin) that allows it to read and elaborate pure VHDL designs.
  There are multiple ways of obtaining the plugin outlined in their documentation. 
  We focus on the Docker based method. First, obtain the docker image, and then run the provided Python script to synthesize:

```bash
docker pull hdlc/ghdl:yosys
python3 synth.py
```
  The `synth.py` script merely automates the whole process. It prepares a custom synthesis script, and runs it using the Yosys from the docker image.
  This produces a flattened verilog netlist. Since the AES-DOM implementation is pipelined and only gets one plaintext and key share per cycle, we provide an additional wrapper that expects the whole plaintext and key simultaneously.
  We synthesize this `aes_dom_wrapper.v` together with the main circuit using `parse.py`. The example below, saves all the data into coco-alma's temporary directory.
```bash
python3 parse.py --log-yosys \
    --top-module aes_wrapper \
    --source examples/aes_dom/tmp/circuit.v \
             examples/aes_dom//aes_dom_wrapper.v \
    --netlist tmp/circuit.v
```

* Create your own Verilator testbench or use the one we provided to generate a trace. 
  If you use the one we provided the VCD trace file is saved as `tmp.vcd` in the same directory as the executable of the Verilator simulation. 
  The example below uses the default location of the synthesized `circuit.v`.
```bash
python3 trace.py \
    --testbench examples/aes_dom/verilator_tb.cpp \
    --netlist tmp/circuit.v
```
* Create a labeling file for the AES-DOM inputs. We have provided a script that does this:
```bash
python3 generate_labels.py tmp/labels.txt tmp/my-labels.txt
```
* Run the verification script. Replace `tmp/tmp.vcd` with your vcd file location. 
  This program call verifies that there are no leaks in the first round of the cipher. For more rounds, increase the number of considered cycles.
```bash
python3 verify.py \
    --json tmp/circuit.json \
    --vcd tmp/tmp.vcd \
    --label tmp/my-labels.txt \
    --rst-name RstxBI \
    --rst-phase 0 \
    --cycles 23 \
    --mode transient \
    --probe-duration always
```

You can customize most of the artefact file locations generated during this process and then need to
additionally specify them as command line parameters. For more information on what options are supported,
please see the help menus available through `python parse.py --help` and similar.

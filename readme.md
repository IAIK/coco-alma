# CocoAlma: Execution-aware Masking Verification

CocoAlma is an _execution-aware_ tool for formal verification of masked implementations.
It can verify any data-independent masked computation that can be implemented as a Verilog 
hardware circuit or as software running on a hardware platform, with properly labeled secret shares and randomness. 

For software verification on hardware level, please look at the examples provided for the
verification of masked software on [CocoIbex](https://github.com/iaik/coco-ibex) and the associated paper [Coco](https://eprint.iacr.org/2020/1294.pdf).
For pure hardware verification of masked designs you can look at the provided examples for Prince-TI and AES-DOM, which have very distinct implementations. 
Here, AES-DOM is implemented as a pipeline of cascading state registers, whereas Prince-TI is implemented in a straightforward
fashion with threshold masking. Consider also looking at the hardware verification paper [CocoAlma](https://pure.tugraz.at/ws/portalfiles/portal/37924631/paper.pdf) for more details.

If you use CocoAlma, please properly cite the following papers:
```
@inproceedings{hadzic2021cocoalma,
  title={COCOALMA: A Versatile Masking Verifier},
  author={Hadžić, Vedad and Bloem, Roderick},
  booktitle={Conference on Formal Methods in Computer-aided Design -- FMCAD 2021},
  year={2021}
}

@inproceedings{gigerl2021coco,
  title={Coco: Co-design and co-verification of masked software implementations on CPUs},
  author={Gigerl, Barbara and Hadžić, Vedad and Primas, Robert and Mangard, Stefan and Bloem, Roderick},
  booktitle={30th {USENIX} Security Symposium ({USENIX} Security 21)},
  year={2021}
}
```

Below, we provide a set of installation and general usage instructions. 
Each provided example includes a more specific set of instructions for verifying that example, as well as a script that automates the whole process for you.

## Setup and Installation

1. Create a new virtual environment
``` bash
python3 -m venv dev
```

2. Activate the virtual environment
``` bash
source dev/bin/activate
```

3. Install the required python packages
``` bash
pip3 install -r requirements.txt
```

4. Install Yosys >= 0.15 and Verilator >= 4.106:
* **Easy**: install it using your favourite package manager
``` bash
apt install yosys verilator
```
Warning! The latest LTS version of Ubuntu does not provide a sufficiently new version of Yosys.

* **Flexible**: Install then according to the official guides 
[Yosys](https://github.com/YosysHQ/yosys) and [Verilator](https://www.veripool.org/projects/verilator/wiki/Installing).

After following these steps, the only additional thing you need is the hardware 
circuit you want to verify in Verilog or System Verilog.
Yosys does not support some SystemVerilog features your project might make use of.
In that case, either try recompilation into vanilla Verilog with [sv2v](https://github.com/zachjs/sv2v) or get a commercial license for [Symbiotic EDA](https://www.symbioticeda.com/seda-suite).

## Usage

CocoAlma consists of several Python programs that represent different stages of the verification flow.
In the following, we briefly show how each of them is used and give an example based on a 1st-order secure DOM AND (`examples/gadgets/design/dom_and.v`).

### 1. **Parse** the circuit
```
python3 parse.py 
  ( --source VERILOG_FILES | --synthesis-file SYNTHESIS_FILE ) 
  --top-module TOP_MODULE 
  [optional arguments]
```
The arguments for the standard mode of operation are:
  * `--source`: file path(s) to the source file(s). `parse.py` will automatically generate a Yosys synthesis script. Then, the `--synthesis-file` option must not be used.
  * `--synthesis-file`: In case one does not want to use the auto-generated script, this option can be used to specify a custom Yosys synthesis script. Then, the `--source` option must not be used because the script already includes the paths of the sources.
  * `--synthesis-template`: an optional template yosys synthesis script that is used for patching instead of default `template/yosys_synth_template.txt`. The `--source` option can be used with this argument. The difference between `--synthesis-file` and `--synthesis-template` is that the former is directly passed to yosys, while the latter is patched with the source files passed with `--source`. 
  * `--top-module`: the name of the top module
  
Optional arguments include:

  * `--label`: Custom output file path of label file. Default: `alma/tmp/labels.txt`
  * `--json`: Custom output file path of JSON file. Default: `alma/tmp/circuit.json`
  * `--netlist`: Custom output file path of netlist file. Default: `alma/tmp/circuit.v`
  * `--yosys`: `parse.py` will search for the Yosys binary using `which yosys`. In case one wants to use a specific Yosys version, the path can be specified with this option.
  * `--log-yosys`: Yosys synthesis output is written to `alma/tmp/yosys_synth_log.txt` 
  
The outputs of this step are a label file, the circuit in JSON format and the netlist file.

### Example

In order to parse the 1st-order DOM AND, run:
```
python3 parse.py 
  --source examples/gadgets/design/dom_and.v
  --top-module dom_and_1storder
```

This will produce the following files:
* `tmp/circuit.v`
* `tmp/circuit.json`
* `tmp/labels.txt`



### 2. **Label** the respective registers and input ports in the label file

`labels.txt` lists all registers and input ports, which can be labeled as:
* `secret`
* `static_random` or `volatile_random`,
* `unimportant`

The format of each line in `labels.txt` is `(register_port_name) ([range]?) = (purpose) (range?)`
The register or port name `register_port_name` is a mangled name from the original design, `[range]` indicates the range of bits in the signal that you are labeling, 
`purpose` is either `secret`, `static_random`, `volatile_random` or `unimportant`. 
Shares of a secret are handled on bit-level, ie, the tool works with 1-bit shares. 
In order to indicate that a specific register or port contains multiple shares (bits), you can specify a range of secrets instead, and the individual
bits of the signal will be associated with the given secret in the range.

The label `static_random` indicates a single fresh random bit, while the label `volatile_random` indicates that in every cycle, a new fresh random bit is obtained by a port/register (as if connected to a RNG which supplies fresh random data in every cycle).
Assume you have a masked AES S-Box design which consumes 28 random bits per cycle via the 28-bit wide input port `rand_i`.
Labeling the 28 bits as `static_random` will assume that `rand_i` supplies the random data once in the beginning of the computation. Using the label `volatile_random` for it will assume that 28 bits of fresh randomness is supplied to it in every clock cycle, which is especially relevant to verify pipelined designs, in which new input data is fed to the circuit in every clock cycle, as shown in the following timing diagram:

![Example timing random](/doc/rand_timing_example.png)


### Example 1
A valid labeling can look like this:

```
# registers
my_super_cpu.r1 [7:0] = secret 7:0
my_super_cpu.r2 [7:0] = secret 7:0
my_super_cpu.r3 [3:0] = static_random
my_super_cpu.r3 [7:4] = secret 11:8
my_super_cpu.r4 [4:0] = secret 11:8
# ports
rng_in [11:0] = volatile_random
```
The first eight bits of `my_super_cpu.r1` are labeled
as shares of the first eight secrets. The first eight bits of `my_super_cpu.r2`
are similarly labeled as second share of first eight secrets. 

### Example 2

The 1st-order DOM AND can be labeled for 1-bit wide shares, without RNG:

```
X0_i [0] = secret 0
X0_i [7:1] = unimportant
X1_i [0] = secret 0
X1_i [7:1] = unimportant
Y0_i [0] = secret 1
Y0_i [7:1] = unimportant
Y1_i [0] = secret 1
Y1_i [7:1] = unimportant
Z_i [0] = static_random
Z_i [7:1] = unimportant
```

...or e.g. for 8-bit wide shares with RNG:

```
X0_i [7:0] = secret 7:0
X1_i [7:0] = secret 7:0
Y0_i [7:0] = secret 15:8
Y1_i [7:0] = secret 15:8
Z_i [7:0] = volatile_random
```

### 3. Generate an execution **trace**
```
python3 trace.py 
  --testbench TB_FILE_PATH 
  --netlist NETLIST_FILE_PATH 
  [optional arguments]
```
The arguments for the standard mode of operation are:
  * `--testbench`: Path to the Verilator testbench.
  * `--netlist`: Path of Verilog netlist generated by Yosys in the parsing step.

Special arguments include:
  * `--skip-compile-netlist`: Use cached object files from a previous verilator run.
  * `--c-compiler`: Compiler used by Verilator (either clang or gcc), the default and recommended option is clang.
  * `--make-jobs`: CNumber of cores used for the make command.
  * `--output-bin`: Path to verilated binary. Default: `alma/tmp/<netlist_name>`


### Example

In order to create a VCD trace for the the 1st-order DOM AND, run:
```
python3 trace.py --testbench examples/gadgets/verilator_tb.cpp --netlist tmp/circuit.v
```

### 4. **Verify** the masking implementation
```
python3 verify.py 
  --json JSON_FILE_PATH 
  --label LABEL_PATH 
  --vcd TRACE_PATH 
  [optional arguments]
```
The arguments for the standard mode of operation are:
  * `--json`: File path of JSON file
  * `--label`: File path of label file
  * `--vcd`: File path of VCD file

Optinal arguments include:
  * `--cycles`: The verification process will run until the end of the VCD trace per default (-1). In case it should abort earlier, this option can be used.
  * `--from-cycles`: The verification process will start immediately after reset (0). In case it should start later, this option can be used.
  * `--order`: Verification order to be checked for the masked design (the number of probes). Default: 1
  * `--mode`: Verification can be done for the stable (no glitches) or transient case (glitches+transitions). Default: stable
  * `--glitch-behavior`: Determines behavior of glitches.
  * `--probing-model`: Specifies whether to use the classic (`classic`) or time-constrained (`time-constrained`) probing model. For more details about the differences between these models, see the associated [paper](https://eprint.iacr.org/2020/1294.pdf) Default: time-constrained
  * `--trace-stable`: If specified, trace signals are assumed to be stable
  * `--minimize-leaks`: Tells the solver to find the smallest correlating linear combination
  * `--checking-mode`: Specifies checking mode. `per-secret` means one formula is built per secret and the solver identifies leaking probing locations. `per-location` means one formula is built per potentially leaking probing locations and the solver identifies combinations of secrets causing leaks (default: %(default)s). Usually, `per-location` is expected to perform better for first-order designs, while `per-secret` is faster for higher-order designs.
  * `--num-leaks`: Number of leakage locations to be reported if the circuit is insecure.
  * `--rst-name`: Name of the reset signal. Verification will start after the circuit reset is over. Default: `rst_i`
  * `--rst-cycles`: Duration of the system reset in cycles. Default: 2
  * `--rst-phase`: Value of the reset signal which triggers the reset. Default: 1
  * `--num-leaks`: Number of leakage locations to be reported if the circuit is insecure. Default: 1
  * `--dbg-output-dir`: Directory in which debug leakage traces (dbg-label-trace-?.txt, dbg-circuit-?.dot) are written. Default: `alma/tmp/`
  * `--dbg-signals`: List of debug signals whose values (from VCD) should be printed
  * `--dbg-exact-formula`: For each node, print exact formula computed by the tool.
  * `--export-cnf`: Export CNF which needs to be solved for each secret to dbg_output_dir. This allows to use other solvers than CaDiCaL, e.g. Kissat.
  * `--kissat`: Path to a the Kissat binary file. Note that for enabling solving with Kissat, you need to set the `--export-cnf` option.


## Resources

The `examples` directory contains instructions on how to perform software masking verification on an improved version of the [IBEX RISC-V core](https://github.com/IAIK/coco-ibex) presented in the paper, and some hardware case-studies.



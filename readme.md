# CocoAlma: Execution-aware Masking Verification

CocoAlma is an _execution-aware_ tool for formal verification of masked implementations.
It can verify any data-independent masked computation that can be implemented as a Verilog 
hardware circuit or as software running on a hardware platform, with properly labeled secret 
shares, masks and RNG ports. 

For software verification on hardware level, please look at the examples provided for the
verification of masked software on CocoIbex and the associated paper [COCO](https://eprint.iacr.org/2020/1294.pdf).
For pure hardware verification of masked designs you can look at the provided examples for
Prince-TI and AES-DOM, which have very distinct implementations. Here, AES-DOM is implemented
as a pipeline of cascading state registers, whereas Prince-TI is implemented in a straightforward
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

## Setup

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

4. Install Yosys >= 0.9 and Verilator >= 4.106:
* **Easy**: install it using your favourite package manager
``` bash
apt install yosys verilator
```
* **Flexible**: Install then according to the official guides 
[here](https://github.com/YosysHQ/yosys/blob/yosys-0.9/README.md) and [here](https://www.veripool.org/projects/verilator/wiki/Installing).

After following these steps, the only additional thing you need is the hardware 
circuit you want to verify in Verilog or System Verilog. For System Verilog support,
either try recompilation into vanilla Verilog with [sv2v](https://github.com/zachjs/sv2v) 
or get a commercial license for [Symbiotic EDA](https://www.symbioticeda.com/seda-suite).

## Usage

Alma provides several Python programs that represent different stages of the verification flow.
In the following, we briefly show how each of them is used and what its purpose is. 

### 1. Parse the circuit
```
python3 parse.py --source VERILOG_FILES --top-module TOP_MODULE [optional arguments]
```
The arguments for the standard mode of operation are:
  * `--source`: file path(s) to the source file(s)
  * `--top-module`: the top module name
  
Special arguments include:
  * `--synthesis-file`: `parse.py` will automatically generate a Yosys synthesis script. In case one does not want to use this script, this option can be used to specify a custom Yosys synthesis script. Then, the `--source` option must not be used because the script already includes the paths of the sources.
  * `--label`: Custom output file path of label file. Default: `alma/tmp/labels.txt`
  * `--json`: Custom output file path of JSON file. Default: `alma/tmp/circuit.json`
  * `--netlist`: Custom output file path of netlist file. Default: `alma/tmp/circuit.v`
  * `--yosys`: `parse.py` will search for the Yosys binary using `which yosys`. In case one wants to use a specific Yosys version, the path can be specified with this option.
  * `--log-yosys`: Yosys synthesis output is required and will be written to `alma/tmp/yosys_synth_log.txt` 
  
The outputs of this step are a label file, the circuit in JSON format and the netlist file.

### 2. Label the registers and memory in the label file

`labels.txt` lists all registers and memory locations, which can be labeled as `secret`, `mask` or `random`.

For example, a valid labeling can look like this:

```
# registers
my_super_cpu.r1 [7:0] = secret 7:0
my_super_cpu.r2 [7:0] = secret 7:0
my_super_cpu.r3 [3:0] = mask
my_super_cpu.r3 [7:4] = secret 11:8
my_super_cpu.r4 [4:0] = secret 11:8
# ports
rng_in [11:0] = random
```

The format of each line is `(display_name) ([range]?) = (purpose) (range?)`
The register name `display_name` is a mangled name from the original design, 
`[range]` indicates the range of bits in the signal that you are labeling, 
`purpose` is either `secret`, `mask` or `random`. When labeling something as
shares of a secret, you can specify a range of secrets instead, and the individual
bits of the signal will be associated with the given secret in the range.
In the example above, the first eight bits of `my_super_cpu.r1` are labeled
as shares of the first eight secrets. The first eight bits of `my_super_cpu.r2`
are similarly labeled as second share of first eight secrets. 

For your convenience, the exact names you are supposed to use are given in the
form of a template `labels.txt` that is generated through `parse.py`.

### 3. Generate an execution trace
```
python3 trace.py --testbench TB_FILE_PATH --netlist NETLIST_FILE_PATH [optional arguments]
```
The arguments for the standard mode of operation are:
  * `--testbench`: Path to the Verilator testbench.
  * `--netlist`: Path of Verilog netlist generated by Yosys in the parsing step.

Special arguments include:
  * `--skip-compile-netlist`: Use cached object files from a previous verilator run.
  * `--c-compiler`: Compiler used by Verilator (either clang or gcc), the default and recommended option is clang.
  * `--output-bin`: Path to verilated binary. Default: `alma/tmp/<netlist_name>`

### 4. Verify the masking implementation
```
python3 verify.py --json JSON_FILE_PATH --label LABEL_PATH --vcd TRACE_PATH [optional arguments]
```
The arguments for the standard mode of operation are:
  * `--json`: File path of JSON file
  * `--label`: File path of label file
  * `--vcd`: File path of VCD file

Special arguments include:
  * `--cycles`: The verification process will run until the end of the VCD trace per default (-1). In case it should abort earlier, this option can be used.
  * `--order`: Verification order, i.e., the number of probes. Default: 1
  * `--mode`: Verification can be done for the stable or transient case. Default: stable
  * `--probe-duration`: Specifies how a long probe records values. Default: once
  * `--trace-stable`: If specified, trace signals are assumed to be stable
  * `--rst-name`: Verification will start after the circuit reset is over. This is the name of the reset signal. Default: `rst_i`
  * `--rst-cycles`: Duration of the system reset in cycles. Default: 2
  * `--rst-phase`: Value of the reset signal which triggers the reset. Default: 1
  * `--num-leaks`: Number of leakage locations to be reported if the circuit is insecure. Default: 1
  * `--dbg-output-dir`: Directory in which debug traces (dbg-label-trace-?.txt, dbg-circuit-?.dot) are written. Default: `alma/tmp/`

## Resources

The `examples` directory contains instructions on how to perform software masking verification
on an improved version of the [IBEX RISC-V core](https://github.com/IAIK/coco-ibex) presented in the paper.

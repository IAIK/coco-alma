import pytest
import subprocess
import sys

YOSYS_BIN = "_outputs/linux-x64/yosys/yosyshq/bin/yosys"

def test_keccak_dom():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "keccak_sbox", "--source", "examples/keccak_dom/design/keccak_sbox.v", "--netlist", "tmp/circuit.v", "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0

    args = ["python3","trace.py","--testbench","examples/keccak_dom/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0

    args = ["cp", "examples/keccak_dom/labels.txt", "tmp/"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "keccak_sbox", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert verify_process.returncode == 0


def test_dom_and_1storder():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "dom_and_1storder", "--source", "examples/gadgets/design/dom_and.v", "--netlist", "tmp/circuit.v" , "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0


    args = ["sed", "-i", "s/TC_NAME/dom_and_1storder/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["python3","trace.py","--testbench","examples/gadgets/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0

        
    args = ["sed", "-i", "s/#define TC dom_and_1storder/#define TC TC_NAME/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["cp", "examples/gadgets/labels_dom_and_1storder.txt", "tmp/labels.txt"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "dom_and_1storder", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)




    assert verify_process.returncode == 0



def test_dom_and_1storder_broken():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "dom_and_1storder_broken", "--source", "examples/gadgets/design/dom_and.v", "--netlist", "tmp/circuit.v", "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0


    args = ["sed", "-i", "s/TC_NAME/dom_and_1storder/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["python3","trace.py","--testbench","examples/gadgets/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0

    
    args = ["sed", "-i", "s/#define TC dom_and_1storder/#define TC TC_NAME/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["cp", "examples/gadgets/labels_dom_and_1storder_broken.txt", "tmp/labels.txt"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "stable", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "dom_and_1storder_broken", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert verify_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "dom_and_1storder_broken", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)


    assert verify_process.returncode != 0


def test_dom_and_2ndorder():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "dom_and_2ndorder", "--source", "examples/gadgets/design/dom_and.v", "--netlist", "tmp/circuit.v", "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0


    args = ["sed", "-i", "s/TC_NAME/dom_and_2ndorder/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["python3","trace.py","--testbench","examples/gadgets/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0


    args = ["sed", "-i", "s/#define TC dom_and_2ndorder/#define TC TC_NAME/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["cp", "examples/gadgets/labels_dom_and_2ndorder.txt", "tmp/labels.txt"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "dom_and_2ndorder", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert verify_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "dom_and_2ndorder", "--glitch-behavior", "strict", "--order", "2"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)


    assert verify_process.returncode == 0

#MAX 4 minutes
@pytest.mark.timeout(10)
def test_dom_and_3rdorder():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "dom_and_3rdorder", "--source", "examples/gadgets/design/dom_and.v", "--netlist", "tmp/circuit.v", "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0


    args = ["sed", "-i", "s/TC_NAME/dom_and_3rdorder/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["python3","trace.py","--testbench","examples/gadgets/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0

    args = ["sed", "-i", "s/#define TC dom_and_3rdorder/#define TC TC_NAME/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["cp", "examples/gadgets/labels_dom_and_3rdorder.txt", "tmp/labels.txt"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0


    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "dom_and_3rdorder", "--glitch-behavior", "strict", "--order", "3"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)

    assert verify_process.returncode == 0




def test_isw_and_1storder():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "isw_and_1storder", "--source", "examples/gadgets/design/isw_and.v", "--netlist", "tmp/circuit.v", "--log-yosys", "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0


    args = ["sed", "-i", "s/TC_NAME/isw_and_1storder/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["python3","trace.py","--testbench","examples/gadgets/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0
        
    args = ["sed", "-i", "s/#define TC isw_and_1storder/#define TC TC_NAME/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["cp", "examples/gadgets/labels_isw_and_1storder.txt", "tmp/labels.txt"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "stable", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "isw_and_1storder", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)

    assert verify_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "isw_and_1storder", "--glitch-behavior", "strict"] #, "--dbg-exact-formula"]
    print(" ".join(args))
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert verify_process.returncode == 0
    

def test_isw_and_1storder_broken():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "isw_and_1storder_broken", "--source", "examples/gadgets/design/isw_and.v", "--netlist", "tmp/circuit.v", "--log-yosys", "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0


    args = ["sed", "-i", "s/TC_NAME/isw_and_1storder/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["python3","trace.py","--testbench","examples/gadgets/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0
        
    args = ["sed", "-i", "s/#define TC isw_and_1storder/#define TC TC_NAME/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["cp", "examples/gadgets/labels_isw_and_1storder_broken.txt", "tmp/labels.txt"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "stable", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "isw_and_1storder_broken", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)

    assert verify_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "isw_and_1storder_broken", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)

    assert verify_process.returncode != 0


def test_ti_toffoli():
    args = ["mkdir", "tmp/"]
    subprocess.run(args)
    
    args = ["python3", "parse.py", "--top-module", "ti_toffoli", "--source", "examples/gadgets/design/ti_toffoli.v", "--netlist", "tmp/circuit.v", "--log-yosys", "--yosys", YOSYS_BIN]
    parse_process = subprocess.run(args, input="Y".encode(),stdout=sys.stdout, stderr=sys.stderr)
    assert parse_process.returncode == 0


    args = ["sed", "-i", "s/TC_NAME/ti_toffoli/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["python3","trace.py","--testbench","examples/gadgets/verilator_tb.cpp","--netlist","tmp/circuit.v"]
    trace_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert trace_process.returncode == 0
        
    args = ["sed", "-i", "s/#define TC ti_toffoli/#define TC TC_NAME/g", "examples/gadgets/verilator_tb.cpp"]
    subprocess.run(args)

    args = ["cp", "examples/gadgets/labels_ti_toffoli.txt", "tmp/labels.txt"]
    label_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)
    assert label_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "stable", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "ti_toffoli", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)

    assert verify_process.returncode == 0

    args = ["python3", "verify.py", "--json","tmp/circuit.json","--label", "tmp/labels.txt", "--vcd", "tmp/tmp.vcd", "--cycles", "5", "--mode", "transient", "--rst-name", "rst_i", "--probe-duration", "once", "--top-module", "ti_toffoli", "--glitch-behavior", "strict"]
    verify_process = subprocess.run(args,stdout=sys.stdout, stderr=sys.stderr)

    assert verify_process.returncode == 0
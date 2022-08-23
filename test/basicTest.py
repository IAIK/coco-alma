import pytest
import subprocess
import sys
import time
from typing import List, Dict
from verificatonContext import *


YOSYS_BIN = "_outputs/linux-x64/yosys/yosyshq/bin/yosys"
#YOSYS_BIN = "/usr/local/bin/yosys"

contextMap= {}

def teardown_module(module):
    import matplotlib.pyplot as plt
    import matplotlib as mpl

    mpl.rcParams['figure.figsize'] = (8,8*len(contextMap))

    fig, ax = plt.subplots(len(contextMap))
    

    for (row,(desc,contextList)) in enumerate(contextMap.items()):

        for cnt, vc in enumerate(contextList):
            ax[row].bar(cnt, vc.runtime, color = "red" if vc.mode==STABLE else "green", hatch = "//" if vc.checking_mode == PER_LOCATION else "", edgecolor="white", label=vc.shortStr())


        #ax[row].set_xticks(range(0, len(contextList)), [vc.shortStr() for vc in contextList])
        ax[row].set_ylabel("Runtime (s)")
        ax[row].set_title(desc)
        ax[row].legend(loc="upper right", bbox_to_anchor=(0.5, -0.05))
    plt.subplots_adjust(hspace=0.5)
    plt.savefig("runtime.png", bbox_inches='tight')


@pytest.mark.timeout(30)
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
    
    vc: VerificationContext = VerificationContext("keccak_sbox", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET)
    contextMap["keccak_sbox"] = [vc]

    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    assert verify_process.returncode == 0
    vc.runtime = time.time() - t

    vc: VerificationContext = VerificationContext("keccak_sbox", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["keccak_sbox"].append(vc)

    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    assert verify_process.returncode == 0
    vc.runtime = time.time() - t





@pytest.mark.timeout(30)
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

    vc: VerificationContext = VerificationContext("dom_and_1storder", 5, STABLE, TIME_CONSTRAINED, PER_SECRET)
    contextMap["dom_and_1storder"] = [vc]
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0


    vc: VerificationContext = VerificationContext("dom_and_1storder", 5, STABLE, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["dom_and_1storder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_1storder", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET)
    contextMap["dom_and_1storder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_1storder", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["dom_and_1storder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0


@pytest.mark.timeout(30)
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

    
    vc: VerificationContext = VerificationContext("dom_and_1storder_broken", 5, STABLE, TIME_CONSTRAINED, PER_SECRET)
    contextMap["dom_and_1storder_broken"] = [vc]
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_1storder_broken", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET)
    contextMap["dom_and_1storder_broken"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode != 0

    vc: VerificationContext = VerificationContext("dom_and_1storder_broken", 5, STABLE, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["dom_and_1storder_broken"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0


    vc: VerificationContext = VerificationContext("dom_and_1storder_broken", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["dom_and_1storder_broken"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode != 0

@pytest.mark.timeout(30)
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
        
    vc: VerificationContext = VerificationContext("dom_and_2ndorder", 5, STABLE, TIME_CONSTRAINED, PER_SECRET, order=2)
    contextMap["dom_and_2ndorder"] = [vc]
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_2ndorder", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET, order=2)
    contextMap["dom_and_2ndorder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_2ndorder", 5, STABLE, TIME_CONSTRAINED, PER_LOCATION, order=2)
    contextMap["dom_and_2ndorder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    assert verify_process.returncode == 0
    vc: VerificationContext = VerificationContext("dom_and_2ndorder", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION, order=2)
    contextMap["dom_and_2ndorder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    assert verify_process.returncode == 0

    

@pytest.mark.timeout(30)
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


    vc: VerificationContext = VerificationContext("dom_and_3rdorder", 5, STABLE, TIME_CONSTRAINED, PER_SECRET, order=3)
    contextMap["dom_and_3rdorder"] = [vc]
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_3rdorder", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET, order=3)
    contextMap["dom_and_3rdorder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_3rdorder", 5, STABLE, TIME_CONSTRAINED, PER_LOCATION, order=3)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    contextMap["dom_and_3rdorder"].append(vc)
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("dom_and_3rdorder", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION, order=3)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time()-t
    contextMap["dom_and_3rdorder"].append(vc)
    assert verify_process.returncode == 0


@pytest.mark.timeout(30)
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

    vc: VerificationContext = VerificationContext("isw_and_1storder", 5, STABLE, TIME_CONSTRAINED, PER_SECRET)
    contextMap["isw_and_1storder"] = [vc]
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0


    vc: VerificationContext = VerificationContext("isw_and_1storder", 5, STABLE, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["isw_and_1storder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("isw_and_1storder", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET)
    contextMap["isw_and_1storder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("isw_and_1storder", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["isw_and_1storder"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    
@pytest.mark.timeout(30)
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

    vc: VerificationContext = VerificationContext("isw_and_1storder_broken", 5, STABLE, TIME_CONSTRAINED, PER_SECRET)
    contextMap["isw_and_1storder_broken"] = [vc]
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0


    vc: VerificationContext = VerificationContext("isw_and_1storder_broken", 5, STABLE, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["isw_and_1storder_broken"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("isw_and_1storder_broken", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET)
    contextMap["isw_and_1storder_broken"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode != 0

    vc: VerificationContext = VerificationContext("isw_and_1storder_broken", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["isw_and_1storder_broken"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode != 0

@pytest.mark.timeout(30)
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

    vc: VerificationContext = VerificationContext("ti_toffoli", 5, STABLE, TIME_CONSTRAINED, PER_SECRET)
    contextMap["ti_toffoli"] = [vc]
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0


    vc: VerificationContext = VerificationContext("ti_toffoli", 5, STABLE, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["ti_toffoli"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("ti_toffoli", 5, TRANSIENT, TIME_CONSTRAINED, PER_SECRET)
    contextMap["ti_toffoli"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0

    vc: VerificationContext = VerificationContext("ti_toffoli", 5, TRANSIENT, TIME_CONSTRAINED, PER_LOCATION)
    contextMap["ti_toffoli"].append(vc)
    t = time.time()
    verify_process = subprocess.run(vc.toCmdArgs(),stdout=sys.stdout, stderr=sys.stderr)
    vc.runtime = time.time() - t
    assert verify_process.returncode == 0
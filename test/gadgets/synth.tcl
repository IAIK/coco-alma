# read design 
yosys read_verilog -sv -defer $::env(VLOG_IN_FILE);
yosys proc;

if { [info exists ::env(NUM_SHARES)] } {
yosys chparam -set NUM_SHARES $::env(NUM_SHARES) $::env(VLOG_TOP)
}
# generic synthesis
yosys synth -top $::env(VLOG_TOP);

# mapping cells
yosys flatten
yosys clean
yosys stat

set tmpname [exec mktemp];

# write synthesized design
if { [info exists ::env(VLOG_OUT_FILE)] } {
set vlog_out_file $::env(VLOG_OUT_FILE);
} else {
set vlog_out_file $tmpname.v;
}

if { [info exists ::env(JSON_OUT_FILE)] } {
set json_out_file $::env(JSON_OUT_FILE);
} else {
set json_out_file $tmpname.json;
}

puts stdout "Writing synthesized verilog to $vlog_out_file";
yosys write_verilog $vlog_out_file
puts stdout "Writing synthesized json to $json_out_file";
yosys write_json $json_out_file

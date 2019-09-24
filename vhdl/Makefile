VHDL_SRC:= $(wildcard *.vhd)
VHDL_SRC:= $(patsubst %, ../%,$(VHDL_SRC))
TOP_LEVEL_DESIGN=FU_instruction_memory_tb
BUILD_DIR=build
SIM_TIME="30"

all: analyze_vhdl create_snapshot

sim:
	echo "add_wave {{/${TOP_LEVEL_DESIGN}/UUT}}" >> ${BUILD_DIR}/run.tcl
	echo "run ${SIM_TIME} us" >> ${BUILD_DIR}/run.tcl
	icebuddha "cd ${BUILD_DIR};xsim work.${TOP_LEVEL_DESIGN} -gui -tclbatch run.tcl"
	rm ${BUILD_DIR}/run.tcl

create_snapshot:
	icebuddha "cd ${BUILD_DIR};xelab ${TOP_LEVEL_DESIGN} -debug typical"

analyze_vhdl:
	icebuddha 'cd ${BUILD_DIR};xvhdl ${VHDL_SRC}'

clean:
	cd ${BUILD_DIR};rm -rf *

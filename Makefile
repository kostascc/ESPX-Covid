#####################################
# Real Time Embedded Systems
# Project 1
# 
# Ⓒ 2021 Kostas Chatzis
# kachatzis <at> ece.auth.gr
#####################################

GCC=gcc
XGCC=arm-linux-gnueabihf-gcc

GCC_FLAGS=-O3
XGCC_FLAGS=-O3

INC_DIR=./include
SRC_DIR=./src

LIB=-lpthread -lm


################################
#   x86/GCC                    #
################################
L_OUTPUT=./x86.o
x86:
	$(GCC) $(GCC_FLAGS) -o $(L_OUTPUT) $(shell find $(SRC_DIR) -name '*.c') -I $(INC_DIR) $(LIB)
run_x86:
	$(L_OUTPUT)


################################
#   RPI/GCC                    #
################################
RPI_OUTPUT=./rpi.o
run_rpi: 
	$(RPI_OUTPUT)

################################
#   Mode A:                    #
#   Raspberry Pi Zero W/WH     #
#   Raspberry Pi 1 A/B/A+/B+   #
################################
RPI_A_FLAGS=-march=armv6 -mfloat-abi=hard -mfpu=vfp


rpi_a:
	$(XGCC) $(XGCC_FLAGS) -o $(RPI_OUTPUT) $(shell find $(SRC_DIR) -name '*.c') -I $(INC_DIR) $(RPI_A_FLAGS) $(LIB)

rpi_zero_w: rpi_a
rpi_zero_wh: rpi_a
rpi_1_a: rpi_a
rpi_1_b: rpi_a
rpi_1_ap: rpi_a
rpi_1_bp: rpi_a


###########################
#   Mode B:               #
#   Raspberry 2 A/B       #
#   Raspberry 3 A/B       #
###########################
RPI_B_FLAGS=-march=armv7-a -mfloat-abi=hard -mfpu=neon-vfpv4

rpi_b:
	$(XGCC) $(XGCC_FLAGS) -o $(RPI_OUTPUT) $(shell find $(SRC_DIR) -name '*.c') -I $(INC_DIR) $(RPI_B_FLAGS) $(LIB)

rpi_3_b: rpi_b


# TODO: add executions

###########################
#   Mode C:               #
#   Raspberry 3 A+/B+     #
#   Raspberry 4 A+/B+     #
###########################
RPI_C_FLAGS=-march=armv8-a -mfloat-abi=hard -mfpu=neon-fp-armv8

rpi_c:
	$(XGCC) $(XGCC_FLAGS) -o $(RPI_OUTPUT) $(shell find $(SRC_DIR) -name '*.c') -I $(INC_DIR) $(RPI_C_FLAGS) $(LIB)

rpi_3_bp: rpi_c

run_rpi_3_bp: run_rpi_c

# TODO: add executions



###########################
#   Utilities             #
###########################
help:
	@echo "For Raspberry Pi"
	@echo "  Build: make rpi_(version)"
	@echo "  Run:   make run_rpi_(version)"
	@echo ""
	@echo "  Versions: zero_w, zero_wh, 1_a, 1_ap, 1_b, 1_bp"
	@echo ""
	@echo "For x86/64"
	@echo "  Build: make x86"
	@echo "  Run:   make run_x86"

clean:
	rm -rf ./*.o ./*.out
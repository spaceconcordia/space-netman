# Warning flags (what did you expect)
WARNINGS= -Wall -Wextra -fpermissive
# Optimization level for gcc
OPT_LVL = 1
# C Compiler
CC      = g++
CCFLAGS = -c -O$(OPT_LVL) $(WARNINGS)
MICROCC=microblazeel-xilinx-linux-gnu-g++
BEAGLECC=arm-linux-gnueabi-g++
MICROCFLAGS=-mcpu=v8.10.a -mxl-barrel-shift -mxl-multiply-high -mxl-pattern-compare -mno-xl-soft-mul -mno-xl-soft-div -mxl-float-sqrt -mhard-float -mxl-float-convert -ffixed-r31 --sysroot /usr/local/lib/mbgcc/microblaze-unknown-linux-gnu/sys-root -Wall
# Linker
LD      = g++
MICROLD = $(MICROCC)
LDFLAGS = -O$(OPT_LVL) $(WARNINGS) -lrt

PROJ    = netman
# Source files that we will use
SRCS    := netman.c of2g.c
SRCS    := $(addprefix src/, $(SRCS))

LIBS     := Net2Com.a NamedPipe.a libfletcher.a libhe100.a libtimer.a libshakespeare.a
LIBS     := $(addprefix lib/, $(LIBS))
INCFLAGS = -I./lib/include

MICROLIBS     := Net2Com-mbcc.a NamedPipe-mbcc.a libtimer-mbcc.a libfletcher-mbcc.a libhe100-mbcc.a libshakespeare-mbcc.a
MICROLIBS     := $(addprefix lib/, $(MICROLIBS))
BEAGLELIBS    := Net2Com-BB.a NamedPipe-BB.a libtimer-BB.a libflecher-BB.a libhe100-BB.a libshakespeare-BB.a
BEAGLELIBS    := $(addprefix lib/, $(BEAGLELIBS))

BIN_DIR = bin
SAT_BIN_FILE= $(BIN_DIR)/sat
GND_BIN_FILE= $(BIN_DIR)/gnd
VALVE_BIN_FILE = $(BIN_DIR)/valve
SAT_BIN_FILEQ6= $(BIN_DIR)/sat-mbcc
GND_BIN_FILEQ6= $(BIN_DIR)/gnd-mbcc
SAT_BIN_FILEBB= $(BIN_DIR)/sat-BB
GND_BIN_FILEBB= $(BIN_DIR)/gnd-BB
ALL_TRG = $(SAT_BIN_FILE) $(GND_BIN_FILE)
ALL_TRGQ6 = $(SAT_BIN_FILEQ6) $(GND_BIN_FILEQ6)
ALL_TRGBB = $(SAT_BIN_FILEBB) $(GND_BIN_FILEBB)
# Generate exact dependencies using a smart method that I found online.
#
# The basic idea is that we don't need to know the dependencies until
# after the first time we build a file. If the file itself changes, then
# obviously we will have to recalculate its dependencies, but %.c is always
# a dependency of %.o, so in that case it would be recompiled anyways
#
# TODO - find the link I got this method from
DEP_DIR     = .deps
MAKE_DEPEND = mkdir -p $(DEP_DIR)/$(dir $*); $(CPP) -MM $(CCFLAGS) $< -o $(DEP_DIR)/$*.d
MAKE_DEPENDQ6 = mkdir -p $(DEP_DIR)/$(dir $*); $(CPP) -MM $(CCFLAGS) $< -o $(DEP_DIR)/$*.d

# Phony targets
.PHONY: all clean namepipe

# Make our dep_dir and our hex file
all: $(ALL_TRG)
Q6: $(ALL_TRGQ6)
BB: $(ALL_TRGBB)
# get rid of all the shit we created
clean:
	rm -rf $(DEP_DIR)
	rm -rf $(BIN_DIR)

# Here's how we make our dep dir. (Not alll that complicated)
$(DEP_DIR):
	mkdir -p $(DEP_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Here is where all the .dep file are #include'd
-include $(SRCS:%.c=$(DEP_DIR)/%.d)

# TODO - integrate the libs properly!!
# For each c file, we compile it to an o file, and then make a
# dependency file for it, as explained above
%.o: %.cpp $(DEP_DIR)
	$(CC) $(INCFLAGS) $(CCFLAGS) $< -o $@
	@$(MAKE_DEPEND)

%.o: %.c $(DEP_DIR)
	$(CC) $(INCFLAGS) $(CCFLAGS) $< -o $@
	@$(MAKE_DEPEND)

src/sat_transceiver.o: src/transceiver.c $(DEP_DIR)
	$(CC) $(INCFLAGS) -D'TRNSCVR_TX_PIPE="sat-out-gnd-in"' \
	                  -D'TRNSCVR_RX_PIPE="gnd-out-sat-in"' \
							-D'USE_PIPE_TRNSCVR' \
							-D'VALVE_TX_PIPE="sat_valve"' \
							$(CCFLAGS) $< -o $@

src/gnd_transceiver.o: src/transceiver.c $(DEP_DIR)
	$(CC) $(INCFLAGS) -D'TRNSCVR_TX_PIPE="gnd-out-sat-in"' \
	                  -D'TRNSCVR_RX_PIPE="sat-out-gnd-in"' \
							-D'USE_PIPE_TRNSCVR' \
							-D'VALVE_TX_PIPE="gnd_valve"' \
							$(CCFLAGS) $< -o $@

# For each c file, we compile it to an o file, and then make a
# dependency file for it, as explained above
#%.o: %.c $(DEP_DIR)
#	$(CC) $(INCFLAGS) $(CCFLAGS) $< -o $@
#	@$(MAKE_DEPEND)

# Our binary requires all our o files, and is fairly simple to make
$(GND_BIN_FILE): $(SRCS:%.c=%.o) src/gnd_transceiver.o src/gnd_main.o $(LIBS) $(BIN_DIR)
	$(LD) $(filter %.o, $^) $(filter %.a, $^) $(LDFLAGS) -o $@

# Our binary requires all our o files, and is fairly simple to make
$(SAT_BIN_FILE): $(SRCS:%.c=%.o) src/sat_transceiver.o src/sat_main.o $(LIBS) $(BIN_DIR)
	$(LD) $(filter %.o, $^) $(filter %.a, $^) $(LDFLAGS) -o $@


# Q6 shit down here

src/of2gQ6.o : src/of2g.c
	$(MICROCC) $(INCFLAGS) $(MICROCCFLAGS) $< -c -o src/of2gQ6.o

src/netmanQ6.o : src/netman.c
	$(MICROCC) $(INCFLAGS) $(MICROCCFLAGS) $< -c -o src/netmanQ6.o

src/gnd_transceiverQ6.o : src/transceiver.c
	$(MICROCC) $(INCFLAGS) -D'TRNSCVR_TX_PIPE="gnd-out-sat-in"' \
	                  -D'TRNSCVR_RX_PIPE="sat-out-gnd-in"' \
							-D'USE_PIPE_TRNSCVR' \
							-D'VALVE_TX_PIPE="gnd_valve"' \
$(MICROCCFLAGS) $< -c -o src/gnd_transceiverQ6.o

src/gnd_mainQ6.o : src/gnd_main.c src/gnd_transceiverQ6.o
	$(MICROCC) $(INCFLAGS) $(MICROCCFLAGS) $< -c -o src/gnd_mainQ6.o

src/sat_mainQ6.o : src/sat_main.c src/sat_transceiverQ6.o
	$(MICROCC) $(INCFLAGS) $(MICROCCFLAGS) $< -c -o src/sat_mainQ6.o

src/sat_transceiverQ6.o: src/transceiver.c
	$(MICROCC) $(INCFLAGS) $(MICROCCFLAGS) $< -c -o src/sat_transceiverQ6.o

$(GND_BIN_FILEQ6): src/of2gQ6.o src/netmanQ6.o src/gnd_transceiverQ6.o src/gnd_mainQ6.o $(MICROLIBS) $(BIN_DIR)
	$(MICROLD) $(filter %.o, $^) $(filter %.a, $^) $(LDFLAGS) -o $@

$(SAT_BIN_FILEQ6): src/of2gQ6.o src/netmanQ6.o src/sat_transceiverQ6.o src/sat_mainQ6.o $(MICROLIBS) $(BIN_DIR)
	$(MICROLD) $(filter %.o, $^) $(filter %.a, $^) $(LDFLAGS) -o $@

# BeagleBone is THE shit

src/of2gBB.o : src/of2g.c
	$(BEAGLECC) $(INCFLAGS) $< -c -o src/of2gBB.o

src/netmanBB.o : src/netman.c
	$(BEAGLECC) $(INCFLAGS) $< -c -o src/netmanBB.o

src/gnd_transceiverBB.o : src/transceiver.c
	$(BEAGLECC) $(INCFLAGS) -D'TRNSCVR_TX_PIPE="gnd-out-sat-in"' \
	                  -D'TRNSCVR_RX_PIPE="sat-out-gnd-in"' \
							-D'USE_PIPE_TRNSCVR' \
							-D'VALVE_TX_PIPE="gnd_valve"' \
$(MICROCCFLAGS) $< -c -o src/gnd_transceiverBB.o

src/gnd_mainBB.o : src/gnd_main.c src/gnd_transceiverBB.o
	$(BEAGLECC) $(INCFLAGS) $< -c -o src/gnd_mainBB.o

src/sat_mainBB.o : src/sat_main.c src/sat_transceiverBB.o
	$(BEAGLECC) $(INCFLAGS) $< -c -o src/sat_mainBB.o

src/sat_transceiverBB.o: src/transceiver.c
	$(BEAGLECC) $(INCFLAGS) $< -c -o src/sat_transceiverBB.o

$(GND_BIN_FILEBB): src/of2gBB.o src/netmanBB.o src/gnd_transceiverBB.o src/gnd_mainBB.o $(BEAGLELIBS) $(BIN_DIR)
	$(BEAGLECC) $(filter %.o, $^) $(filter %.a, $^) $(LDFLAGS) -o $@

$(SAT_BIN_FILEBB): src/of2gBB.o src/netmanBB.o src/sat_transceiverBB.o src/sat_mainBB.o $(BEAGLELIBS) $(BIN_DIR)
	$(BEAGLECC) $(filter %.o, $^) $(filter %.a, $^) $(LDFLAGS) -o $@


# Warning flags (what did you expect)
WARNINGS= -Wall -Wextra -Werror
# Optimization level for gcc
OPT_LVL = 1
# C Compiler
CC      = g++
CCFLAGS = -c -O$(OPT_LVL) $(WARNINGS)

# Linker
LD      = g++
LDFLAGS = -O$(OPT_LVL) $(WARNINGS) -lrt

PROJ    = netman
# Source files that we will use
SRCS    := main.c netman.c of2g.c transceiver.c
SRCS    := $(addprefix src/, $(SRCS))

LIBS     := Net2Com.a NamedPipe.a libtimer.a libhe100.a 
LIBS     := $(addprefix lib/, $(LIBS))
INCFLAGS = -I./lib/include

BIN_DIR = bin
BIN_FILE= $(BIN_DIR)/$(PROJ)
ALL_TRG = $(BIN_FILE)

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

# Phony targets
.PHONY: all clean

# Make our dep_dir and our hex file
all: $(ALL_TRG)

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

# For each c file, we compile it to an o file, and then make a
# dependency file for it, as explained above
#%.o: %.c $(DEP_DIR)
#	$(CC) $(INCFLAGS) $(CCFLAGS) $< -o $@
#	@$(MAKE_DEPEND) 

# Our binary requires all our o files, and is fairly simple to make
$(BIN_FILE): $(SRCS:%.c=%.o) $(LIBS) $(BIN_DIR)
	$(LD) $(filter %.o, $^) $(filter %.a, $^) $(LDFLAGS) -o $@


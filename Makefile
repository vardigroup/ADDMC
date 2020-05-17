OPTIMIZATION = -Ofast# on for MCC
STATIC = -static# on for MCC

################################################################################

HPP_DIR = src/interface
CPP_DIR = src/implementation

################################################################################

LIB_DIR = ./lib# unzipped from lib.zip by INSTALL.sh

CUDD_DIR = $(LIB_DIR)/cudd-3.0.0
CUDD_INCLUSIONS = -I$(CUDD_DIR) -I$(CUDD_DIR)/cudd -I$(CUDD_DIR)/epd -I$(CUDD_DIR)/mtr -I$(CUDD_DIR)/st
CUDD_LIBS = $(CUDD_DIR)/cudd/.libs
CUDD_LINK = -lcudd

################################################################################

O_DIR = obj

_OBJ = util.o graph.o formula.o join.o visual.o counter.o
_OBJ_ADDMC = $(_OBJ) main_addmc.o

OBJ_ADDMC = $(patsubst %, $(O_DIR)/%, $(_OBJ_ADDMC))

################################################################################

BIN_DIR = .

ADDMC = addmc

ADDMC_BIN = $(BIN_DIR)/$(ADDMC)

################################################################################

GCC = g++ -g# debugging information
GCC_FLAGS = $(OPTIMIZATION) $(STATIC) -I./$(HPP_DIR) -I$(LIB_DIR) $(CUDD_INCLUSIONS) -fconcepts -std=c++14

################################################################################

$(ADDMC): $(OBJ_ADDMC)
	mkdir -p $(BIN_DIR) && $(GCC) -o $(ADDMC_BIN) $^ $(GCC_FLAGS) -L $(CUDD_LIBS) $(CUDD_LINK) && echo

$(O_DIR)/%.o: $(CPP_DIR)/%.cpp $(HPP_DIR)/%.hpp
	mkdir -p $(O_DIR) && $(GCC) -c -o $@ $< $(GCC_FLAGS) && echo

################################################################################

.PHONY: clean

clean:
	rm -rf $(O_DIR) $(ADDMC_BIN) && echo

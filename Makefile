OPTIMIZATION = -Ofast# on for MCC
STATIC = -static# on for MCC

################################################################################

LIB_DIR = lib
LIB_ZIP = $(LIB_DIR).zip

CUDD_DIR = $(LIB_DIR)/cudd-3.0.0
CUDD_INCLUSIONS = -I$(CUDD_DIR) -I$(CUDD_DIR)/cudd -I$(CUDD_DIR)/epd -I$(CUDD_DIR)/mtr -I$(CUDD_DIR)/st
CUDD_LINK = -L$(CUDD_DIR)/cudd/.libs -lcudd

HPP_DIR = src/interface
CPP_DIR = src/implementation

OBJ_DIR = obj
_OBJ = util.o graph.o formula.o join.o visual.o counter.o
_OBJ_ADDMC = $(_OBJ) main_addmc.o
OBJ_ADDMC = $(patsubst %, $(OBJ_DIR)/%, $(_OBJ_ADDMC))

ADDMC_BIN = ./addmc

################################################################################

GCC = g++ -g# debugging information
GCC_FLAGS = $(OPTIMIZATION) $(STATIC) -I$(HPP_DIR) -I$(LIB_DIR) $(CUDD_INCLUSIONS) -fconcepts -std=c++14

################################################################################

$(ADDMC_BIN): $(OBJ_ADDMC)
	$(GCC) -o $(ADDMC_BIN) $^ $(GCC_FLAGS) $(CUDD_LINK) && echo

$(OBJ_DIR)/%.o: $(CPP_DIR)/%.cpp $(HPP_DIR)/%.hpp $(LIB_DIR)
	mkdir -p $(OBJ_DIR) && $(GCC) -c -o $@ $< $(GCC_FLAGS) && echo

$(LIB_DIR): $(LIB_ZIP)
	unzip -D $(LIB_ZIP) && cd $(CUDD_DIR) && ./configure --enable-silent-rules --enable-obj && make && echo

################################################################################

.PHONY: clean

clean:
	rm -rf $(OBJ_DIR) $(ADDMC_BIN) && echo

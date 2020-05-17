LIBRARIES_DIR = ../libraries

################################################################################

CUDD_DIR = $(LIBRARIES_DIR)/cudd-3.0.0
CUDD_INCLUSIONS = -I$(CUDD_DIR) -I$(CUDD_DIR)/cudd -I$(CUDD_DIR)/epd -I$(CUDD_DIR)/mtr -I$(CUDD_DIR)/st

CUDD_LIBS = $(CUDD_DIR)/cudd/.libs
CUDD_LINK = -lcudd

################################################################################

HPP_DIR = interface
CPP_DIR = implementation
O_DIR = object

_OBJ = util.o graph.o formula.o counter.o visual.o main.o
OBJ = $(patsubst %, $(O_DIR)/%, $(_OBJ))

EXECUTABLE = addmc

################################################################################

CXX = g++ -g # default debug information

CXXFLAGS = $(CUDD_INCLUSIONS) -I$(LIBRARIES_DIR) -I$(HPP_DIR) -fconcepts -std=c++14 -Ofast

################################################################################

$(EXECUTABLE): $(OBJ)
	$(CXX) -o $(EXECUTABLE) $^ $(CXXFLAGS) -L $(CUDD_LIBS) $(CUDD_LINK)

$(O_DIR)/%.o: $(CPP_DIR)/%.cpp $(HPP_DIR)/%.hpp
	mkdir -p $(O_DIR) && $(CXX) -c -o $@ $< $(CXXFLAGS) && echo

################################################################################

.PHONY: clean

clean:
	rm -rf $(O_DIR)/ $(EXECUTABLE)

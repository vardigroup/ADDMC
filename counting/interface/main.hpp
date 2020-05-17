/* includes *******************************************************************/

#include "../../libraries/cxxopts.hpp"

#include "counter.hpp"

/* classes ********************************************************************/

class OptionDict {
public:
  std::string filePath = DUMMY_STR;
  int_t weightFormatOption;
  int_t clusteringHeuristicOption;
  int_t formulaVarOrderingHeuristicOption;
  int_t addVarOrderingHeuristicOption;

  OptionDict(int argc, char *argv[]);
  bool hasValidFilePath();
};

/* namespaces *****************************************************************/

namespace testing {
  void test();
}

namespace solving {
  void solveFile(
    const std::string &filePath,
    WeightFormat weightFormat,
    ClusteringHeuristic clusteringHeuristic,
    VarOrderingHeuristic formulaVarOrderingHeuristic,
    bool inverseFormulaVarOrdering,
    VarOrderingHeuristic addVarOrderingHeuristic,
    bool inverseAddVarOrdering);
  void solveOptions(
    const std::string &filePath,
    int_t weightFormatOption = DEFAULT_WEIGHT_FORMAT_OPTION,
    int_t clusteringHeuristicOption = DEFAULT_CLUSTERING_HEURISTIC_OPTION,
    int_t formulaVarOrderingHeuristicOption = DEFAULT_FORMULA_VAR_ORDERING_HEURISTIC_OPTION,
    int_t addVarOrderingHeuristicOption = DEFAULT_ADD_VAR_ORDERING_HEURISTIC_OPTION);
  void solveCommand(int argc, char *argv[]);
}

/* globals ********************************************************************/

int main(int argc, char *argv[]);

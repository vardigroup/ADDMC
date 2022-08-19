/* inclusions *****************************************************************/

#include <limits>

#include "../../lib/cxxopts.hpp"

#include "counter.hpp"

/* classes ********************************************************************/

class OptionDict {
public:
  /* optional: */
  bool helpFlag;
  string cnfFilePath;
  Int weightFormatOption;
  string jtFilePath;
  Float jtWaitSeconds;
  Int outputFormatOption;
  Int clusteringHeuristicOption;
  Int cnfVarOrderingHeuristicOption;
  Int ddVarOrderingHeuristicOption;
  Int randomSeedOption;
  Int verbosityLevelOption;

  cxxopts::Options *options;

  void printOptionalOptions() const;
  void printHelp() const;
  void printWelcome() const;
  OptionDict(int argc, char *argv[]);
};

/* namespaces *****************************************************************/

namespace testing {
  void test();
}

namespace solving {
  void solveFile(
    const string &cnfFilePath,
    WeightFormat weightFormat,
    const string &jtFilePath,
    Float jtWaitSeconds,
    OutputFormat outputFormat,
    ClusteringHeuristic clusteringHeuristic,
    VarOrderingHeuristic cnfVarOrderingHeuristic,
    bool inverseCnfVarOrdering,
    VarOrderingHeuristic ddVarOrderingHeuristic,
    bool inverseDdVarOrdering
  );
  void solveOptions(
    const string &cnfFilePath,
    Int weightFormatOption,
    const string &jtFilePath,
    Float jtWaitSeconds,
    Int outputFormatOption,
    Int clusteringHeuristicOption,
    Int cnfVarOrderingHeuristicOption,
    Int ddVarOrderingHeuristicOption
  );
  void solveCommand(int argc, char *argv[]);
}

/* global functions ***********************************************************/

int main(int argc, char *argv[]);

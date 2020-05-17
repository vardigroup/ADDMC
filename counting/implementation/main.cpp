/* includes *******************************************************************/

#include "../interface/main.hpp"

/* classes ********************************************************************/

/* class OptionDict ***********************************************************/

OptionDict::OptionDict(int argc, char *argv[]) {
  cxxopts::Options options("addmc", "");

  options.add_options(REQUIRED_OPTION_GROUP)
    (CNF_FILE_OPTION, "CNF file path", cxxopts::value<std::string>())
  ;

  options.add_options(OPTIONAL_OPTION_GROUP)
    (WEIGHT_FORMAT_OPTION, "Weight format", cxxopts::value<std::string>()->default_value(std::to_string(DEFAULT_WEIGHT_FORMAT_OPTION)))
    (CLUSTERING_HEURISTIC_OPTION, "Clustering heuristic", cxxopts::value<std::string>()->default_value(std::to_string(DEFAULT_CLUSTERING_HEURISTIC_OPTION)))
    (CLUSTER_VAR_ORDER_OPTION, "Cluster variable order heuristic", cxxopts::value<std::string>()->default_value(std::to_string(DEFAULT_FORMULA_VAR_ORDERING_HEURISTIC_OPTION)))
    (DIAGRAM_VAR_ORDER_OPTION, "Diagram variable order heuristic", cxxopts::value<std::string>()->default_value(std::to_string(DEFAULT_ADD_VAR_ORDERING_HEURISTIC_OPTION)))
  ;

  cxxopts::ParseResult result = options.parse(argc, argv);

  switch (result.count(CNF_FILE_OPTION)) {
    case 0: {
      std::cout << options.help({
        REQUIRED_OPTION_GROUP,
        // OPTIONAL_OPTION_GROUP
      }) << "\n";
      util::printOptions();
      break;
    }
    case 1: {
      filePath = result[CNF_FILE_OPTION].as<std::string>();
      weightFormatOption = std::stoi(result[WEIGHT_FORMAT_OPTION].as<std::string>());
      clusteringHeuristicOption = std::stoi(result[CLUSTERING_HEURISTIC_OPTION].as<std::string>());
      formulaVarOrderingHeuristicOption = std::stoi(result[CLUSTER_VAR_ORDER_OPTION].as<std::string>());
      addVarOrderingHeuristicOption = std::stoi(result[DIAGRAM_VAR_ORDER_OPTION].as<std::string>());
      break;
    }
    default: {
      util::showError("must provide exactly one CNF file");
    }
  }
}

bool OptionDict::hasValidFilePath() {
  return filePath != DUMMY_STR;
}

/* namespaces *****************************************************************/

/* namespace testing **********************************************************/

void testing::test() {
  if (VERBOSITY >= 1) std::cout << "test\n\n";

  Formula formula(TEST_DIR + "weighted.cnf", WeightFormat::CACHET);
  if (VERBOSITY >= 1) {
    std::cout << "DIMACS declared var count: " << formula.getDeclaredVarCount() << "\n";
    util::printCnf(formula.getCnf());
    std::cout << "\n";
  }

  VarOrderingHeuristic addVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_OPTIONS.at(DEFAULT_ADD_VAR_ORDERING_HEURISTIC_OPTION);
  VarOrderingHeuristic formularVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_OPTIONS.at(DEFAULT_FORMULA_VAR_ORDERING_HEURISTIC_OPTION);

  MonolithicCounter monolithicCounter(addVarOrderingHeuristic, false);
  LinearCounter linearCounter(addVarOrderingHeuristic, false);
  BucketCounter bucketListCounter(false, formularVarOrderingHeuristic, false, addVarOrderingHeuristic, false);
  BucketCounter bucketTreeCounter(true, formularVarOrderingHeuristic, false, addVarOrderingHeuristic, false);
  BouquetCounter bouquetListCounter(false, formularVarOrderingHeuristic, false, addVarOrderingHeuristic, false);
  BouquetCounter bouquetTreeCounter(true, formularVarOrderingHeuristic, false, addVarOrderingHeuristic, false);

  int_t m = monolithicCounter.count(formula);
  int_t l = linearCounter.count(formula);
  int_t bel = bucketListCounter.count(formula);
  int_t bet = bucketTreeCounter.count(formula);
  int_t bml = bouquetListCounter.count(formula);
  int_t bmt = bouquetTreeCounter.count(formula);

  VectorT<int_t> counts = {m, l, bel, bet, bml, bmt};
  for (int_t i = 0; i < counts.size(); i++) {
    for (int_t j = i + 1; j < counts.size(); j++) {
      if (counts.at(i) != counts.at(j)) util::showWarning("different model counts");
    }
  }

  if (VERBOSITY >= 1) {
    std::cout << "\nmonolithic model count: " << m << "\n";
    std::cout << "linear model count: " << l << "\n";
    std::cout << "bucket list model count: " << bel << "\n";
    std::cout << "bucket tree model count: " << bet << "\n";
    std::cout << "bouquet list model count: " << bml << "\n";
    std::cout << "bouquet tree model count: " << bmt << "\n\n";
  }
}

/* namespace solving **********************************************************/

void solving::solveFile(const std::string &filePath, WeightFormat weightFormat, ClusteringHeuristic clusteringHeuristic, VarOrderingHeuristic formulaVarOrderingHeuristic, bool inverseFormulaVarOrdering, VarOrderingHeuristic addVarOrderingHeuristic, bool inverseAddVarOrdering) {
  std::cout << "\nReading DIMACS CNF file:\n";
  util::printRow("cnfFilePath", filePath);
  Formula formula(filePath, weightFormat);
  if (VERBOSITY >= 1) {
    formula.printLiteralWeights();
    formula.printCnf();
  }

  std::cout << "\nConstructing model counter:\n";
  util::printRow("weightFormat", util::getWeightFormatName(weightFormat));
  util::printRow("clustering", util::getClusteringHeuristicName(clusteringHeuristic));
  util::printRow("clusterVarOrder", util::getVarOrderingHeuristicName(formulaVarOrderingHeuristic));
  util::printRow("inverseClusterVarOrder", inverseFormulaVarOrdering);
  util::printRow("diagramVarOrder", util::getVarOrderingHeuristicName(addVarOrderingHeuristic));
  util::printRow("inverseDiagramVarOrder", inverseAddVarOrdering);

  std::cout << "\nCounting models...\n";
  double modelCount;
  switch (clusteringHeuristic) {
    case ClusteringHeuristic::MONOLITHIC: {
      MonolithicCounter monolithicCounter(addVarOrderingHeuristic, inverseAddVarOrdering);
      modelCount = monolithicCounter.count(formula);
      break;
    }
    case ClusteringHeuristic::LINEAR: {
      LinearCounter linearCounter(addVarOrderingHeuristic, inverseAddVarOrdering);
        modelCount = linearCounter.count(formula);
      break;
    }
    case ClusteringHeuristic::BUCKET_LIST: {
      BucketCounter bucketCounter(false, formulaVarOrderingHeuristic, inverseFormulaVarOrdering, addVarOrderingHeuristic, inverseAddVarOrdering);
      modelCount = bucketCounter.count(formula);
      break;
    }
    case ClusteringHeuristic::BUCKET_TREE: {
      BucketCounter bucketCounter(true, formulaVarOrderingHeuristic, inverseFormulaVarOrdering, addVarOrderingHeuristic, inverseAddVarOrdering);
      modelCount = bucketCounter.count(formula);
      break;
    }
    case ClusteringHeuristic::BOUQUET_LIST: {
      BouquetCounter bouquetCounter(false, formulaVarOrderingHeuristic, inverseFormulaVarOrdering, addVarOrderingHeuristic, inverseAddVarOrdering);
      modelCount = bouquetCounter.count(formula);
      break;
    }
    case ClusteringHeuristic::BOUQUET_TREE: {
      BouquetCounter bouquetCounter(true, formulaVarOrderingHeuristic, inverseFormulaVarOrdering, addVarOrderingHeuristic, inverseAddVarOrdering);
      modelCount = bouquetCounter.count(formula);
      break;
    }
    util::showError("no such clusteringHeuristic");
  }
  util::printRow("modelCount", modelCount);
}

void solving::solveOptions(const std::string &filePath, int_t weightFormatOption, int_t clusteringHeuristicOption, int_t formulaVarOrderingHeuristicOption, int_t addVarOrderingHeuristicOption) {
  WeightFormat weightFormat = WEIGHT_FORMAT_OPTIONS.at(weightFormatOption);

  ClusteringHeuristic clusteringHeuristic = CLUSTERING_HEURISTIC_OPTIONS.at(clusteringHeuristicOption);

  VarOrderingHeuristic formulaVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_OPTIONS.at(std::abs(formulaVarOrderingHeuristicOption));
  bool inverseFormulaVarOrdering = formulaVarOrderingHeuristicOption < 0;

  VarOrderingHeuristic addVarOrderingHeuristic = VAR_ORDERING_HEURISTIC_OPTIONS.at(std::abs(addVarOrderingHeuristicOption));
  bool inverseAddVarOrdering = addVarOrderingHeuristicOption < 0;

  solveFile(filePath, weightFormat, clusteringHeuristic, formulaVarOrderingHeuristic, inverseFormulaVarOrdering, addVarOrderingHeuristic, inverseAddVarOrdering);
}

void solving::solveCommand(int argc, char *argv[]) {
  OptionDict optionDict(argc, argv);
  if (optionDict.hasValidFilePath()) {
    time_point_t startTime = util::getTimePoint();
    solveOptions(
      optionDict.filePath,
      optionDict.weightFormatOption,
      optionDict.clusteringHeuristicOption,
      optionDict.formulaVarOrderingHeuristicOption,
      optionDict.addVarOrderingHeuristicOption
    );
    util::printDuration(startTime);
  }
}

/* globals ********************************************************************/

int main(int argc, char *argv[]) {
  std::cout << std::unitbuf; /* enables automatic flushing */
  std::cout << "\n";
  util::printThickLine();
  std::cout << "ADDMC: exact Model Counter using Algebraic Decision Diagrams\n";

  std::string version = "v1.0.0";
  std::string date = "2020/02/02";
  std::cout << "Version " << version << ", released on " << date << "\n";

  // mainVisual(argc, argv);
  // testing::test();
  solving::solveCommand(argc, argv);
}

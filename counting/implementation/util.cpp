/* includes *******************************************************************/

#include "../interface/util.hpp"

/* namespaces *****************************************************************/

/* namespace util *************************************************************/

bool util::isInt(double d) {
  double intPart;
  double fractionalPart = modf(d, &intPart);
  return fractionalPart == 0.0;
}

/* functions: options *********************************************************/

VectorT<std::string> util::getArgV(int argc, char *argv[]) {
  VectorT<std::string> argV;
  for (int_t i = 0; i < argc; i++) argV.push_back(std::string(argv[i]));
  return argV;
}

void util::printThickLine() {
  std::cout << "============================================================\n";
}

void util::printThinLine() {
  std::cout << "------------------------------------------------------------\n";
}

std::string util::getWeightFormatName(WeightFormat weightFormat) {
  switch (weightFormat) {
    case WeightFormat::UNWEIGHTED: return "UNWEIGHTED";
    case WeightFormat::MINIC2D: return "MINIC2D";
    case WeightFormat::CACHET: return "CACHET";
    default: showError("no such weightFormat"); return DUMMY_STR;
  }
}

std::string util::getClusteringHeuristicName(ClusteringHeuristic clusteringHeuristic) {
  switch (clusteringHeuristic) {
    case ClusteringHeuristic::MONOLITHIC: return "MONOLITHIC";
    case ClusteringHeuristic::LINEAR: return "LINEAR";
    case ClusteringHeuristic::BUCKET_LIST: return "BUCKET_LIST";
    case ClusteringHeuristic::BUCKET_TREE: return "BUCKET_TREE";
    case ClusteringHeuristic::BOUQUET_LIST: return "BOUQUET_LIST";
    case ClusteringHeuristic::BOUQUET_TREE: return "BOUQUET_TREE";
    default: showError("no such clusteringHeuristic"); return DUMMY_STR;
  }
}

std::string util::getVarOrderingHeuristicName(VarOrderingHeuristic varOrderingHeuristic) {
  switch (varOrderingHeuristic) {
    case VarOrderingHeuristic::APPEARANCE: return "APPEARANCE";
    case VarOrderingHeuristic::DECLARATION: return "DECLARATION";
    case VarOrderingHeuristic::RANDOM: return "RANDOM";
    case VarOrderingHeuristic::LEXP: return "LEXP";
    case VarOrderingHeuristic::LEXM: return "LEXM";
    case VarOrderingHeuristic::MCS: return "MCS";
    default: showError("no such varOrderingHeuristic"); return DUMMY_STR;
  }
}

void util::printWeightFormatOptions() {
  std::cout << "      --" << WEIGHT_FORMAT_OPTION << " arg  ";
  std::cout << "Weight format options:" << "\n";
  for (const auto &kv : WEIGHT_FORMAT_OPTIONS) {
    int num = kv.first;
    std::cout << "                 " << num << "\t" << std::left << std::setw(15) << getWeightFormatName(kv.second);
    if (num == std::abs(DEFAULT_WEIGHT_FORMAT_OPTION)) std::cout << "default: " << DEFAULT_WEIGHT_FORMAT_OPTION;
    std::cout << "\n";
  }
}

void util::printClusteringHeuristicOptions() {
  std::cout << "      --" << CLUSTERING_HEURISTIC_OPTION << " arg  ";
  std::cout << "Clustering heuristic options:" << "\n";
  for (const auto &kv : CLUSTERING_HEURISTIC_OPTIONS) {
    int num = kv.first;
    std::cout << "                 " << num << "\t" << std::left << std::setw(15) << getClusteringHeuristicName(kv.second);
    if (num == std::abs(DEFAULT_CLUSTERING_HEURISTIC_OPTION)) std::cout << "default: " << DEFAULT_CLUSTERING_HEURISTIC_OPTION;
    std::cout << "\n";
  }
}

void util::printFormulaVarOrderingHeuristicOptions() {
  std::cout << "      --" << CLUSTER_VAR_ORDER_OPTION << " arg  ";
  std::cout << "Cluster variable order heuristic options (negate to invert):" << "\n";
  for (const auto &kv : VAR_ORDERING_HEURISTIC_OPTIONS) {
    int num = kv.first;
    std::cout << "                 " << num << "\t" << std::left << std::setw(15) << getVarOrderingHeuristicName(kv.second);
    if (num == std::abs(DEFAULT_FORMULA_VAR_ORDERING_HEURISTIC_OPTION)) std::cout << "default: " << DEFAULT_FORMULA_VAR_ORDERING_HEURISTIC_OPTION;
    std::cout << "\n";
  }
}

void util::printAddVarOrderingHeuristicOptions() {
  std::cout << "      --" << DIAGRAM_VAR_ORDER_OPTION << " arg  ";
  std::cout << "Diagram variable order heuristic options (negate to invert):" << "\n";
  for (const auto &kv : VAR_ORDERING_HEURISTIC_OPTIONS) {
    int num = kv.first;
    std::cout << "                 " << num << "\t" << std::left << std::setw(15) << getVarOrderingHeuristicName(kv.second);
    if (num == std::abs(DEFAULT_ADD_VAR_ORDERING_HEURISTIC_OPTION)) std::cout << "default: " << DEFAULT_ADD_VAR_ORDERING_HEURISTIC_OPTION;
    std::cout << "\n";
  }
}

void util::printOptions() {
  // std::cout << "Command syntax:\n";
  // std::cout << "\taddmc <cnfFile> [<weightFormat> [<clusterPartition> <clusterVarOrder> <diagramVarOrder>]]\n\n";

  std::cout << " Optional options:\n";
  printWeightFormatOptions();
  printClusteringHeuristicOptions();
  printFormulaVarOrderingHeuristicOptions();
  printAddVarOrderingHeuristicOptions();
}

/* functions: CNF ***********************************************************/

int_t util::getFormulaVar(int_t literal) {
  if (literal == 0) showError("literal is 0");
  return std::abs(literal);
}

SetT<int_t> util::getClauseFormulaVars(const VectorT<int_t> &clause) {
  SetT<int_t> formulaVars;
  for (int_t literal : clause) formulaVars.insert(getFormulaVar(literal));
  return formulaVars;
}

SetT<int_t> util::getClusterFormulaVars(const VectorT<int_t> &cluster, const VectorT<VectorT<int_t>> &cnf) {
  SetT<int_t> formulaVars;
  for (int_t clauseIndex : cluster) unionize(formulaVars, getClauseFormulaVars(cnf.at(clauseIndex)));
  return formulaVars;
}

bool util::appearsIn(int_t formulaVar, const VectorT<int_t> &clause) {
  for (int_t literal : clause) if (getFormulaVar(literal) == formulaVar) return true;
  return false;
}

bool util::isPositiveLiteral(int_t literal) {
  if (literal == 0) showError("literal is 0");
  return literal > 0;
}

int_t util::getLiteralRank(int_t literal, const VectorT<int_t> &formulaVarOrdering) {
  int_t formularVar = getFormulaVar(literal);
  auto it = std::find(formulaVarOrdering.begin(), formulaVarOrdering.end(), formularVar);
  if (it == formulaVarOrdering.end()) showError("formularVar not found in formulaVarOrdering");
  return it - formulaVarOrdering.begin();
}

int_t util::getMinClauseRank(const VectorT<int_t> &clause, const VectorT<int_t> &formulaVarOrdering) {
  int_t minRank = DUMMY_MAX_INT;
  for (int_t literal : clause) {
    int_t rank = getLiteralRank(literal, formulaVarOrdering);
    if (rank < minRank) minRank = rank;
  }
  return minRank;
}

int_t util::getMaxClauseRank(const VectorT<int_t> &clause, const VectorT<int_t> &formulaVarOrdering) {
  int_t maxRank = DUMMY_MIN_INT;
  for (int_t literal : clause) {
    int_t rank = getLiteralRank(literal, formulaVarOrdering);
    if (rank > maxRank) maxRank = rank;
  }
  return maxRank;
}

void util::printClause(const VectorT<int_t> &clause) {
  std::cout << ":\t";
  for (int_t literal : clause) std::cout << std::right << std::setw(6) << literal << " ";
  std::cout << "\n";
}

void util::printCnf(const VectorT<VectorT<int_t>> &cnf) {
  std::cout << "cnf: {\n";
  for (int_t i = 0; i < cnf.size(); i++) {
    std::cout << "\tclause " << std::setw(5) << i + 1 << " ";
    printClause(cnf.at(i));
  }
  std::cout << "}\n";
}

void util::printLiteralWeights(const MapT<int_t, double> &literalWeights) {
  int_t maxFormulaVar = DUMMY_MIN_INT;
  for (const std::pair<int_t, double> &kv : literalWeights) {
    int_t formulaVar = kv.first;
    if (formulaVar > maxFormulaVar) maxFormulaVar = formulaVar;
  }

  std::cout << "literalWeights {\n";
  for (int_t formulaVar = 1; formulaVar <= maxFormulaVar; formulaVar++) {
    std::cout << "\t " << std::setw(5) << formulaVar << "\t" << std::setw(6) << literalWeights.at(formulaVar) << "\n";
    std::cout << "\t " << std::setw(5) << -formulaVar << "\t" << std::setw(6) << literalWeights.at(-formulaVar) << "\n";
  }
  std::cout << "}\n";
}

/* functions: timing **********************************************************/

time_point_t util::getTimePoint() {
  return std::chrono::system_clock::now();
}

std::chrono::milliseconds util::getDuration(time_point_t startTime) {
  time_point_t endTime = getTimePoint();
  return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
}

void util::printDuration(time_point_t startTime) {
  printRow("seconds", getDuration(startTime).count() / 1000.0);
}

/* functions: error handling **************************************************/

void util::showWarning(std::string message) {
  printThinLine();
  std::cout << "WARNING: " << message << "\n";
  printThinLine();
}

void util::showError(std::string message) {
  printThinLine();
  std::cout << "ERROR: " << message << "\n";
  printThinLine();
  throw std::logic_error("void error");
}

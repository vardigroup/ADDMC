/* inclusions *****************************************************************/

#include "../interface/util.hpp"

/* global variables ***********************************************************/

Int randomSeed = DEFAULT_RANDOM_SEED;
Int verbosityLevel = DEFAULT_VERBOSITY_LEVEL_CHOICE;
TimePoint startTime;

/* constants ******************************************************************/

const string &COMMENT_WORD = "c";
const string &PROBLEM_WORD = "p";

const string &STDIN_CONVENTION = "-";

const string &REQUIRED_OPTION_GROUP = "Required";
const string &OPTIONAL_OPTION_GROUP = "Optional";

const string &HELP_OPTION = "h, hi";
const string &CNF_FILE_OPTION = "cf";
const string &WEIGHT_FORMAT_OPTION = "wf";
const string &JT_FILE_OPTION = "jf";
const string &JT_WAIT_DURAION_OPTION = "jw";
const string &OUTPUT_FORMAT_OPTION = "of";
const string &CLUSTERING_HEURISTIC_OPTION = "ch";
const string &CLUSTER_VAR_ORDER_OPTION = "cv";
const string &DIAGRAM_VAR_ORDER_OPTION = "dv";
const string &RANDOM_SEED_OPTION = "rs";
const string &VERBOSITY_LEVEL_OPTION = "vl";

const std::map<Int, WeightFormat> WEIGHT_FORMAT_CHOICES = {
  {1, WeightFormat::UNWEIGHTED},
  {2, WeightFormat::MINIC2D},
  {3, WeightFormat::CACHET}, // buggy '-1' weight
  {4, WeightFormat::MCC} // weight line's trailing '0' is optional
};
const Int DEFAULT_WEIGHT_FORMAT_CHOICE = 4;

const Float DEFAULT_JT_WAIT_SECONDS = 10.0;

const std::map<Int, OutputFormat> OUTPUT_FORMAT_CHOICES = {
  {1, OutputFormat::JOIN_TREE},
  {2, OutputFormat::MODEL_COUNT}
};
const Int DEFAULT_OUTPUT_FORMAT_CHOICE = 2;

const std::map<Int, ClusteringHeuristic> CLUSTERING_HEURISTIC_CHOICES = {
  {1, ClusteringHeuristic::MONOLITHIC},
  {2, ClusteringHeuristic::LINEAR},
  {3, ClusteringHeuristic::BUCKET_LIST},
  {4, ClusteringHeuristic::BUCKET_TREE},
  {5, ClusteringHeuristic::BOUQUET_LIST},
  {6, ClusteringHeuristic::BOUQUET_TREE}
};
const Int DEFAULT_CLUSTERING_HEURISTIC_CHOICE = 6;

const std::map<Int, VarOrderingHeuristic> VAR_ORDERING_HEURISTIC_CHOICES = {
  {1, VarOrderingHeuristic::APPEARANCE},
  {2, VarOrderingHeuristic::DECLARATION},
  {3, VarOrderingHeuristic::RANDOM},
  {4, VarOrderingHeuristic::MCS},
  {5, VarOrderingHeuristic::LEXP},
  {6, VarOrderingHeuristic::LEXM}
};
const Int DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE = 5;
const Int DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE = 4;

const Int DEFAULT_RANDOM_SEED = 10;

const vector<Int> VERBOSITY_LEVEL_CHOICES = {0, 1, 2, 3, 4};
const Int DEFAULT_VERBOSITY_LEVEL_CHOICE = 0;

const Float NEGATIVE_INFINITY = -std::numeric_limits<Float>::infinity();

const Int DUMMY_MIN_INT = std::numeric_limits<Int>::min();
const Int DUMMY_MAX_INT = std::numeric_limits<Int>::max();

const string &DUMMY_STR = "";

const string &DOT_DIR = "./";

/* namespaces *****************************************************************/

/* namespace util *************************************************************/

bool util::isInt(Float d) {
  Float intPart;
  Float fractionalPart = modf(d, &intPart);
  return fractionalPart == 0.0;
}

/* functions: printing ********************************************************/

void util::printComment(const string &message, Int preceedingNewLines, Int followingNewLines, bool commented) {
  for (Int i = 0; i < preceedingNewLines; i++) cout << "\n";
  cout << (commented ? COMMENT_WORD + " " : "") << message;
  for (Int i = 0; i < followingNewLines; i++) cout << "\n";
}

void util::printSolutionLine(WeightFormat weightFormat, Float modelCount, Int preceedingThinLines, Int followingThinLines) {
  for (Int i = 0; i < preceedingThinLines; i++) printThinLine();
  cout << "s " << (weightFormat == WeightFormat::UNWEIGHTED ? "mc" : "wmc") << " " << modelCount << "\n";
  for (Int i = 0; i < followingThinLines; i++) printThinLine();
}

void util::printBoldLine(bool commented) {
  printComment("******************************************************************", 0, 1, commented);
}

void util::printThickLine(bool commented) {
  printComment("==================================================================", 0, 1, commented);
}

void util::printThinLine() {
  printComment("------------------------------------------------------------------");
}

void util::printHelpOption() {
  cout << "  -h, --hi      help information\n";
}

void util::printCnfFileOption() {
  cout << "      --" << CNF_FILE_OPTION << std::left << std::setw(56) << " arg  cnf file path (to use stdin, type: '--" + CNF_FILE_OPTION + " -')";
  cout << "Default: -\n";
}

void util::printWeightFormatOption() {
  cout << "      --" << WEIGHT_FORMAT_OPTION << " arg  ";
  cout << "weight format in cnf file:\n";
  for (const auto &kv : WEIGHT_FORMAT_CHOICES) {
    int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getWeightFormatName(kv.second);
    if (num == DEFAULT_WEIGHT_FORMAT_CHOICE) cout << "Default: " << DEFAULT_WEIGHT_FORMAT_CHOICE;
    cout << "\n";
  }
}

void util::printJtFileOption() {
  cout << "      --" << JT_FILE_OPTION << std::left << std::setw(56) << " arg  jt file path (to use stdin, type: '--" + JT_FILE_OPTION + " -')";
  cout << "Default: (no jt file)\n";
}

void util::printJtWaitOption() {
  cout << "      --" << JT_WAIT_DURAION_OPTION << std::left << std::setw(56) << " arg  jt wait duration before tree builder is killed";
  cout << "Default: " + to_string(DEFAULT_JT_WAIT_SECONDS) + " (seconds)\n";
}

void util::printOutputFormatOption() {
  cout << "      --" << OUTPUT_FORMAT_OPTION << " arg  ";
  cout << "output format:\n";
  for (const auto &kv : OUTPUT_FORMAT_CHOICES) {
    int num = kv.first;
    cout << "           " << num << "    " << getOutputFormatName(kv.second);
    if (num == DEFAULT_OUTPUT_FORMAT_CHOICE) {
      cout << std::left << std::setw(39) << " (using input jt file if provided)";
      cout << "Default: " << DEFAULT_OUTPUT_FORMAT_CHOICE;
    }
    cout << "\n";
  }
}

void util::printClusteringHeuristicOption() {
  cout << "      --" << CLUSTERING_HEURISTIC_OPTION << " arg  ";
  cout << "clustering heuristic:\n";
  for (const auto &kv : CLUSTERING_HEURISTIC_CHOICES) {
    int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getClusteringHeuristicName(kv.second);
    if (num == DEFAULT_CLUSTERING_HEURISTIC_CHOICE) cout << "Default: " << DEFAULT_CLUSTERING_HEURISTIC_CHOICE;
    cout << "\n";
  }
}

void util::printCnfVarOrderingHeuristicOption() {
  cout << "      --" << CLUSTER_VAR_ORDER_OPTION << " arg  ";
  cout << "cluster variable order heuristic (negate to invert):\n";
  for (const auto &kv : VAR_ORDERING_HEURISTIC_CHOICES) {
    int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getVarOrderingHeuristicName(kv.second);
    if (num == std::abs(DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE)) cout << "Default: " << DEFAULT_CNF_VAR_ORDERING_HEURISTIC_CHOICE;
    cout << "\n";
  }
}

void util::printDdVarOrderingHeuristicOption() {
  cout << "      --" << DIAGRAM_VAR_ORDER_OPTION << " arg  ";
  cout << "diagram variable order heuristic (negate to invert):\n";
  for (const auto &kv : VAR_ORDERING_HEURISTIC_CHOICES) {
    int num = kv.first;
    cout << "           " << num << "    " << std::left << std::setw(50) << getVarOrderingHeuristicName(kv.second);
    if (num == std::abs(DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE)) cout << "Default: " << DEFAULT_DD_VAR_ORDERING_HEURISTIC_CHOICE;
    cout << "\n";
  }
}

void util::printRandomSeedOption() {
  cout << "      --" << RANDOM_SEED_OPTION << std::left << std::setw(56) << " arg  random seed";
  cout << "Default: " + to_string(DEFAULT_RANDOM_SEED) + "\n";
}

void util::printVerbosityLevelOption() {
  cout << "      --" << VERBOSITY_LEVEL_OPTION << " arg  ";
  cout << "verbosity level:\n";
  for (Int verbosityLevelOption : VERBOSITY_LEVEL_CHOICES) {
    cout << "           " << verbosityLevelOption << "    " << std::left << std::setw(50) << getVerbosityLevelName(verbosityLevelOption);
    if (verbosityLevelOption == DEFAULT_VERBOSITY_LEVEL_CHOICE) cout << "Default: " << DEFAULT_VERBOSITY_LEVEL_CHOICE;
    cout << "\n";
  }
}

/* functions: argument parsing ************************************************/

vector<string> util::getArgV(int argc, char *argv[]) {
  vector<string> argV;
  for (Int i = 0; i < argc; i++) argV.push_back(string(argv[i]));
  return argV;
}

string util::getWeightFormatName(WeightFormat weightFormat) {
  switch (weightFormat) {
    case WeightFormat::UNWEIGHTED: {
      return "UNWEIGHTED";
    }
    case WeightFormat::MINIC2D: {
      return "MINIC2D";
    }
    case WeightFormat::CACHET: {
      return "CACHET";
    }
    case WeightFormat::MCC: {
      return "MCC";
    }
    default: {
      showError("no such weightFormat");
      return DUMMY_STR;
    }
  }
}

string util::getOutputFormatName(OutputFormat outputFormat) {
  switch (outputFormat) {
    case OutputFormat::JOIN_TREE: {
      return "JOIN_TREE";
    }
    case OutputFormat::MODEL_COUNT: {
      return "MODEL_COUNT";
    }
    default: {
      showError("no such outputFormat");
      return DUMMY_STR;
    }
  }
}

string util::getClusteringHeuristicName(ClusteringHeuristic clusteringHeuristic) {
  switch (clusteringHeuristic) {
    case ClusteringHeuristic::MONOLITHIC: {
      return "MONOLITHIC";
    }
    case ClusteringHeuristic::LINEAR: {
      return "LINEAR";
    }
    case ClusteringHeuristic::BUCKET_LIST: {
      return "BUCKET_LIST";
    }
    case ClusteringHeuristic::BUCKET_TREE: {
      return "BUCKET_TREE";
    }
    case ClusteringHeuristic::BOUQUET_LIST: {
      return "BOUQUET_LIST";
    }
    case ClusteringHeuristic::BOUQUET_TREE: {
      return "BOUQUET_TREE";
    }
    default: {
      showError("no such clusteringHeuristic");
      return DUMMY_STR;
    }
  }
}

string util::getVarOrderingHeuristicName(VarOrderingHeuristic varOrderingHeuristic) {
  switch (varOrderingHeuristic) {
    case VarOrderingHeuristic::APPEARANCE: {
      return "APPEARANCE";
    }
    case VarOrderingHeuristic::DECLARATION: {
      return "DECLARATION";
    }
    case VarOrderingHeuristic::RANDOM: {
      return "RANDOM";
    }
    case VarOrderingHeuristic::LEXP: {
      return "LEXP";
    }
    case VarOrderingHeuristic::LEXM: {
      return "LEXM";
    }
    case VarOrderingHeuristic::MCS: {
      return "MCS";
    }
    default: {
      showError("DUMMY_VAR_ORDERING_HEURISTIC in util::getVarOrderingHeuristicName");
      return DUMMY_STR;
    }
  }
}

string util::getVerbosityLevelName(Int verbosityLevel) {
  switch (verbosityLevel) {
    case 0: {
      return "solution only";
    }
    case 1: {
      return "parsed info as well";
    }
    case 2: {
      return "clusters as well";
    }
    case 3: {
      return "cnf literal weights as well";
    }
    case 4: {
      return "input lines as well";
    }
    default: {
      showError("no such verbosityLevel");
      return DUMMY_STR;
    }
  }
}

/* functions: CNF *************************************************************/

Int util::getCnfVar(Int literal) {
  if (literal == 0) {
    showError("literal is 0");
  }
  return std::abs(literal);
}

Set<Int> util::getClauseCnfVars(const vector<Int> &clause) {
  Set<Int> cnfVars;
  for (Int literal : clause) cnfVars.insert(getCnfVar(literal));
  return cnfVars;
}

Set<Int> util::getClusterCnfVars(const vector<Int> &cluster, const vector<vector<Int>> &clauses) {
  Set<Int> cnfVars;
  for (Int clauseIndex : cluster) unionize(cnfVars, getClauseCnfVars(clauses.at(clauseIndex)));
  return cnfVars;
}

bool util::appearsIn(Int cnfVar, const vector<Int> &clause) {
  for (Int literal : clause) if (getCnfVar(literal) == cnfVar) return true;
  return false;
}

bool util::isPositiveLiteral(Int literal) {
  if (literal == 0) showError("literal is 0");
  return literal > 0;
}

Int util::getLiteralRank(Int literal, const vector<Int> &cnfVarOrdering) {
  Int cnfVar = getCnfVar(literal);
  auto it = std::find(cnfVarOrdering.begin(), cnfVarOrdering.end(), cnfVar);
  if (it == cnfVarOrdering.end()) showError("cnfVar not found in cnfVarOrdering");
  return it - cnfVarOrdering.begin();
}

Int util::getMinClauseRank(const vector<Int> &clause, const vector<Int> &cnfVarOrdering) {
  Int minRank = DUMMY_MAX_INT;
  for (Int literal : clause) {
    Int rank = getLiteralRank(literal, cnfVarOrdering);
    if (rank < minRank) minRank = rank;
  }
  return minRank;
}

Int util::getMaxClauseRank(const vector<Int> &clause, const vector<Int> &cnfVarOrdering) {
  Int maxRank = DUMMY_MIN_INT;
  for (Int literal : clause) {
    Int rank = getLiteralRank(literal, cnfVarOrdering);
    if (rank > maxRank) maxRank = rank;
  }
  return maxRank;
}

void util::printClause(const vector<Int> &clause) {
  for (Int literal : clause) {
    cout << std::right << std::setw(5) << literal << " ";
  }
  cout << "\n";
}

void util::printCnf(const vector<vector<Int>> &clauses) {
  printThinLine();
  printComment("cnf {");
  for (Int i = 0; i < clauses.size(); i++) {
    cout << COMMENT_WORD << "\t" "clause ";
    cout << std::right << std::setw(5) << i + 1 << " : ";
    printClause(clauses.at(i));
  }
  printComment("}");
  printThinLine();
}

void util::printLiteralWeights(const Map<Int, Float> &literalWeights) {
  Int maxCnfVar = DUMMY_MIN_INT;
  for (const std::pair<Int, Float> &kv : literalWeights) {
    Int cnfVar = kv.first;
    if (cnfVar > maxCnfVar) {
      maxCnfVar = cnfVar;
    }
  }

  printThinLine();
  printComment("literalWeights {");
  cout << std::right;
  for (Int cnfVar = 1; cnfVar <= maxCnfVar; cnfVar++) {
    cout << COMMENT_WORD << " " << std::right << std::setw(10) << cnfVar << "\t" << std::left << std::setw(15) << literalWeights.at(cnfVar) << "\n";
    cout << COMMENT_WORD << " " << std::right << std::setw(10) << -cnfVar << "\t" << std::left << std::setw(15) << literalWeights.at(-cnfVar) << "\n";
  }
  printComment("}");
  printThinLine();
}

/* functions: timing **********************************************************/

TimePoint util::getTimePoint() {
  return std::chrono::steady_clock::now();
}

Float util::getSeconds(TimePoint startTime) {
  TimePoint endTime = getTimePoint();
  return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
}

void util::printDuration(TimePoint startTime) {
  printThickLine();
  printRow("seconds", getSeconds(startTime));
  printThickLine();
}

/* functions: error handling **************************************************/

void util::showWarning(const string &message, bool commented) {
  printBoldLine(commented);
  printComment("MY_WARNING: " + message, 0, 1, commented);
  printBoldLine(commented);
}

void util::showError(const string &message, bool commented) {
  throw MyError(message, commented);
}

/* classes ********************************************************************/

/* class MyError **************************************************************/

MyError::MyError(const string &message, bool commented) {
  util::printBoldLine(commented);
  util::printComment("MY_ERROR: " + message, 0, 1, commented);
  util::printBoldLine(commented);
}

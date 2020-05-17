#pragma once

/* includes *******************************************************************/

#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include <unordered_map>
#include <unordered_set>

#include "../../libraries/cudd-3.0.0/cplusplus/cuddObj.hh"
#include "../../libraries/cudd-3.0.0/cudd/cuddInt.h"

/* types **********************************************************************/

using int_t = int_fast64_t;
using time_point_t = std::chrono::time_point<std::chrono::system_clock>;

template<typename K, typename V> using MapT = std::unordered_map<K, V>;
template<typename T> using SetT = std::unordered_set<T>;
template<typename T> using VectorT = std::vector<T>;

/* constants ******************************************************************/

const int_t VERBOSITY = 0; /* 0: prints warnings/errors, 1: prints debugging info */

const std::string REQUIRED_OPTION_GROUP = "Required";
const std::string CNF_FILE_OPTION = "cf";

const std::string OPTIONAL_OPTION_GROUP = "Optional";
const std::string WEIGHT_FORMAT_OPTION = "wf";
const std::string CLUSTERING_HEURISTIC_OPTION = "ch";
const std::string CLUSTER_VAR_ORDER_OPTION = "cv";
const std::string DIAGRAM_VAR_ORDER_OPTION = "dv";

enum class WeightFormat { UNWEIGHTED, MINIC2D, CACHET };
const std::map<int_t, WeightFormat> WEIGHT_FORMAT_OPTIONS = {
  {1, WeightFormat::UNWEIGHTED},
  {2, WeightFormat::MINIC2D},
  {3, WeightFormat::CACHET} /* buggy -1 weight */
};
const int_t DEFAULT_WEIGHT_FORMAT_OPTION = 2;

enum class ClusteringHeuristic { MONOLITHIC, LINEAR, BUCKET_LIST, BUCKET_TREE, BOUQUET_LIST, BOUQUET_TREE };
const std::map<int_t, ClusteringHeuristic> CLUSTERING_HEURISTIC_OPTIONS = {
  {1, ClusteringHeuristic::MONOLITHIC},
  {2, ClusteringHeuristic::LINEAR},
  {3, ClusteringHeuristic::BUCKET_LIST},
  {4, ClusteringHeuristic::BUCKET_TREE},
  {5, ClusteringHeuristic::BOUQUET_LIST},
  {6, ClusteringHeuristic::BOUQUET_TREE}
};
const int_t DEFAULT_CLUSTERING_HEURISTIC_OPTION = 6;

enum class VarOrderingHeuristic { APPEARANCE, DECLARATION, RANDOM, MCS, LEXP, LEXM };
const std::map<int_t, VarOrderingHeuristic> VAR_ORDERING_HEURISTIC_OPTIONS = {
  {1, VarOrderingHeuristic::APPEARANCE},
  {2, VarOrderingHeuristic::DECLARATION},
  {3, VarOrderingHeuristic::RANDOM},
  {4, VarOrderingHeuristic::MCS},
  {5, VarOrderingHeuristic::LEXP},
  {6, VarOrderingHeuristic::LEXM},
};
const int_t DEFAULT_FORMULA_VAR_ORDERING_HEURISTIC_OPTION = 5;
const int_t DEFAULT_ADD_VAR_ORDERING_HEURISTIC_OPTION = 4;

const double DEFAULT_VAR_WEIGHT = 0.5; /* Cachet */

const double DUMMY_MODEL_COUNT = -42;

const int_t DUMMY_MIN_INT = std::numeric_limits<int_t>::min();
const int_t DUMMY_MAX_INT = std::numeric_limits<int_t>::max();

const std::string DUMMY_STR = "DUMMY_STR";

const std::string DOT_DIR = "../tests/";
const std::string TEST_DIR = "../tests/test/";

/* namespaces *****************************************************************/

namespace util {
  bool isInt(double d);

  /* functions: options *******************************************************/

  VectorT<std::string> getArgV(int argc, char *argv[]);

  void printThickLine();
  void printThinLine();

  std::string getWeightFormatName(WeightFormat weightFormat);
  std::string getClusteringHeuristicName(ClusteringHeuristic clusteringHeuristic);
  std::string getVarOrderingHeuristicName(VarOrderingHeuristic varOrderingHeuristic);

  void printWeightFormatOptions();
  void printClusteringHeuristicOptions();
  void printFormulaVarOrderingHeuristicOptions();
  void printAddVarOrderingHeuristicOptions();
  void printOptions();

  /* functions: CNF ***********************************************************/

  int_t getFormulaVar(int_t literal);
  SetT<int_t> getClauseFormulaVars(const VectorT<int_t> &clause);
  SetT<int_t> getClusterFormulaVars(const VectorT<int_t> &cluster, const VectorT<VectorT<int_t>> &cnf);

  bool appearsIn(int_t formulaVar, const VectorT<int_t> &clause);
  bool isPositiveLiteral(int_t literal);

  int_t getLiteralRank(int_t literal, const VectorT<int_t> &formulaVarOrdering);
  int_t getMinClauseRank(const VectorT<int_t> &clause, const VectorT<int_t> &formulaVarOrdering);
  int_t getMaxClauseRank(const VectorT<int_t> &clause, const VectorT<int_t> &formulaVarOrdering);

  void printClause(const VectorT<int_t> &clause);
  void printCnf(const VectorT<VectorT<int_t>> &cnf);
  void printLiteralWeights(const MapT<int_t, double> &literalWeights);

  /* functions: timing ********************************************************/

  time_point_t getTimePoint();
  std::chrono::milliseconds getDuration(time_point_t startTime);
  void printDuration(time_point_t startTime);

  /* functions: error handling ************************************************/

  void showWarning(std::string message = "");
  void showError(std::string message = "");

  /* functions: templates implemented in headers to avoid linker errors *******/

  template<typename T> void printRow(const std::string &name, const T &value) {
    int_t COLUMN_WIDTH = 30;
    std::cout << "* " << std::left << std::setw(COLUMN_WIDTH) << name << value << "\n";
  }

  template<typename T> void printContainer(const T &container) {
    std::cout << "printContainer:\n";
    for (const auto &member : container) {
      std::cout << "\t" << member << "\t";
    }
    std::cout << "\n";
  }

  template<typename K, typename V> void printMap(const MapT<K, V> &m) {
    std::cout << "printMap:\n";
    for (const auto &kv : m) {
      std::cout << "\t" << kv.first << "\t:\t" << kv.second << "\n";
    }
    std::cout << "\n";
  }

  template<typename Key, typename Value> bool isLessValued(std::pair<Key, Value> a, std::pair<Key, Value> b) {
    return a.second < b.second;
  }

  template<typename T> T getSoleMember(const VectorT<T> &v) {
    if (v.size() != 1) showError("vector is not singleton");
    return v.front();
  }

  template<typename T> void popBack(T &element, VectorT<T> &v) {
    if (v.empty()) showError("vector is empty");
    element = v.back();
    v.pop_back();
  }

  template<typename T> void invert(T &t) {
    std::reverse(t.begin(), t.end());
  }

  template<typename T, typename U> bool isFound(const T &element, const U &container) {
    return std::find(std::begin(container), std::end(container), element) != std::end(container);
  }

  template<typename T, typename U1, typename U2> void differ(SetT<T> &diff, const U1 &members, const U2 &nonmembers) {
    for (const auto &member : members) if (!isFound(member, nonmembers)) diff.insert(member);
  }

  template<typename T, typename U> void unionize(SetT<T> &unionSet, const U &container) {
    for (const auto &member : container) unionSet.insert(member);
  }

  template<typename T, typename U> bool isDisjoint(const T &container, const U &container2) {
    for (const auto &member : container) {
      for (const auto &member2 : container2) {
        if (member == member2) {
          return false;
        }
      }
    }
    return true;
  }

  template<typename T> double adjustModelCount(double apparentModelCount, const T &projectedFormulaVars, const MapT<int_t, double> &literalWeights) {
    double totalModelCount = apparentModelCount;

    int_t totalLiteralCount = literalWeights.size();
    if (totalLiteralCount % 2 == 1) showError("odd total literal count");

    int_t totalVarCount = totalLiteralCount / 2;
    if (totalVarCount < projectedFormulaVars.size()) showError("more projected vars than total vars");

    for (int_t formulaVar = 1; formulaVar <= totalVarCount; formulaVar++) {
      if (!isFound(formulaVar, projectedFormulaVars)) {
        totalModelCount *= literalWeights.at(formulaVar) + literalWeights.at(-formulaVar);
      }
    }

    if (totalModelCount == 0) {
      showWarning("floating-point underflow may have occured; please double-check with another tool");
    }
    return totalModelCount;
  }

  template<typename T> void shuffleRandomly(T &container) {
    std::mt19937 generator;
    generator.seed(314); /* for reproducibility */
    std::shuffle(container.begin(), container.end(), generator);
  }

  template<typename Dd> SetT<int_t> getSupport(const Dd &dd) {
    SetT<int_t> support;
    for (int_t ddVar : dd.SupportIndices()) support.insert(ddVar);
    return support;
  }

  template<typename Dd> SetT<int_t> getSupportSuperset(const VectorT<Dd> &dds) {
    SetT<int_t> supersupport;
    for (const Dd &dd : dds) for (int_t var : dd.SupportIndices()) supersupport.insert(var);
    return supersupport;
  }

  template<typename Dd> int_t getMinDdRank(const Dd &dd, const VectorT<int_t> &ddVarToFormulaVarMap, const VectorT<int_t> &formulaVarOrdering) {
    int_t minRank = DUMMY_MAX_INT;
    for (int_t ddVar : getSupport(dd)) {
      int_t formulaVar = ddVarToFormulaVarMap.at(ddVar);
      int_t rank = getLiteralRank(formulaVar, formulaVarOrdering);
      if (rank < minRank) minRank = rank;
    }
    return minRank;
  }

  template<typename Dd> int_t getMaxDdRank(const Dd &dd, const VectorT<int_t> &ddVarToFormulaVarMap, const VectorT<int_t> &formulaVarOrdering) {
    int_t maxRank = DUMMY_MIN_INT;
    for (int_t ddVar : getSupport(dd)) {
      int_t formulaVar = ddVarToFormulaVarMap.at(ddVar);
      int_t rank = getLiteralRank(formulaVar, formulaVarOrdering);
      if (rank > maxRank) maxRank = rank;
    }
    return maxRank;
  }

  template<typename Dd> int_t getDdRank(const Dd &dd, const VectorT<int_t> &ddVarToFormulaVarMap, const VectorT<int_t> &formulaVarOrdering, bool minRank) {
    return minRank ? getMinDdRank(dd, ddVarToFormulaVarMap, formulaVarOrdering) : getMaxDdRank(dd, ddVarToFormulaVarMap, formulaVarOrdering);
  }
}

/* globals ********************************************************************/

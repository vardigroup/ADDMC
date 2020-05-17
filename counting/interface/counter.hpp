#pragma once

/* includes *******************************************************************/

#include "formula.hpp"
#include "visual.hpp"

/* namespaces *****************************************************************/

namespace dd {
  double getTerminalValue(const ADD &terminal);
  double countConstAddFloat(const ADD &add);
  int_t countConstAddInt(const ADD &add);
  void printMaxAddVarCount(int_t maxAddVarCount);
}

/* classes ********************************************************************/

class Counter { /* abstract */
protected:
  int_t dotFileIndex = 1;
  Cudd mgr;
  VarOrderingHeuristic addVarOrderingHeuristic;
  bool inverseAddVarOrdering;
  MapT<int_t, int_t> formulaVarToAddVarMap; /* e.g. {42: 0, 13: 1} */
  VectorT<int_t> addVarToFormulaVarMap; /* e.g. [42, 13], i.e. addVarOrdering */

  void writeDotFile(ADD &add, std::string dotFileDir = DOT_DIR);
  template<typename T> SetT<int_t> getFormulaVars(const T &addVars) {
    SetT<int_t> formulaVars;
    for (int_t addVar : addVars) formulaVars.insert(addVarToFormulaVarMap.at(addVar));
    return formulaVars;
  }
  const VectorT<int_t> &getAddVarOrdering() const; /* addVarToFormulaVarMap */
  void orderAddVars(const Formula &formula); /* writes: formulaVarToAddVarMap, addVarToFormulaVarMap */
  ADD getClauseAdd(const VectorT<int_t> &clause);
  void abstract(ADD &add, int_t addVar, const MapT<int_t, double> &literalWeights);
  void abstractCube(ADD &add, const SetT<int_t> &addVars, const MapT<int_t, double> &literalWeights);

public:
  virtual double count(const Formula &formula) = 0;
  double count(const std::string &filePath, WeightFormat weightFormat);
};

class MonolithicCounter : public Counter { /* builds an ADD for the entire CNF */
protected:
  void setMonolithicClauseAdds(VectorT<ADD> &clauseAdds, const Formula &formula);
  void setCnfAdd(ADD &cnfAdd, const Formula &formula);

public:
  double count(const Formula &formula);
  MonolithicCounter(VarOrderingHeuristic addVarOrderingHeuristic, bool inverseAddVarOrdering);
};

class FactoredCounter : public Counter {}; /* abstract; builds an ADD for each clause */

class LinearCounter : public FactoredCounter { /* combines adjacent clauses */
protected:
  void setLinearClauseAdds(VectorT<ADD> &clauseAdds, const Formula &formula);

public:
  double count(const Formula &formula);
  LinearCounter(VarOrderingHeuristic addVarOrderingHeuristic, bool inverseAddVarOrdering);
};

class NonlinearCounter : public FactoredCounter { /* abstract; puts clauses in clusters */
protected:
  bool clusterTree;
  VarOrderingHeuristic formulaVarOrderingHeuristic;
  bool inverseFormulaVarOrdering;
  VectorT<VectorT<int_t>> clusters; /* clusterIndex |-> clauseIndices */
  VectorT<VectorT<ADD>> addClusters; /* clusterIndex |-> adds (for cluster tree) */
  VectorT<SetT<int_t>> projectingAddVarSets; /* clusterIndex |-> addVars (for cluster tree) */

  SetT<int_t> getProjectingAddVars(int_t clusterIndex, bool minRank, const VectorT<int_t> &formulaVarOrdering, const VectorT<VectorT<int_t>> &cnf);
  void printClusters(const VectorT<VectorT<int_t>> &cnf) const;
  void fillClusters(const VectorT<VectorT<int_t>> &cnf, const VectorT<int_t> &formulaVarOrdering, bool minRank);
  void fillAddClusters(const VectorT<VectorT<int_t>> &cnf, const VectorT<int_t> &formulaVarOrdering, bool minRank); /* (for cluster tree) */
  void fillProjectingAddVarSets(const VectorT<VectorT<int_t>> &cnf, const VectorT<int_t> &formulaVarOrdering, bool minRank); /* (for cluster tree) */
  int_t getNewClusterIndex(const ADD &abstractedClusterAdd, const VectorT<int_t> &formulaVarOrdering, bool minRank); /* returns DUMMY_MAX_INT if no var remains (for cluster tree) */
  int_t getNewClusterIndex(const SetT<int_t> &remainingAddVars); /* returns DUMMY_MAX_INT if no var remains (for cluster tree) #MAVC */
  double countWithList(const Formula &formula, bool minRank);
  double countWithTree(const Formula &formula, bool minRank);
  double countWithTree(const Formula &formula); /* #MAVC */
};

class BucketCounter : public NonlinearCounter { /* bucket elimination */
public:
  double count(const Formula &formula);
  BucketCounter(
    bool clusterTree,
    VarOrderingHeuristic formulaVarOrderingHeuristic,
    bool inverseFormulaVarOrdering,
    VarOrderingHeuristic addVarOrderingHeuristic,
    bool inverseAddVarOrdering);
};

class BouquetCounter : public NonlinearCounter { /* Bouquet's Method */
public:
  double count(const Formula &formula);  /* #MAVC */
  BouquetCounter(
    bool clusterTree,
    VarOrderingHeuristic formulaVarOrderingHeuristic,
    bool inverseFormulaVarOrdering,
    VarOrderingHeuristic addVarOrderingHeuristic,
    bool inverseAddVarOrdering);
};

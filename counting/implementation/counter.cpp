/* includes *******************************************************************/

#include "../interface/counter.hpp"

/* namespaces *****************************************************************/

/* namespace dd ***************************************************************/

double dd::getTerminalValue(const ADD &terminal) {
  DdNode *node = terminal.getNode();
  return (node -> type).value;
}

double dd::countConstAddFloat(const ADD &add) {
  ADD minTerminal = add.FindMin();
  ADD maxTerminal = add.FindMax();

  double minValue = getTerminalValue(minTerminal);
  double maxValue = getTerminalValue(maxTerminal);

  if (minValue != maxValue) {
    util::showError("ADD is nonconst: min value " + std::to_string(minValue) +
      ", max value " + std::to_string(maxValue));
  }

  return minValue;
}

int_t dd::countConstAddInt(const ADD &add) {
  double value = countConstAddFloat(add);

  if (!util::isInt(value)) util::showError("unweighted model count is not int");

  return value;
}

void dd::printMaxAddVarCount(int_t maxAddVarCount) {
  util::printRow("maxAddVarCount", maxAddVarCount);
}

/* classes ********************************************************************/

/* class Counter **************************************************************/

void Counter::writeDotFile(ADD &add, std::string dotFileDir) {
  if (VERBOSITY >= 1) {
    writeDd(mgr, add, dotFileDir + "dd" + std::to_string(dotFileIndex) + ".dot");
    dotFileIndex++;
  }
}

const VectorT<int_t> &Counter::getAddVarOrdering() const {
  return addVarToFormulaVarMap;
}

void Counter::orderAddVars(const Formula &formula) {
  addVarToFormulaVarMap = formula.getVarOrdering(addVarOrderingHeuristic, inverseAddVarOrdering);
  for (int_t addVar = 0; addVar < addVarToFormulaVarMap.size(); addVar++) {
    int_t formulaVar = addVarToFormulaVarMap.at(addVar);
    formulaVarToAddVarMap[formulaVar] = addVar;
    mgr.addVar(addVar); /* creates addVar-th ADD var */
  }
}

ADD Counter::getClauseAdd(const VectorT<int_t> &clause) {
  ADD clauseAdd = mgr.addZero();
  for (int_t literal : clause) {
    int_t addVar = formulaVarToAddVarMap.at(util::getFormulaVar(literal));
    ADD literalAdd = mgr.addVar(addVar);
    if (!util::isPositiveLiteral(literal)) literalAdd = ~literalAdd;
    clauseAdd |= literalAdd;
  }
  return clauseAdd;
}

void Counter::abstract(ADD &add, int_t addVar, const MapT<int_t, double> &literalWeights) {
  int_t formulaVar = addVarToFormulaVarMap.at(addVar);
  ADD positiveWeight = mgr.constant(literalWeights.at(formulaVar));
  ADD negativeWeight = mgr.constant(literalWeights.at(-formulaVar));

  add = positiveWeight * add.Compose(mgr.addOne(), addVar) + negativeWeight * add.Compose(mgr.addZero(), addVar);
}

void Counter::abstractCube(ADD &add, const SetT<int_t> &addVars, const MapT<int_t, double> &literalWeights) {
  for (int_t addVar :addVars) {
    abstract(add, addVar, literalWeights);
  }
}

double Counter::count(const std::string &filePath, WeightFormat weightFormat) {
  Formula formula(filePath, weightFormat);
  return count(formula);
}

/* class MonolithicCounter ****************************************************/

void MonolithicCounter::setMonolithicClauseAdds(VectorT<ADD> &clauseAdds, const Formula &formula) {
  clauseAdds.clear();
  for (const VectorT<int_t> &clause : formula.getCnf()) {
    ADD clauseAdd = getClauseAdd(clause);
    clauseAdds.push_back(clauseAdd);
  }
}

void MonolithicCounter::setCnfAdd(ADD &cnfAdd, const Formula &formula) {
  VectorT<ADD> clauseAdds;
  setMonolithicClauseAdds(clauseAdds, formula);
  cnfAdd = mgr.addOne();
  for (const ADD &clauseAdd : clauseAdds) {
    cnfAdd &= clauseAdd; /* operator& is operator* in class ADD */
  }
}

double MonolithicCounter::count(const Formula &formula) {
  orderAddVars(formula);

  ADD cnfAdd;
  setCnfAdd(cnfAdd, formula);
  writeDotFile(cnfAdd);

  SetT<int_t> support = util::getSupport(cnfAdd);
  for (int_t addVar : support) {
    abstract(cnfAdd, addVar, formula.getLiteralWeights());
    if (VERBOSITY >= 1) std::cout << "Abstracted ADD var:\t" << addVar << "\n";

    writeDotFile(cnfAdd);
  }

  double modelCount = dd::countConstAddFloat(cnfAdd);
  modelCount = util::adjustModelCount(modelCount, getFormulaVars(support), formula.getLiteralWeights());
  return modelCount;
}

MonolithicCounter::MonolithicCounter(VarOrderingHeuristic addVarOrderingHeuristic, bool inverseAddVarOrdering) {
  this->addVarOrderingHeuristic = addVarOrderingHeuristic;
  this->inverseAddVarOrdering = inverseAddVarOrdering;
}

/* class FactoredCounter ******************************************************/

/* class LinearCounter ******************************************************/

void LinearCounter::setLinearClauseAdds(VectorT<ADD> &clauseAdds, const Formula &formula) {
  clauseAdds.clear();
  clauseAdds.push_back(mgr.addOne());
  for (const VectorT<int_t> &clause : formula.getCnf()) {
    ADD clauseAdd = getClauseAdd(clause);
    clauseAdds.push_back(clauseAdd);
  }
}

double LinearCounter::count(const Formula &formula) {
  orderAddVars(formula);

  VectorT<ADD> factorAdds;
  setLinearClauseAdds(factorAdds, formula);
  SetT<int_t> projectedFormulaVars;
  while (factorAdds.size() > 1) {
    ADD factor1, factor2;
    util::popBack(factor1, factorAdds);
    util::popBack(factor2, factorAdds);

    ADD product = factor1 * factor2;
    SetT<int_t> productAddVars = util::getSupport(product);

    SetT<int_t> otherAddVars = util::getSupportSuperset(factorAdds);

    SetT<int_t> projectingAddVars;
    util::differ(projectingAddVars, productAddVars, otherAddVars);
    abstractCube(product, projectingAddVars, formula.getLiteralWeights());
    util::unionize(projectedFormulaVars, getFormulaVars(projectingAddVars));

    factorAdds.push_back(product);
  }

  double modelCount = dd::countConstAddFloat(util::getSoleMember(factorAdds));
  modelCount = util::adjustModelCount(modelCount, projectedFormulaVars, formula.getLiteralWeights());
  return modelCount;
}

LinearCounter::LinearCounter(VarOrderingHeuristic addVarOrderingHeuristic, bool inverseAddVarOrdering) {
  this->addVarOrderingHeuristic = addVarOrderingHeuristic;
  this->inverseAddVarOrdering = inverseAddVarOrdering;
}

/* class NonlinearCounter ********************************************************/

SetT<int_t> NonlinearCounter::getProjectingAddVars(int_t clusterIndex, bool minRank, const VectorT<int_t> &formulaVarOrdering, const VectorT<VectorT<int_t>> &cnf) {
  SetT<int_t> projectingFormulaVars;

  if (minRank) { /* bucket elimination */
    projectingFormulaVars.insert(formulaVarOrdering.at(clusterIndex));
  }
  else { /* Bouquet's Method */
    SetT<int_t> activeFormulaVars = util::getClusterFormulaVars(clusters.at(clusterIndex), cnf);

    SetT<int_t> otherFormulaVars;
    for (int_t i = clusterIndex + 1; i < clusters.size(); i++)
      util::unionize(otherFormulaVars, util::getClusterFormulaVars(clusters.at(i), cnf));

    util::differ(projectingFormulaVars, activeFormulaVars, otherFormulaVars);
  }

  SetT<int_t> projectingAddVars;
  for (int_t formulaVar : projectingFormulaVars) {
    projectingAddVars.insert(formulaVarToAddVarMap.at(formulaVar));
  }
  return projectingAddVars;
}

void NonlinearCounter::printClusters(const VectorT<VectorT<int_t>> &cnf) const {
  std::cout << "clusters: {\n";
  for (int_t clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    std::cout << "\tcluster of rank " << clusterIndex << ":\n";
    for (int_t clauseIndex : clusters.at(clusterIndex)) {
      std::cout << "\t\tclause";
      util::printClause(cnf.at(clauseIndex));
    }
  }
  std::cout << "} (end of clusters)\n";
}

void NonlinearCounter::fillClusters(const VectorT<VectorT<int_t>> &cnf, const VectorT<int_t> &formulaVarOrdering, bool minRank) {
  clusters = VectorT<VectorT<int_t>>(formulaVarOrdering.size(), VectorT<int_t>());
  for (int_t clauseIndex = 0; clauseIndex < cnf.size(); clauseIndex++) {
    int_t clusterIndex = minRank ? util::getMinClauseRank(cnf.at(clauseIndex), formulaVarOrdering) : util::getMaxClauseRank(cnf.at(clauseIndex), formulaVarOrdering);
    clusters.at(clusterIndex).push_back(clauseIndex);
  }
}

void NonlinearCounter::fillAddClusters(const VectorT<VectorT<int_t>> &cnf, const VectorT<int_t> &formulaVarOrdering, bool minRank) {
  fillClusters(cnf, formulaVarOrdering, minRank);
  if (VERBOSITY >= 1) printClusters(cnf);

  addClusters = VectorT<VectorT<ADD>>(formulaVarOrdering.size(), VectorT<ADD>());
  for (int_t clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    for (int_t clauseIndex : clusters.at(clusterIndex)) {
      ADD clauseAdd = getClauseAdd(cnf.at(clauseIndex));
      addClusters.at(clusterIndex).push_back(clauseAdd);
    }
  }
}

void NonlinearCounter::fillProjectingAddVarSets(const VectorT<VectorT<int_t>> &cnf, const VectorT<int_t> &formulaVarOrdering, bool minRank) {
  fillAddClusters(cnf, formulaVarOrdering, minRank);

  projectingAddVarSets = VectorT<SetT<int_t>>(formulaVarOrdering.size(), SetT<int_t>());
  for (int_t clusterIndex = 0; clusterIndex < addClusters.size(); clusterIndex++) {
    projectingAddVarSets[clusterIndex] = getProjectingAddVars(clusterIndex, minRank, formulaVarOrdering, cnf);
  }
}

int_t NonlinearCounter::getNewClusterIndex(const ADD &abstractedClusterAdd, const VectorT<int_t> &formulaVarOrdering, bool minRank) {
  if (minRank) {
    return util::getMinDdRank(abstractedClusterAdd, addVarToFormulaVarMap, formulaVarOrdering);
  }
  else {
    const SetT<int_t> &remainingAddVars = util::getSupport(abstractedClusterAdd);
    for (int_t clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
      if (!util::isDisjoint(projectingAddVarSets.at(clusterIndex), remainingAddVars))
        return clusterIndex;
    }
    return DUMMY_MAX_INT;
  }
}
int_t NonlinearCounter::getNewClusterIndex(const SetT<int_t> &remainingAddVars) {  /* #MAVC */
  for (int_t clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    if (!util::isDisjoint(projectingAddVarSets.at(clusterIndex), remainingAddVars))
      return clusterIndex;
  }
  return DUMMY_MAX_INT;
}

double NonlinearCounter::countWithList(const Formula &formula, bool minRank) {
  /* checks whether CNF has empty clause: */
  for (const auto &clause : formula.getCnf()) if (clause.empty()) return 0;

  orderAddVars(formula);

  VectorT<int_t> formulaVarOrdering = formula.getVarOrdering(formulaVarOrderingHeuristic, inverseFormulaVarOrdering);
  const VectorT<VectorT<int_t>> &cnf = formula.getCnf();

  fillClusters(cnf, formulaVarOrdering, minRank);
  if (VERBOSITY >= 1) printClusters(cnf);

  /* builds ADD for CNF: */
  ADD cnfAdd = mgr.addOne();
  SetT<int_t> projectedFormulaVars;
  for (int_t clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    /* builds ADD for cluster: */
    ADD clusterAdd = mgr.addOne();
    const VectorT<int_t> &clauseIndices = clusters.at(clusterIndex);
    for (int_t clauseIndex : clauseIndices) {
      ADD clauseAdd = getClauseAdd(cnf.at(clauseIndex));
      clusterAdd *= clauseAdd;
    }

    cnfAdd *= clusterAdd;

    SetT<int_t> projectingAddVars = getProjectingAddVars(clusterIndex, minRank, formulaVarOrdering, cnf);
    abstractCube(cnfAdd, projectingAddVars, formula.getLiteralWeights());
    util::unionize(projectedFormulaVars, getFormulaVars(projectingAddVars));
  }

  double modelCount = dd::countConstAddFloat(cnfAdd);
  modelCount = util::adjustModelCount(modelCount, formulaVarOrdering, formula.getLiteralWeights());
  return modelCount;
}

double NonlinearCounter::countWithTree(const Formula &formula, bool minRank) {
  /* checks whether CNF has empty clause: */
  for (const auto &clause : formula.getCnf()) if (clause.empty()) return 0;

  orderAddVars(formula);

  VectorT<int_t> formulaVarOrdering = formula.getVarOrdering(formulaVarOrderingHeuristic, inverseFormulaVarOrdering);
  const VectorT<VectorT<int_t>> &cnf = formula.getCnf();

  fillProjectingAddVarSets(cnf, formulaVarOrdering, minRank);

  /* builds ADD for CNF: */
  ADD cnfAdd = mgr.addOne();
  SetT<int_t> projectedFormulaVars;
  int_t clusterCount = clusters.size();
  for (int_t clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
    const VectorT<ADD> &addCluster = addClusters.at(clusterIndex);
    if (!addCluster.empty()) {
      /* builds ADD for cluster: */
      ADD clusterAdd = mgr.addOne();
      for (const ADD &add : addCluster) clusterAdd *= add;

      SetT<int_t> projectingAddVars = projectingAddVarSets.at(clusterIndex);
      if (minRank && projectingAddVars.size() != 1) util::showError("wrong number of projecting vars (bucket elimination)");

      abstractCube(clusterAdd, projectingAddVars, formula.getLiteralWeights());
      util::unionize(projectedFormulaVars, getFormulaVars(projectingAddVars));

      int_t newClusterIndex = getNewClusterIndex(clusterAdd, formulaVarOrdering, minRank);

      if (newClusterIndex <= clusterIndex) {
        util::showError("newClusterIndex == " + std::to_string(newClusterIndex) + " <= clusterIndex == " + std::to_string(clusterIndex));
      }
      else if (newClusterIndex < clusterCount) { /* some var remains */
        addClusters.at(newClusterIndex).push_back(clusterAdd);
      }
      else if (newClusterIndex < DUMMY_MAX_INT) {
        util::showError("clusterCount <= newClusterIndex < DUMMY_MAX_INT");
      }
      else { /* no var remains */
        cnfAdd *= clusterAdd;
      }
    }
  }

  double modelCount = dd::countConstAddFloat(cnfAdd);
  modelCount = util::adjustModelCount(modelCount, projectedFormulaVars, formula.getLiteralWeights());
  return modelCount;
}
double NonlinearCounter::countWithTree(const Formula &formula) {  /* #MAVC */
  /* checks whether CNF has empty clause: */
  for (const auto &clause : formula.getCnf()) {
    if (clause.empty()) {
      dd::printMaxAddVarCount(0);
      util::showWarning("DUMMY_MODEL_COUNT");
      return DUMMY_MODEL_COUNT;
    }
  }

  size_t maxAddVarCount = 0;

  orderAddVars(formula);

  VectorT<int_t> formulaVarOrdering = formula.getVarOrdering(formulaVarOrderingHeuristic, inverseFormulaVarOrdering);
  const VectorT<VectorT<int_t>> &cnf = formula.getCnf();

  bool minRank = false;

  fillProjectingAddVarSets(cnf, formulaVarOrdering, minRank);

  VectorT<SetT<int_t>> clustersAddVars; /* clusterIndex |-> addVars */
  for (const auto &addCluster : addClusters) {
    clustersAddVars.push_back(util::getSupportSuperset(addCluster));
  }

  SetT <int_t> cnfAddVars;
  int_t clusterCount = clusters.size();
  for (int_t clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
    const VectorT<ADD> &addCluster = addClusters.at(clusterIndex);
    if (!addCluster.empty()) {
      SetT<int_t> clusterAddVars = clustersAddVars.at(clusterIndex);

      maxAddVarCount = std::max(maxAddVarCount, clusterAddVars.size());

      SetT<int_t> projectingAddVars = projectingAddVarSets.at(clusterIndex);

      SetT<int_t> remainingAddVars;
      util::differ(remainingAddVars, clusterAddVars, projectingAddVars);

      int_t newClusterIndex = getNewClusterIndex(remainingAddVars);

      if (newClusterIndex <= clusterIndex) {
        util::showError("newClusterIndex == " + std::to_string(newClusterIndex) + " <= clusterIndex == " + std::to_string(clusterIndex));
      }
      else if (newClusterIndex < clusterCount) { /* some var remains */
        util::unionize(clustersAddVars.at(newClusterIndex), remainingAddVars);
      }
      else if (newClusterIndex < DUMMY_MAX_INT) {
        util::showError("clusterCount <= newClusterIndex < DUMMY_MAX_INT");
      }
      else { /* no var remains */
        util::unionize(cnfAddVars, remainingAddVars);
        maxAddVarCount = std::max(maxAddVarCount, cnfAddVars.size());
      }
    }
  }

  dd::printMaxAddVarCount(maxAddVarCount);
  util::showWarning("DUMMY_MODEL_COUNT");
  return DUMMY_MODEL_COUNT;
}

/* class BucketCounter ********************************************************/

double BucketCounter::count(const Formula &formula) {
  bool minRank = true;
  return clusterTree ? NonlinearCounter::countWithTree(formula, minRank) : NonlinearCounter::countWithList(formula, minRank);
}

BucketCounter::BucketCounter(bool clusterTree, VarOrderingHeuristic formulaVarOrderingHeuristic, bool inverseFormulaVarOrdering, VarOrderingHeuristic addVarOrderingHeuristic, bool inverseAddVarOrdering) {
  this->clusterTree = clusterTree;
  this->formulaVarOrderingHeuristic = formulaVarOrderingHeuristic;
  this->inverseFormulaVarOrdering = inverseFormulaVarOrdering;
  this->addVarOrderingHeuristic = addVarOrderingHeuristic;
  this->inverseAddVarOrdering = inverseAddVarOrdering;
}

/* class BouquetCounter *******************************************************/

double BouquetCounter::count(const Formula &formula) {
  bool minRank = false;
  return clusterTree ? NonlinearCounter::countWithTree(formula, minRank) : NonlinearCounter::countWithList(formula, minRank);
  // return NonlinearCounter::countWithTree(formula); /* #MAVC */
}

BouquetCounter::BouquetCounter(bool clusterTree, VarOrderingHeuristic formulaVarOrderingHeuristic, bool inverseFormulaVarOrdering, VarOrderingHeuristic addVarOrderingHeuristic, bool inverseAddVarOrdering) {
  this->clusterTree = clusterTree;
  this->formulaVarOrderingHeuristic = formulaVarOrderingHeuristic;
  this->inverseFormulaVarOrdering = inverseFormulaVarOrdering;
  this->addVarOrderingHeuristic = addVarOrderingHeuristic;
  this->inverseAddVarOrdering = inverseAddVarOrdering;
}

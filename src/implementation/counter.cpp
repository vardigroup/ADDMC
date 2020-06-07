/* inclusions *****************************************************************/

#include "../interface/counter.hpp"

/* namespaces *****************************************************************/

/* namespace dd ***************************************************************/

Float diagram::getTerminalValue(const ADD &terminal) {
  DdNode *node = terminal.getNode();
  return (node -> type).value;
}

Float diagram::countConstDdFloat(const ADD &dd) {
  ADD minTerminal = dd.FindMin();
  ADD maxTerminal = dd.FindMax();

  Float minValue = getTerminalValue(minTerminal);
  Float maxValue = getTerminalValue(maxTerminal);

  if (minValue != maxValue) {
    showError("ADD is nonconst: min value " + to_string(minValue) +
      ", max value " + to_string(maxValue));
  }

  return minValue;
}

Int diagram::countConstDdInt(const ADD &dd) {
  Float value = countConstDdFloat(dd);

  if (!util::isInt(value)) showError("unweighted model count is not int");

  return value;
}

void diagram::printMaxDdVarCount(Int maxDdVarCount) {
  util::printRow("maxAddVarCount", maxDdVarCount);
}

/* classes ********************************************************************/

/* class Counter **************************************************************/

WeightFormat Counter::weightFormat;

void Counter::handleSignals(int signal) {
  cout << "\n";
  util::printDuration(startTime);
  cout << "\n";

  util::printSolutionLine(weightFormat, 0, 0, 0);
  showError("received system signal " + to_string(signal) + "; printed dummy model count");
}

void Counter::writeDotFile(ADD &dd, const string &dotFileDir) {
  writeDd(mgr, dd, dotFileDir + "dd" + to_string(dotFileIndex) + ".dot");
  dotFileIndex++;
}

const vector<Int> &Counter::getDdVarOrdering() const {
  return ddVarToCnfVarMap;
}

void Counter::orderDdVars(const Cnf &cnf) {
  ddVarToCnfVarMap = cnf.getVarOrdering(ddVarOrderingHeuristic, inverseDdVarOrdering);
  for (Int ddVar = 0; ddVar < ddVarToCnfVarMap.size(); ddVar++) {
    Int cnfVar = ddVarToCnfVarMap.at(ddVar);
    cnfVarToDdVarMap[cnfVar] = ddVar;
    mgr.addVar(ddVar); // creates ddVar-th ADD var
  }
}

ADD Counter::getClauseDd(const vector<Int> &clause) const {
  ADD clauseDd = mgr.addZero();
  for (Int literal : clause) {
    Int ddVar = cnfVarToDdVarMap.at(util::getCnfVar(literal));
    ADD literalDd = mgr.addVar(ddVar);
    if (!util::isPositiveLiteral(literal)) literalDd = ~literalDd;
    clauseDd |= literalDd;
  }
  return clauseDd;
}

void Counter::abstract(ADD &dd, Int ddVar, const Map<Int, Float> &literalWeights) {
  Int cnfVar = ddVarToCnfVarMap.at(ddVar);
  ADD positiveWeight = mgr.constant(literalWeights.at(cnfVar));
  ADD negativeWeight = mgr.constant(literalWeights.at(-cnfVar));

  dd = positiveWeight * dd.Compose(mgr.addOne(), ddVar) + negativeWeight * dd.Compose(mgr.addZero(), ddVar);
}

void Counter::abstractCube(ADD &dd, const Set<Int> &ddVars, const Map<Int, Float> &literalWeights) {
  for (Int ddVar :ddVars) {
    abstract(dd, ddVar, literalWeights);
  }
}

void Counter::printJoinTree(const Cnf &cnf) const {
  cout << PROBLEM_WORD << " " << JT_WORD << " " << cnf.getDeclaredVarCount() << " " << joinRoot->getTerminalCount() << " " << joinRoot->getNodeCount() << "\n";
  joinRoot->printSubtree();
}

void Counter::setJoinTree(const Cnf &cnf) {
  if (cnf.getClauses().empty()) { // empty cnf
    // showWarning("cnf is empty"); // different warning for empty clause
    joinRoot = new JoinNonterminal(vector<JoinNode *>());
    return;
  }

  Int i = cnf.getEmptyClauseIndex();
  if (i != DUMMY_MIN_INT) { // empty clause found
    showWarning("clause " + to_string(i + 1) + " of cnf is empty (1-indexing); generating dummy join tree");
    joinRoot = new JoinNonterminal(vector<JoinNode *>());
  }
  else {
    constructJoinTree(cnf);
  }
}

ADD Counter::countSubtree(JoinNode *joinNode, const Cnf &cnf, Set<Int> &projectedCnfVars) {
  if (joinNode->isTerminal()) {
    return getClauseDd(cnf.getClauses().at(joinNode->getNodeIndex()));
  }
  else {
    ADD dd = mgr.addOne();
    for (JoinNode *child : joinNode->getChildren()) {
      dd *= countSubtree(child, cnf, projectedCnfVars);
    }
    for (Int cnfVar : joinNode->getProjectableCnfVars()) {
      projectedCnfVars.insert(cnfVar);

      Int ddVar = cnfVarToDdVarMap.at(cnfVar);
      abstract(dd, ddVar, cnf.getLiteralWeights());
    }
    return dd;
  }
}

Float Counter::countJoinTree(const Cnf &cnf) {
  Int i = cnf.getEmptyClauseIndex();
  if (i != DUMMY_MIN_INT) { // empty clause found
    showWarning("clause " + to_string(i + 1) + " of cnf is empty (1-indexing)");
    return 0;
  }
  else {
    orderDdVars(cnf);

    Set<Int> projectedCnfVars;
    ADD dd = countSubtree(static_cast<JoinNode *>(joinRoot), cnf, projectedCnfVars);

    Float modelCount = diagram::countConstDdFloat(dd);
    modelCount = util::adjustModelCount(modelCount, projectedCnfVars, cnf.getLiteralWeights());
    return modelCount;
  }
}

Float Counter::getModelCount(const Cnf &cnf) {
  Int i = cnf.getEmptyClauseIndex();
  if (i != DUMMY_MIN_INT) { // empty clause found
    showWarning("clause " + to_string(i + 1) + " of cnf is empty (1-indexing)");
    return 0;
  }
  else {
    return computeModelCount(cnf);
  }
}

void Counter::output(const string &filePath, WeightFormat weightFormat, OutputFormat outputFormat) {
  Counter::weightFormat = weightFormat;

  Cnf cnf(filePath, weightFormat);

  printComment("Computing output...", 1);

  signal(SIGINT, handleSignals); // Ctrl c
  signal(SIGTERM, handleSignals); // timeout

  switch (outputFormat) {
    case OutputFormat::JOIN_TREE: {
      setJoinTree(cnf);
      printThinLine();
      printJoinTree(cnf);
      printThinLine();
      break;
    }
    case OutputFormat::MODEL_COUNT: {
      util::printSolutionLine(weightFormat, getModelCount(cnf));
      break;
    }
    default: {
      showError("no such outputFormat");
    }
  }
}

/* class JoinTreeCounter ******************************************************/

void JoinTreeCounter::constructJoinTree(const Cnf &cnf) {}

Float JoinTreeCounter::computeModelCount(const Cnf &cnf) {
  bool testing = false;
  // testing = true;
  if (testing) {
    printJoinTree(cnf);
  }

  return countJoinTree(cnf);
}

JoinTreeCounter::JoinTreeCounter(const string &jtFilePath, Float jtWaitSeconds, VarOrderingHeuristic ddVarOrderingHeuristic, bool inverseDdVarOrdering) {
  this->ddVarOrderingHeuristic = ddVarOrderingHeuristic;
  this->inverseDdVarOrdering = inverseDdVarOrdering;

  JoinTreeReader joinTreeReader(jtFilePath, jtWaitSeconds);
  joinRoot = joinTreeReader.getJoinTreeRoot();
}

/* class MonolithicCounter ****************************************************/

void MonolithicCounter::setMonolithicClauseDds(vector<ADD> &clauseDds, const Cnf &cnf) {
  clauseDds.clear();
  for (const vector<Int> &clause : cnf.getClauses()) {
    ADD clauseDd = getClauseDd(clause);
    clauseDds.push_back(clauseDd);
  }
}

void MonolithicCounter::setCnfDd(ADD &cnfDd, const Cnf &cnf) {
  vector<ADD> clauseDds;
  setMonolithicClauseDds(clauseDds, cnf);
  cnfDd = mgr.addOne();
  for (const ADD &clauseDd : clauseDds) {
    cnfDd &= clauseDd; // operator& is operator* in class ADD
  }
}

void MonolithicCounter::constructJoinTree(const Cnf &cnf) {
  vector<JoinNode *> terminals;
  for (Int clauseIndex = 0; clauseIndex < cnf.getClauses().size(); clauseIndex++) {
    terminals.push_back(new JoinTerminal());
  }

  vector<Int> projectableCnfVars = cnf.getApparentVars();

  joinRoot = new JoinNonterminal(terminals, Set<Int>(projectableCnfVars.begin(), projectableCnfVars.end()));
}

Float MonolithicCounter::computeModelCount(const Cnf &cnf) {
  orderDdVars(cnf);

  ADD cnfDd;
  setCnfDd(cnfDd, cnf);

  Set<Int> support = util::getSupport(cnfDd);
  for (Int ddVar : support) {
    abstract(cnfDd, ddVar, cnf.getLiteralWeights());
  }

  Float modelCount = diagram::countConstDdFloat(cnfDd);
  modelCount = util::adjustModelCount(modelCount, getCnfVars(support), cnf.getLiteralWeights());
  return modelCount;
}

MonolithicCounter::MonolithicCounter(VarOrderingHeuristic ddVarOrderingHeuristic, bool inverseDdVarOrdering) {
  this->ddVarOrderingHeuristic = ddVarOrderingHeuristic;
  this->inverseDdVarOrdering = inverseDdVarOrdering;
}

/* class FactoredCounter ******************************************************/

/* class LinearCounter ******************************************************/

void LinearCounter::fillProjectableCnfVarSets(const vector<vector<Int>> &clauses) {
  projectableCnfVarSets = vector<Set<Int>>(clauses.size(), Set<Int>());

  Set<Int> placedCnfVars; // cumulates vars placed in projectableCnfVarSets so far
  for (Int clauseIndex = clauses.size() - 1; clauseIndex >= 0; clauseIndex--) {
    Set<Int> clauseCnfVars = util::getClauseCnfVars(clauses.at(clauseIndex));

    Set<Int> placingCnfVars;
    util::differ(placingCnfVars, clauseCnfVars, placedCnfVars);
    projectableCnfVarSets[clauseIndex] = placingCnfVars;
    util::unionize(placedCnfVars, placingCnfVars);
  }
}

void LinearCounter::setLinearClauseDds(vector<ADD> &clauseDds, const Cnf &cnf) {
  clauseDds.clear();
  clauseDds.push_back(mgr.addOne());
  for (const vector<Int> &clause : cnf.getClauses()) {
    ADD clauseDd = getClauseDd(clause);
    clauseDds.push_back(clauseDd);
  }
}

void LinearCounter::constructJoinTree(const Cnf &cnf) {
  const vector<vector<Int>> &clauses = cnf.getClauses();
  fillProjectableCnfVarSets(clauses);

  vector<JoinNode *> clauseNodes;
  for (Int clauseIndex = 0; clauseIndex < clauses.size(); clauseIndex++) {
    clauseNodes.push_back(new JoinTerminal());
  }

  joinRoot = new JoinNonterminal({clauseNodes.at(0)}, projectableCnfVarSets.at(0));

  for (Int clauseIndex = 1; clauseIndex < clauses.size(); clauseIndex++) {
    joinRoot = new JoinNonterminal({joinRoot, clauseNodes.at(clauseIndex)}, projectableCnfVarSets.at(clauseIndex));
  }
}

Float LinearCounter::computeModelCount(const Cnf &cnf) {
  orderDdVars(cnf);

  vector<ADD> factorDds;
  setLinearClauseDds(factorDds, cnf);
  Set<Int> projectedCnfVars;
  while (factorDds.size() > 1) {
    ADD factor1, factor2;
    util::popBack(factor1, factorDds);
    util::popBack(factor2, factorDds);

    ADD product = factor1 * factor2;
    Set<Int> productDdVars = util::getSupport(product);

    Set<Int> otherDdVars = util::getSupportSuperset(factorDds);

    Set<Int> projectingDdVars;
    util::differ(projectingDdVars, productDdVars, otherDdVars);
    abstractCube(product, projectingDdVars, cnf.getLiteralWeights());
    util::unionize(projectedCnfVars, getCnfVars(projectingDdVars));

    factorDds.push_back(product);
  }

  Float modelCount = diagram::countConstDdFloat(util::getSoleMember(factorDds));
  modelCount = util::adjustModelCount(modelCount, projectedCnfVars, cnf.getLiteralWeights());
  return modelCount;
}

LinearCounter::LinearCounter(VarOrderingHeuristic ddVarOrderingHeuristic, bool inverseDdVarOrdering) {
  this->ddVarOrderingHeuristic = ddVarOrderingHeuristic;
  this->inverseDdVarOrdering = inverseDdVarOrdering;
}

/* class NonlinearCounter ********************************************************/

void NonlinearCounter::printClusters(const vector<vector<Int>> &clauses) const {
  printThinLine();
  printComment("clusters {");
  for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    printComment("\t" "cluster " + to_string(clusterIndex + 1) + ":");
    for (Int clauseIndex : clusters.at(clusterIndex)) {
      cout << COMMENT_WORD << "\t\t" "clause " << clauseIndex + 1 << + ":\t";
      util::printClause(clauses.at(clauseIndex));
    }
  }
  printComment("}");
  printThinLine();
}

void NonlinearCounter::fillClusters(const vector<vector<Int>> &clauses, const vector<Int> &cnfVarOrdering, bool usingMinVar) {
  clusters = vector<vector<Int>>(cnfVarOrdering.size(), vector<Int>());
  for (Int clauseIndex = 0; clauseIndex < clauses.size(); clauseIndex++) {
    Int clusterIndex = usingMinVar ? util::getMinClauseRank(clauses.at(clauseIndex), cnfVarOrdering) : util::getMaxClauseRank(clauses.at(clauseIndex), cnfVarOrdering);
    clusters.at(clusterIndex).push_back(clauseIndex);
  }
}

void NonlinearCounter::printOccurrentCnfVarSets() const {
  printThinLine();
  printComment("occurrentCnfVarSets {");
  for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    const Set<Int> &cnfVarSet = occurrentCnfVarSets.at(clusterIndex);
    cout << COMMENT_WORD << "\t" << "cluster " << clusterIndex + 1 << ":";
    for (Int cnfVar : cnfVarSet) {
      cout << " " << cnfVar;
    }
    cout << "\n";
  }
  printComment("}");
  printThinLine();
}

void NonlinearCounter::printProjectableCnfVarSets() const {
  printThinLine();
  printComment("projectableCnfVarSets {");
  for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    const Set<Int> &cnfVarSet = projectableCnfVarSets.at(clusterIndex);
    cout << COMMENT_WORD << "\t" << "cluster " << clusterIndex + 1 << ":";
    for (Int cnfVar : cnfVarSet) {
      cout << " " << cnfVar;
    }
    cout << "\n";
  }
  cout << "}\n";
  printComment("}");
  printThinLine();
}

void NonlinearCounter::fillCnfVarSets(const vector<vector<Int>> &clauses, bool usingMinVar) {
  occurrentCnfVarSets = vector<Set<Int>>(clusters.size(), Set<Int>());
  projectableCnfVarSets = vector<Set<Int>>(clusters.size(), Set<Int>());

  Set<Int> placedCnfVars; // cumulates vars placed in projectableCnfVarSets so far
  for (Int clusterIndex = clusters.size() - 1; clusterIndex >= 0; clusterIndex--) {
    Set<Int> clusterCnfVars = util::getClusterCnfVars(clusters.at(clusterIndex), clauses);

    occurrentCnfVarSets[clusterIndex] = clusterCnfVars;

    Set<Int> placingCnfVars;
    util::differ(placingCnfVars, clusterCnfVars, placedCnfVars);
    projectableCnfVarSets[clusterIndex] = placingCnfVars;
    util::unionize(placedCnfVars, placingCnfVars);
  }
}

Set<Int> NonlinearCounter::getProjectingDdVars(Int clusterIndex, bool usingMinVar, const vector<Int> &cnfVarOrdering, const vector<vector<Int>> &clauses) {
  Set<Int> projectableCnfVars;

  if (usingMinVar) { // bucket elimination
    projectableCnfVars.insert(cnfVarOrdering.at(clusterIndex));
  }
  else { // Bouquet's Method
    Set<Int> activeCnfVars = util::getClusterCnfVars(clusters.at(clusterIndex), clauses);

    Set<Int> otherCnfVars;
    for (Int i = clusterIndex + 1; i < clusters.size(); i++) {
      util::unionize(otherCnfVars, util::getClusterCnfVars(clusters.at(i), clauses));
    }

    util::differ(projectableCnfVars, activeCnfVars, otherCnfVars);
  }

  Set<Int> projectingDdVars;
  for (Int cnfVar : projectableCnfVars) {
    projectingDdVars.insert(cnfVarToDdVarMap.at(cnfVar));
  }
  return projectingDdVars;
}

void NonlinearCounter::fillDdClusters(const vector<vector<Int>> &clauses, const vector<Int> &cnfVarOrdering, bool usingMinVar) {
  fillClusters(clauses, cnfVarOrdering, usingMinVar);
  if (verbosityLevel >= 2) printClusters(clauses);

  ddClusters = vector<vector<ADD>>(clusters.size(), vector<ADD>());
  for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    for (Int clauseIndex : clusters.at(clusterIndex)) {
      ADD clauseDd = getClauseDd(clauses.at(clauseIndex));
      ddClusters.at(clusterIndex).push_back(clauseDd);
    }
  }
}

void NonlinearCounter::fillProjectingDdVarSets(const vector<vector<Int>> &clauses, const vector<Int> &cnfVarOrdering, bool usingMinVar) {
  fillDdClusters(clauses, cnfVarOrdering, usingMinVar);

  projectingDdVarSets = vector<Set<Int>>(clusters.size(), Set<Int>());
  for (Int clusterIndex = 0; clusterIndex < ddClusters.size(); clusterIndex++) {
    projectingDdVarSets[clusterIndex] = getProjectingDdVars(clusterIndex, usingMinVar, cnfVarOrdering, clauses);
  }
}

Int NonlinearCounter::getTargetClusterIndex(Int clusterIndex) const {
  const Set<Int> &remainingCnfVars = occurrentCnfVarSets.at(clusterIndex);
  for (Int i = clusterIndex + 1; i < clusters.size(); i++) {
    if (!util::isDisjoint(occurrentCnfVarSets.at(i), remainingCnfVars)) {
      return i;
    }
  }
  return DUMMY_MAX_INT;
}

Int NonlinearCounter::getNewClusterIndex(const ADD &abstractedClusterDd, const vector<Int> &cnfVarOrdering, bool usingMinVar) const {
  if (usingMinVar) {
    return util::getMinDdRank(abstractedClusterDd, ddVarToCnfVarMap, cnfVarOrdering);
  }
  else {
    const Set<Int> &remainingDdVars = util::getSupport(abstractedClusterDd);
    for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
      if (!util::isDisjoint(projectingDdVarSets.at(clusterIndex), remainingDdVars)) {
        return clusterIndex;
      }
    }
    return DUMMY_MAX_INT;
  }
}
Int NonlinearCounter::getNewClusterIndex(const Set<Int> &remainingDdVars) const { // #MAVC
  for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    if (!util::isDisjoint(projectingDdVarSets.at(clusterIndex), remainingDdVars)) {
      return clusterIndex;
    }
  }
  return DUMMY_MAX_INT;
}

void NonlinearCounter::constructJoinTreeUsingListClustering(const Cnf &cnf, bool usingMinVar) {
  vector<Int> cnfVarOrdering = cnf.getVarOrdering(cnfVarOrderingHeuristic, inverseCnfVarOrdering);
  const vector<vector<Int>> &clauses = cnf.getClauses();

  fillClusters(clauses, cnfVarOrdering, usingMinVar);
  if (verbosityLevel >= 2) printClusters(clauses);

  fillCnfVarSets(clauses, usingMinVar);
  if (verbosityLevel >= 2) {
    printOccurrentCnfVarSets();
    printProjectableCnfVarSets();
  }

  vector<JoinNode *> terminals;
  for (Int clauseIndex = 0; clauseIndex < clauses.size(); clauseIndex++) {
    terminals.push_back(new JoinTerminal());
  }

  /* creates cluster nodes: */
  vector<JoinNonterminal *> clusterNodes(clusters.size(), nullptr); // null node for empty cluster
  for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    const vector<Int> &clauseIndices = clusters.at(clusterIndex);
    if (!clauseIndices.empty()) {
      vector<JoinNode *> children;
      for (Int clauseIndex : clauseIndices) {
        children.push_back(terminals.at(clauseIndex));
      }
      clusterNodes.at(clusterIndex) = new JoinNonterminal(children);
    }
  }

  Int nonNullClusterNodeIndex = 0;
  while (clusterNodes.at(nonNullClusterNodeIndex) == nullptr) {
    nonNullClusterNodeIndex++;
  }
  JoinNonterminal *nonterminal = clusterNodes.at(nonNullClusterNodeIndex);
  nonterminal->addProjectableCnfVars(projectableCnfVarSets.at(nonNullClusterNodeIndex));
  joinRoot = nonterminal;

  for (Int clusterIndex = nonNullClusterNodeIndex + 1; clusterIndex < clusters.size(); clusterIndex++) {
    JoinNonterminal *clusterNode = clusterNodes.at(clusterIndex);
    if (clusterNode != nullptr) {
      joinRoot = new JoinNonterminal({joinRoot, clusterNode}, projectableCnfVarSets.at(clusterIndex));
    }
  }
}

void NonlinearCounter::constructJoinTreeUsingTreeClustering(const Cnf &cnf, bool usingMinVar) {
  vector<Int> cnfVarOrdering = cnf.getVarOrdering(cnfVarOrderingHeuristic, inverseCnfVarOrdering);
  const vector<vector<Int>> &clauses = cnf.getClauses();

  fillClusters(clauses, cnfVarOrdering, usingMinVar);
  if (verbosityLevel >= 2) printClusters(clauses);

  fillCnfVarSets(clauses, usingMinVar);
  if (verbosityLevel >= 2) {
    printOccurrentCnfVarSets();
    printProjectableCnfVarSets();
  }

  vector<JoinNode *> terminals;
  for (Int clauseIndex = 0; clauseIndex < clauses.size(); clauseIndex++) {
    terminals.push_back(new JoinTerminal());
  }

  Int clusterCount = clusters.size();
  joinNodeSets = vector<vector<JoinNode *>>(clusterCount, vector<JoinNode *>()); // clusterIndex -> non-null nodes

  /* creates cluster nodes: */
  for (Int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
    const vector<Int> &clauseIndices = clusters.at(clusterIndex);
    if (!clauseIndices.empty()) {
      vector<JoinNode *> children;
      for (Int clauseIndex : clauseIndices) {
        children.push_back(terminals.at(clauseIndex));
      }
      joinNodeSets.at(clusterIndex).push_back(new JoinNonterminal(children));
    }
  }

  vector<JoinNode *> rootChildren;
  for (Int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
    if (joinNodeSets.at(clusterIndex).empty()) continue;

    Set<Int> projectableCnfVars = projectableCnfVarSets.at(clusterIndex);

    Set<Int> remainingCnfVars;
    util::differ(remainingCnfVars, occurrentCnfVarSets.at(clusterIndex), projectableCnfVars);
    occurrentCnfVarSets[clusterIndex] = remainingCnfVars;

    Int targetClusterIndex = getTargetClusterIndex(clusterIndex);
    if (targetClusterIndex <= clusterIndex) {
      showError("targetClusterIndex == " + to_string(targetClusterIndex) + " <= clusterIndex == " + to_string(clusterIndex));
    }
    else if (targetClusterIndex < clusterCount) { // some var remains
      util::unionize(occurrentCnfVarSets.at(targetClusterIndex), remainingCnfVars);

      JoinNonterminal *nonterminal = new JoinNonterminal(joinNodeSets.at(clusterIndex), projectableCnfVars);
      joinNodeSets.at(targetClusterIndex).push_back(nonterminal);
    }
    else if (targetClusterIndex < DUMMY_MAX_INT) {
      showError("clusterCount <= targetClusterIndex < DUMMY_MAX_INT");
    }
    else { // no var remains
      JoinNonterminal *nonterminal = new JoinNonterminal(joinNodeSets.at(clusterIndex), projectableCnfVars);
      rootChildren.push_back(nonterminal);
    }
  }
  joinRoot = new JoinNonterminal(rootChildren);
}

Float NonlinearCounter::countUsingListClustering(const Cnf &cnf, bool usingMinVar) {
  orderDdVars(cnf);

  vector<Int> cnfVarOrdering = cnf.getVarOrdering(cnfVarOrderingHeuristic, inverseCnfVarOrdering);
  const vector<vector<Int>> &clauses = cnf.getClauses();

  fillClusters(clauses, cnfVarOrdering, usingMinVar);
  if (verbosityLevel >= 2) printClusters(clauses);

  /* builds ADD for CNF: */
  ADD cnfDd = mgr.addOne();
  Set<Int> projectedCnfVars;
  for (Int clusterIndex = 0; clusterIndex < clusters.size(); clusterIndex++) {
    /* builds ADD for cluster: */
    ADD clusterDd = mgr.addOne();
    const vector<Int> &clauseIndices = clusters.at(clusterIndex);
    for (Int clauseIndex : clauseIndices) {
      ADD clauseDd = getClauseDd(clauses.at(clauseIndex));
      clusterDd *= clauseDd;
    }

    cnfDd *= clusterDd;

    Set<Int> projectingDdVars = getProjectingDdVars(clusterIndex, usingMinVar, cnfVarOrdering, clauses);
    abstractCube(cnfDd, projectingDdVars, cnf.getLiteralWeights());
    util::unionize(projectedCnfVars, getCnfVars(projectingDdVars));
  }

  Float modelCount = diagram::countConstDdFloat(cnfDd);
  modelCount = util::adjustModelCount(modelCount, cnfVarOrdering, cnf.getLiteralWeights());
  return modelCount;
}

Float NonlinearCounter::countUsingTreeClustering(const Cnf &cnf, bool usingMinVar) {
  orderDdVars(cnf);

  vector<Int> cnfVarOrdering = cnf.getVarOrdering(cnfVarOrderingHeuristic, inverseCnfVarOrdering);
  const vector<vector<Int>> &clauses = cnf.getClauses();

  fillProjectingDdVarSets(clauses, cnfVarOrdering, usingMinVar);

  /* builds ADD for CNF: */
  ADD cnfDd = mgr.addOne();
  Set<Int> projectedCnfVars;
  Int clusterCount = clusters.size();
  for (Int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
    const vector<ADD> &ddCluster = ddClusters.at(clusterIndex);
    if (!ddCluster.empty()) {
      /* builds ADD for cluster: */
      ADD clusterDd = mgr.addOne();
      for (const ADD &dd : ddCluster) clusterDd *= dd;

      Set<Int> projectingDdVars = projectingDdVarSets.at(clusterIndex);
      if (usingMinVar && projectingDdVars.size() != 1) showError("wrong number of projecting vars (bucket elimination)");

      abstractCube(clusterDd, projectingDdVars, cnf.getLiteralWeights());
      util::unionize(projectedCnfVars, getCnfVars(projectingDdVars));

      Int newClusterIndex = getNewClusterIndex(clusterDd, cnfVarOrdering, usingMinVar);

      if (newClusterIndex <= clusterIndex) {
        showError("newClusterIndex == " + to_string(newClusterIndex) + " <= clusterIndex == " + to_string(clusterIndex));
      }
      else if (newClusterIndex < clusterCount) { // some var remains
        ddClusters.at(newClusterIndex).push_back(clusterDd);
      }
      else if (newClusterIndex < DUMMY_MAX_INT) {
        showError("clusterCount <= newClusterIndex < DUMMY_MAX_INT");
      }
      else { // no var remains
        cnfDd *= clusterDd;
      }
    }
  }

  Float modelCount = diagram::countConstDdFloat(cnfDd);
  modelCount = util::adjustModelCount(modelCount, projectedCnfVars, cnf.getLiteralWeights());
  return modelCount;
}
Float NonlinearCounter::countUsingTreeClustering(const Cnf &cnf) { // #MAVC
  orderDdVars(cnf);

  vector<Int> cnfVarOrdering = cnf.getVarOrdering(cnfVarOrderingHeuristic, inverseCnfVarOrdering);
  const vector<vector<Int>> &clauses = cnf.getClauses();

  bool usingMinVar = false;

  fillProjectingDdVarSets(clauses, cnfVarOrdering, usingMinVar);

  vector<Set<Int>> clustersDdVars; // clusterIndex |-> ddVars
  for (const auto &ddCluster : ddClusters) {
    clustersDdVars.push_back(util::getSupportSuperset(ddCluster));
  }

  Set<Int> cnfDdVars;
  size_t maxDdVarCount = 0;
  Int clusterCount = clusters.size();
  for (Int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
    const vector<ADD> &ddCluster = ddClusters.at(clusterIndex);
    if (!ddCluster.empty()) {
      Set<Int> clusterDdVars = clustersDdVars.at(clusterIndex);

      maxDdVarCount = std::max(maxDdVarCount, clusterDdVars.size());

      Set<Int> projectingDdVars = projectingDdVarSets.at(clusterIndex);

      Set<Int> remainingDdVars;
      util::differ(remainingDdVars, clusterDdVars, projectingDdVars);

      Int newClusterIndex = getNewClusterIndex(remainingDdVars);

      if (newClusterIndex <= clusterIndex) {
        showError("newClusterIndex == " + to_string(newClusterIndex) + " <= clusterIndex == " + to_string(clusterIndex));
      }
      else if (newClusterIndex < clusterCount) { // some var remains
        util::unionize(clustersDdVars.at(newClusterIndex), remainingDdVars);
      }
      else if (newClusterIndex < DUMMY_MAX_INT) {
        showError("clusterCount <= newClusterIndex < DUMMY_MAX_INT");
      }
      else { // no var remains
        util::unionize(cnfDdVars, remainingDdVars);
        maxDdVarCount = std::max(maxDdVarCount, cnfDdVars.size());
      }
    }
  }

  diagram::printMaxDdVarCount(maxDdVarCount);
  showWarning("NEGATIVE_INFINITY");
  return NEGATIVE_INFINITY;
}

/* class BucketCounter ********************************************************/

void BucketCounter::constructJoinTree(const Cnf &cnf) {
  bool usingMinVar = true;
  return usingTreeClustering ? NonlinearCounter::constructJoinTreeUsingTreeClustering(cnf, usingMinVar) : NonlinearCounter::constructJoinTreeUsingListClustering(cnf, usingMinVar);
}

Float BucketCounter::computeModelCount(const Cnf &cnf) {
  bool usingMinVar = true;
  return usingTreeClustering ? NonlinearCounter::countUsingTreeClustering(cnf, usingMinVar) : NonlinearCounter::countUsingListClustering(cnf, usingMinVar);
}

BucketCounter::BucketCounter(bool usingTreeClustering, VarOrderingHeuristic cnfVarOrderingHeuristic, bool inverseCnfVarOrdering, VarOrderingHeuristic ddVarOrderingHeuristic, bool inverseDdVarOrdering) {
  this->usingTreeClustering = usingTreeClustering;
  this->cnfVarOrderingHeuristic = cnfVarOrderingHeuristic;
  this->inverseCnfVarOrdering = inverseCnfVarOrdering;
  this->ddVarOrderingHeuristic = ddVarOrderingHeuristic;
  this->inverseDdVarOrdering = inverseDdVarOrdering;
}

/* class BouquetCounter *******************************************************/

void BouquetCounter::constructJoinTree(const Cnf &cnf) {
  bool usingMinVar = false;
  return usingTreeClustering ? NonlinearCounter::constructJoinTreeUsingTreeClustering(cnf, usingMinVar) : NonlinearCounter::constructJoinTreeUsingListClustering(cnf, usingMinVar);
}

Float BouquetCounter::computeModelCount(const Cnf &cnf) {
  bool usingMinVar = false;
  // return NonlinearCounter::countUsingTreeClustering(cnf); // #MAVC
  return usingTreeClustering ? NonlinearCounter::countUsingTreeClustering(cnf, usingMinVar) : NonlinearCounter::countUsingListClustering(cnf, usingMinVar);
}

BouquetCounter::BouquetCounter(bool usingTreeClustering, VarOrderingHeuristic cnfVarOrderingHeuristic, bool inverseCnfVarOrdering, VarOrderingHeuristic ddVarOrderingHeuristic, bool inverseDdVarOrdering) {
  this->usingTreeClustering = usingTreeClustering;
  this->cnfVarOrderingHeuristic = cnfVarOrderingHeuristic;
  this->inverseCnfVarOrdering = inverseCnfVarOrdering;
  this->ddVarOrderingHeuristic = ddVarOrderingHeuristic;
  this->inverseDdVarOrdering = inverseDdVarOrdering;
}

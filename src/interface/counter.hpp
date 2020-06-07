#pragma once

/* inclusions *****************************************************************/

#include "formula.hpp"
#include "join.hpp"
#include "visual.hpp"

/* namespaces *****************************************************************/

namespace diagram {
  Float getTerminalValue(const ADD &terminal);
  Float countConstDdFloat(const ADD &dd);
  Int countConstDdInt(const ADD &dd);
  void printMaxDdVarCount(Int maxDdVarCount);
}

/* classes ********************************************************************/

class Counter { // abstract
protected:
  static WeightFormat weightFormat;

  Int dotFileIndex = 1;
  Cudd mgr;
  VarOrderingHeuristic ddVarOrderingHeuristic;
  bool inverseDdVarOrdering;
  Map<Int, Int> cnfVarToDdVarMap; // e.g. {42: 0, 13: 1}
  vector<Int> ddVarToCnfVarMap; // e.g. [42, 13], i.e. ddVarOrdering

  JoinNonterminal *joinRoot;

  static void handleSignals(int signal); // `timeout` sends SIGTERM

  void writeDotFile(ADD &dd, const string &dotFileDir = DOT_DIR);
  template<typename T> Set<Int> getCnfVars(const T &ddVars) {
    Set<Int> cnfVars;
    for (Int ddVar : ddVars) cnfVars.insert(ddVarToCnfVarMap.at(ddVar));
    return cnfVars;
  }
  const vector<Int> &getDdVarOrdering() const; // ddVarToCnfVarMap
  void orderDdVars(const Cnf &cnf); // writes: cnfVarToDdVarMap, ddVarToCnfVarMap
  ADD getClauseDd(const vector<Int> &clause) const;
  void abstract(ADD &dd, Int ddVar, const Map<Int, Float> &literalWeights);
  void abstractCube(ADD &dd, const Set<Int> &ddVars, const Map<Int, Float> &literalWeights);

  void printJoinTree(const Cnf &cnf) const;

public:
  virtual void constructJoinTree(const Cnf &cnf) = 0; // handles cnf without empty clause
  void setJoinTree(const Cnf &cnf); // handles cnf with/without empty clause

  ADD countSubtree(JoinNode *joinNode, const Cnf &cnf, Set<Int> &projectedCnfVars); // handles cnf without empty clause
  Float countJoinTree(const Cnf &cnf); // handles cnf with/without empty clause

  virtual Float computeModelCount(const Cnf &cnf) = 0; // handles cnf without empty clause
  Float getModelCount(const Cnf &cnf); // handles cnf with/without empty clause

  void output(const string &filePath, WeightFormat weightFormat, OutputFormat outputFormat);
};

class JoinTreeCounter : public Counter {
public:
  void constructJoinTree(const Cnf &cnf) override;
  Float computeModelCount(const Cnf &cnf) override;
  JoinTreeCounter(
    const string &jtFilePath,
    Float jtWaitSeconds,
    VarOrderingHeuristic ddVarOrderingHeuristic,
    bool inverseDdVarOrdering
  );
};

class MonolithicCounter : public Counter { // builds an ADD for the entire CNF
protected:
  void setMonolithicClauseDds(vector<ADD> &clauseDds, const Cnf &cnf);
  void setCnfDd(ADD &cnfDd, const Cnf &cnf);

public:
  void constructJoinTree(const Cnf &cnf) override;
  Float computeModelCount(const Cnf &cnf) override;
  MonolithicCounter(
    VarOrderingHeuristic ddVarOrderingHeuristic,
    bool inverseDdVarOrdering
  );
};

class FactoredCounter : public Counter {}; // abstract; builds an ADD for each clause

class LinearCounter : public FactoredCounter { // combines adjacent clauses
protected:
  vector<Set<Int>> projectableCnfVarSets; // clauseIndex |-> cnfVars

  void fillProjectableCnfVarSets(const vector<vector<Int>> &clauses);
  void setLinearClauseDds(vector<ADD> &clauseDds, const Cnf &cnf);

public:
  void constructJoinTree(const Cnf &cnf) override;
  Float computeModelCount(const Cnf &cnf) override;
  LinearCounter(
    VarOrderingHeuristic ddVarOrderingHeuristic,
    bool inverseDdVarOrdering
  );
};

class NonlinearCounter : public FactoredCounter { // abstract; puts clauses in clusters
protected:
  bool usingTreeClustering;
  VarOrderingHeuristic cnfVarOrderingHeuristic;
  bool inverseCnfVarOrdering;
  vector<vector<Int>> clusters; // clusterIndex |-> clauseIndices

  vector<Set<Int>> occurrentCnfVarSets; // clusterIndex |-> cnfVars
  vector<Set<Int>> projectableCnfVarSets; // clusterIndex |-> cnfVars
  vector<vector<JoinNode *>> joinNodeSets; // clusterIndex |-> non-null nodes

  vector<vector<ADD>> ddClusters; // clusterIndex |-> ADDs (if usingTreeClustering)
  vector<Set<Int>> projectingDdVarSets; // clusterIndex |-> ddVars (if usingTreeClustering)

  void printClusters(const vector<vector<Int>> &clauses) const;
  void fillClusters(const vector<vector<Int>> &clauses, const vector<Int> &cnfVarOrdering, bool usingMinVar);

  void printOccurrentCnfVarSets() const;
  void printProjectableCnfVarSets() const;
  void fillCnfVarSets(const vector<vector<Int>> &clauses, bool usingMinVar); // writes: occurrentCnfVarSets, projectableCnfVarSets

  Set<Int> getProjectingDdVars(Int clusterIndex, bool usingMinVar, const vector<Int> &cnfVarOrdering, const vector<vector<Int>> &clauses);
  void fillDdClusters(const vector<vector<Int>> &clauses, const vector<Int> &cnfVarOrdering, bool usingMinVar); // (if usingTreeClustering)
  void fillProjectingDdVarSets(const vector<vector<Int>> &clauses, const vector<Int> &cnfVarOrdering, bool usingMinVar); // (if usingTreeClustering)

  Int getTargetClusterIndex(Int clusterIndex) const; // returns DUMMY_MAX_INT if no var remains
  Int getNewClusterIndex(const ADD &abstractedClusterDd, const vector<Int> &cnfVarOrdering, bool usingMinVar) const; // returns DUMMY_MAX_INT if no var remains (if usingTreeClustering)
  Int getNewClusterIndex(const Set<Int> &remainingDdVars) const; // returns DUMMY_MAX_INT if no var remains (if usingTreeClustering) #MAVC

  void constructJoinTreeUsingListClustering(const Cnf &cnf, bool usingMinVar);
  void constructJoinTreeUsingTreeClustering(const Cnf &cnf, bool usingMinVar);

  Float countUsingListClustering(const Cnf &cnf, bool usingMinVar);
  Float countUsingTreeClustering(const Cnf &cnf, bool usingMinVar);
  Float countUsingTreeClustering(const Cnf &cnf); // #MAVC
};

class BucketCounter : public NonlinearCounter { // bucket elimination
public:
  void constructJoinTree(const Cnf &cnf) override;
  Float computeModelCount(const Cnf &cnf) override;
  BucketCounter(
    bool usingTreeClustering,
    VarOrderingHeuristic cnfVarOrderingHeuristic,
    bool inverseCnfVarOrdering,
    VarOrderingHeuristic ddVarOrderingHeuristic,
    bool inverseDdVarOrdering
  );
};

class BouquetCounter : public NonlinearCounter { // Bouquet's Method
public:
  void constructJoinTree(const Cnf &cnf) override;
  Float computeModelCount(const Cnf &cnf) override;
  BouquetCounter(
    bool usingTreeClustering,
    VarOrderingHeuristic cnfVarOrderingHeuristic,
    bool inverseCnfVarOrdering,
    VarOrderingHeuristic ddVarOrderingHeuristic,
    bool inverseDdVarOrdering
  );
};

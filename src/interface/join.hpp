#pragma once

/* inclusions *****************************************************************/

#include <signal.h>

#include "util.hpp"

/* uses ***********************************************************************/

using util::printComment;
using util::printThickLine;
using util::printThinLine;
using util::showError;
using util::showWarning;

/* constants ******************************************************************/

extern const string &JOIN_TREE_END_WORD;
extern const string &JT_WORD;
extern const string &VAR_ELIM_WORD;

/* classes ********************************************************************/

class JoinNode { // abstract
  friend class JoinTreeReader;

protected:
  static Int backupNodeCount;
  static Int backupTerminalCount;
  static Set<Int> backupNonterminalIndexes;

  static Int nodeCount;
  static Int terminalCount;
  static Set<Int> nonterminalIndexes;

  Int nodeIndex = DUMMY_MIN_INT; // 0-indexing

  /* empty for terminals: */
  vector<JoinNode *> children;
  Set<Int> projectableCnfVars;

public:
  static void resetStaticFields();
  static void restoreStaticFields();

  bool isTerminal() const;
  Int getNodeCount() const;
  Int getTerminalCount() const;
  Int getNodeIndex() const;

  const vector<JoinNode *> &getChildren() const;
  const Set<Int> &getProjectableCnfVars() const;
  virtual void printSubtree(const string &prefix = "") const = 0;
};

class JoinTerminal : public JoinNode {
public:
  void printSubtree(const string &prefix = "") const override;
  JoinTerminal();
};

class JoinNonterminal : public JoinNode {
public:
  void printNode(const string &prefix) const;
  void printSubtree(const string &prefix = "") const override; // post-order
  void addProjectableCnfVars(const Set<Int> &cnfVars);
  JoinNonterminal(
    const vector<JoinNode *> &children,
    const Set<Int> &projectableCnfVars = Set<Int>(),
    Int requestedNodeIndex = DUMMY_MIN_INT
  );
};

class JoinTree {
  friend class JoinTreeReader;

protected:
  Int declaredVarCount = DUMMY_MIN_INT; // in jt file
  Int declaredClauseCount = DUMMY_MIN_INT; // in jt file
  Int declaredNodeCount = DUMMY_MIN_INT; // in jt file

  Map<Int, JoinTerminal *> joinTerminals;
  Map<Int, JoinNonterminal *> joinNonterminals;

  Float plannerSeconds = NEGATIVE_INFINITY; // cumulative time for all join trees

  JoinNode *getJoinNode(Int nodeIndex) const;

public:
  JoinNonterminal *getJoinRoot() const;
  void printTree() const;

  JoinTree(Int declaredVarCount, Int declaredClauseCount, Int declaredNodeCount);
};

class JoinTreeReader {
protected:
  static Int plannerPid;

  JoinTree *backupJoinTree = nullptr;
  JoinTree *joinTree = nullptr;

  Int lineIndex = 0;
  Int problemLineIndex = DUMMY_MIN_INT;
  Int joinTreeEndLineIndex = DUMMY_MIN_INT;

  static void handleAlarm(int signal); // `alarm` sends SIGALRM

  void finishReadingJoinTree(); // after reading '=' or end of stream
  void readInputStream(std::istream *inputStream);

public:
  JoinNonterminal *getJoinTreeRoot() const;
  JoinTreeReader(const string &filePath, Float jtWaitSeconds);
};

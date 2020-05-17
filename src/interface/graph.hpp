#pragma once

/* inclusions *****************************************************************/

#include "util.hpp"

/* uses ***********************************************************************/

using util::printComment;
using util::printThickLine;
using util::printThinLine;
using util::showError;
using util::showWarning;

/* classes ********************************************************************/

class Graph { // undirected
protected:
  Set<Int> vertices;
  Map<Int, Set<Int>> adjacencyMap;

public:
  void printVertices() const;
  void printAdjacencyMap() const;
  Graph(const Set<Int> &cnfrVars);
  void addEdge(Int cnfrVar1, Int cnfrVar2);
  Set<Int>::const_iterator beginVertices() const;
  Set<Int>::const_iterator endVertices() const;
  Set<Int>::const_iterator beginNeighbors(Int cnfrVar);
  Set<Int>::const_iterator endNeighbors(Int cnfrVar);
  void removeVertex(Int v); // also removes v's edges
  bool hasPath(Int from, Int to, Set<Int> &visitedVertices) const;
  bool hasPath(Int from, Int to) const;
};

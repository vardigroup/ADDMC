#pragma once

/* includes *******************************************************************/

#include "util.hpp"

/* classes ********************************************************************/

class Graph { /* undirected */
protected:
  SetT<int_t> vertices;
  MapT<int_t, SetT<int_t>> adjacencyMap;

public:
  void printVertices() const;
  void printAdjacencyMap() const;
  Graph(const SetT<int_t> &formularVars);
  void addEdge(int_t formularVar1, int_t formularVar2);
  SetT<int_t>::const_iterator beginVertices() const;
  SetT<int_t>::const_iterator endVertices() const;
  SetT<int_t>::const_iterator beginNeighbors(int_t formularVar);
  SetT<int_t>::const_iterator endNeighbors(int_t formularVar);
  void removeVertex(int_t v); /* also removes v's edges */
  bool hasPath(int_t from, int_t to, SetT<int_t> &visitedVertices) const;
  bool hasPath(int_t from, int_t to) const;
};

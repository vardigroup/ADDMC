/* includes *******************************************************************/

#include "../interface/graph.hpp"

/* classes ********************************************************************/

/* class Graph ****************************************************************/

void Graph::printVertices() const {
  std::cout << "vertices: ";
  for (int_t vertex : vertices) {
    std::cout << vertex << " ";
  }
  std::cout << "\n\n";
}

void Graph::printAdjacencyMap() const {
  std::cout << "adjacency map: {" << "\n";
  for (auto pair = adjacencyMap.begin(); pair != adjacencyMap.end(); pair++) {
    auto vertex = pair->first;
    auto neighbors = pair->second;
    std::cout << "\t" << vertex << " : ";
    for (auto neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++) {
      std::cout << *neighbor << " ";
    }
    std::cout << "\n";
  }
  std::cout << "}\n\n";
}

Graph::Graph(const SetT<int_t> &formulaVars) {
  vertices = formulaVars;
  for (int_t formulaVar : formulaVars) {
    adjacencyMap[formulaVar] = SetT<int_t>();
  }
}

void Graph::addEdge(int_t formulaVar1, int_t formulaVar2) {
  adjacencyMap.at(formulaVar1).insert(formulaVar2);
  adjacencyMap.at(formulaVar2).insert(formulaVar1);
}

SetT<int_t>::const_iterator Graph::beginVertices() const {
  return vertices.begin();
}

SetT<int_t>::const_iterator Graph::endVertices() const {
  return vertices.end();
}

SetT<int_t>::const_iterator Graph::beginNeighbors(int_t formulaVar) {
  return adjacencyMap.at(formulaVar).begin();
}

SetT<int_t>::const_iterator Graph::endNeighbors(int_t formulaVar) {
  return adjacencyMap.at(formulaVar).end();
}

void Graph::removeVertex(int_t v) {
  vertices.erase(v);

  adjacencyMap.erase(v); /* edges from v */

  for (std::pair<const int_t, SetT<int_t>> &vertexAndNeighbors: adjacencyMap)
    vertexAndNeighbors.second.erase(v); /* edges to v */
}

bool Graph::hasPath(int_t from, int_t to, SetT<int_t> &visitedVertices) const {
  if (from == to) return true;

  visitedVertices.insert(from);

  SetT<int_t> unvisitedNeighbors;
  util::differ(unvisitedNeighbors, adjacencyMap.at(from), visitedVertices);

  for (int_t v : unvisitedNeighbors) if (hasPath(v, to, visitedVertices)) return true;

  return false;
}

bool Graph::hasPath(int_t from, int_t to) const {
  SetT<int_t> visitedVertices;
  return hasPath(from, to, visitedVertices);
}

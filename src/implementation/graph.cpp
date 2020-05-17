/* inclusions *****************************************************************/

#include "../interface/graph.hpp"

/* classes ********************************************************************/

/* class Graph ****************************************************************/

void Graph::printVertices() const {
  cout << "vertices: ";
  for (Int vertex : vertices) {
    cout << vertex << " ";
  }
  cout << "\n\n";
}

void Graph::printAdjacencyMap() const {
  cout << "adjacency map {\n";
  for (auto pair = adjacencyMap.begin(); pair != adjacencyMap.end(); pair++) {
    auto vertex = pair->first;
    auto neighbors = pair->second;
    cout << "\t" << vertex << " : ";
    for (auto neighbor = neighbors.begin(); neighbor != neighbors.end(); neighbor++) {
      cout << *neighbor << " ";
    }
    cout << "\n";
  }
  cout << "}\n\n";
}

Graph::Graph(const Set<Int> &cnfVars) {
  vertices = cnfVars;
  for (Int cnfVar : cnfVars) {
    adjacencyMap[cnfVar] = Set<Int>();
  }
}

void Graph::addEdge(Int cnfVar1, Int cnfVar2) {
  adjacencyMap.at(cnfVar1).insert(cnfVar2);
  adjacencyMap.at(cnfVar2).insert(cnfVar1);
}

Set<Int>::const_iterator Graph::beginVertices() const {
  return vertices.begin();
}

Set<Int>::const_iterator Graph::endVertices() const {
  return vertices.end();
}

Set<Int>::const_iterator Graph::beginNeighbors(Int cnfVar) {
  return adjacencyMap.at(cnfVar).begin();
}

Set<Int>::const_iterator Graph::endNeighbors(Int cnfVar) {
  return adjacencyMap.at(cnfVar).end();
}

void Graph::removeVertex(Int v) {
  vertices.erase(v);

  adjacencyMap.erase(v); // edges from v

  for (std::pair<const Int, Set<Int>> &vertexAndNeighbors: adjacencyMap)
    vertexAndNeighbors.second.erase(v); // edges to v
}

bool Graph::hasPath(Int from, Int to, Set<Int> &visitedVertices) const {
  if (from == to) return true;

  visitedVertices.insert(from);

  Set<Int> unvisitedNeighbors;
  util::differ(unvisitedNeighbors, adjacencyMap.at(from), visitedVertices);

  for (Int v : unvisitedNeighbors) if (hasPath(v, to, visitedVertices)) return true;

  return false;
}

bool Graph::hasPath(Int from, Int to) const {
  Set<Int> visitedVertices;
  return hasPath(from, to, visitedVertices);
}

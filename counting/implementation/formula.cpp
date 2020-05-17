/* includes *******************************************************************/

#include "../interface/formula.hpp"

/* classes ********************************************************************/

/* class Label ****************************************************************/

void Label::addNumber(int_t i) {
  push_back(i);
  std::sort(begin(), end(), std::greater<int_t>());
}

/* class Formula **************************************************************/

void Formula::updateApparentVars(int_t literal) {
  int_t var = util::getFormulaVar(literal);
  if (!util::isFound(var, apparentVars)) apparentVars.push_back(var);
}

void Formula::addClause(const VectorT<int_t> &clause) {
  cnf.push_back(clause);
  for (int_t literal : clause) updateApparentVars(literal);
}

Graph Formula::getGaifmanGraph() const {
  SetT<int_t> vars;
  for (int_t var : apparentVars) vars.insert(var);
  Graph graph(vars);

  for (const VectorT<int_t> &clause : cnf)
    for (auto literal1 = clause.begin(); literal1 != clause.end(); literal1++)
      for (auto literal2 = std::next(literal1); literal2 != clause.end(); literal2++) {
        int_t var1 = util::getFormulaVar(*literal1);
        int_t var2 = util::getFormulaVar(*literal2);
        graph.addEdge(var1, var2);
      }

  return graph;
}

VectorT<int_t> Formula::getAppearanceVarOrdering() const {
  return apparentVars;
}

VectorT<int_t> Formula::getDeclarationVarOrdering() const {
  VectorT<int_t> varOrdering = apparentVars;
  std::sort(varOrdering.begin(), varOrdering.end());
  return varOrdering;
}

VectorT<int_t> Formula::getRandomVarOrdering() const {
  VectorT<int_t> varOrdering = apparentVars;
  util::shuffleRandomly(varOrdering);
  return varOrdering;
}

VectorT<int_t> Formula::getLexpVarOrdering() const {
  MapT<int_t, Label> unnumberedVertices;
  for (int_t vertex : apparentVars) unnumberedVertices[vertex] = Label();
  VectorT<int_t> numberedVertices; /* whose \alpha numbers are decreasing */
  Graph graph = getGaifmanGraph();
  for (int_t number = apparentVars.size(); number > 0; number--) {
    auto vertexIt = std::max_element(unnumberedVertices.begin(),
      unnumberedVertices.end(), util::isLessValued<int_t, Label>);
    int_t vertex = vertexIt->first; /* ignores label */
    numberedVertices.push_back(vertex);
    unnumberedVertices.erase(vertex);
    for (auto neighborIt = graph.beginNeighbors(vertex);
        neighborIt != graph.endNeighbors(vertex); neighborIt++) {
      int_t neighbor = *neighborIt;
      auto unnumberedNeighborIt = unnumberedVertices.find(neighbor);
      if (unnumberedNeighborIt != unnumberedVertices.end()) {
        int_t unnumberedNeighbor = unnumberedNeighborIt->first;
        unnumberedVertices.at(unnumberedNeighbor).addNumber(number);
      }
    }
  }
  return numberedVertices;
}

VectorT<int_t> Formula::getLexmVarOrdering() const {
  MapT<int_t, Label> unnumberedVertices;
  for (int_t vertex : apparentVars) unnumberedVertices[vertex] = Label();
  VectorT<int_t> numberedVertices; /* whose \alpha numbers are decreasing */
  Graph graph = getGaifmanGraph();
  for (int_t i = apparentVars.size(); i > 0; i--) {
    auto vIt = std::max_element(unnumberedVertices.begin(),
      unnumberedVertices.end(), util::isLessValued<int_t, Label>);
    int_t v = vIt->first; /* ignores label */
    numberedVertices.push_back(v);
    unnumberedVertices.erase(v);

    /* updates numberedVertices: */
    Graph subgraph = getGaifmanGraph(); /* will only contain v, w, and unnumbered vertices whose labels are less than w's */
    for (auto wIt = unnumberedVertices.begin(); wIt != unnumberedVertices.end(); wIt++) {
      int_t w = wIt->first;
      Label &wLabel = wIt->second;

      /* removes numbered vertices except v: */
      for (int_t numberedVertex : numberedVertices)
        if (numberedVertex != v)
          subgraph.removeVertex(numberedVertex);

      /* removes each non-w unnumbered vertex whose label is not less than w's */
      for (const std::pair<int_t, Label> &kv : unnumberedVertices) {
        int_t unnumberedVertex = kv.first;
        const Label &label = kv.second;
        if (unnumberedVertex != w && label >= wLabel)
          subgraph.removeVertex(unnumberedVertex);
      }

      if (subgraph.hasPath(v, w)) wLabel.addNumber(i);
    }
  }
  return numberedVertices;
}

VectorT<int_t> Formula::getMcsVarOrdering() const {
  Graph graph = getGaifmanGraph();

  auto startVertex = graph.beginVertices();
  if (startVertex == graph.endVertices()) /* empty graph */
    return VectorT<int_t>();

  MapT<int_t, int_t> rankedNeighborCounts; /* unranked vertex |-> number of ranked neighbors */
  for (auto it = std::next(startVertex); it != graph.endVertices(); it++) rankedNeighborCounts[*it] = 0;

  int_t bestVertex = *startVertex;
  int_t bestRankedNeighborCount = DUMMY_MIN_INT;

  std::vector<int_t> varOrdering;
  do {
    varOrdering.push_back(bestVertex);

    rankedNeighborCounts.erase(bestVertex);

    for (auto n = graph.beginNeighbors(bestVertex); n != graph.endNeighbors(bestVertex); n++) {
      auto entry = rankedNeighborCounts.find(*n);
      if (entry != rankedNeighborCounts.end()) entry->second++;
    }

    bestRankedNeighborCount = DUMMY_MIN_INT;
    for (const std::pair<int_t, int_t> &entry : rankedNeighborCounts)
      if (entry.second > bestRankedNeighborCount) {
        bestRankedNeighborCount = entry.second;
        bestVertex = entry.first;
      }
  }
  while (bestRankedNeighborCount != DUMMY_MIN_INT);

  return varOrdering;
}

VectorT<int_t> Formula::getVarOrdering(VarOrderingHeuristic varOrderingHeuristic, bool inverse) const {
  VectorT<int_t> varOrdering;
  switch (varOrderingHeuristic) {
    case VarOrderingHeuristic::APPEARANCE: {
      varOrdering =  getAppearanceVarOrdering();
      break;
    }
    case VarOrderingHeuristic::DECLARATION: {
      varOrdering =  getDeclarationVarOrdering();
      break;
    }
    case VarOrderingHeuristic::RANDOM: {
      varOrdering =  getRandomVarOrdering();
      break;
    }
    case VarOrderingHeuristic::LEXP: {
      varOrdering =  getLexpVarOrdering();
      break;
    }
    case VarOrderingHeuristic::LEXM: {
      varOrdering =  getLexmVarOrdering();
      break;
    }
    case VarOrderingHeuristic::MCS: {
      varOrdering =  getMcsVarOrdering();
      break;
    }
    util::showError("no such varOrderingHeuristic");
  }
  if (inverse) {
    util::invert(varOrdering);
  }
  return varOrdering;
}

int_t Formula::getDeclaredVarCount() const { return declaredVarCount; }

MapT<int_t, double> Formula::getLiteralWeights() const { return literalWeights; }

const VectorT<VectorT<int_t>> &Formula::getCnf() const { return cnf; }

void Formula::printLiteralWeights() const {
  util::printLiteralWeights(literalWeights);
}

void Formula::printCnf() const {
  util::printCnf(cnf);
}

Formula::Formula(const std::string &filePath, WeightFormat weightFormat) {
  this->weightFormat = weightFormat;

  int_t declaredClauseCount = DUMMY_MIN_INT;
  int_t processedClauseCount = 0;

  int_t lineIndex = 0;
  int_t minic2dWeightLineIndex = lineIndex;

  std::ifstream inputFileStream(filePath);
  if (!inputFileStream.is_open()) util::showError("unable to open file " + filePath);
  std::string line;
  while (std::getline(inputFileStream, line)) {
    lineIndex++;
    std::istringstream inputStringStream(line);

    VectorT<std::string> words;
    std::copy(std::istream_iterator<std::string>(inputStringStream), std::istream_iterator<std::string>(), std::back_inserter(words));

    int_t wordCount = words.size();

    if (wordCount < 1) continue;

    std::string startWord = words.at(0);
    if (startWord == DIMACS_PROBLEM_WORD) {
      std::string cnf = words.at(1);
      if (words.at(1) != DIMACS_CNF_WORD)
        util::showError("expected '" + DIMACS_CNF_WORD + "', found '" + cnf + "' -- line " + std::to_string(lineIndex));
      declaredVarCount = std::stoi(words.at(2));
      declaredClauseCount = std::stoi(words.at(3));
    }
    else if (startWord == DIMACS_COMMENT_WORD) {
      if (weightFormat == WeightFormat::MINIC2D && wordCount > 1 && words.at(1) == MINIC2D_WEIGHT_WORD) { /* miniC2D weight line */
        if (minic2dWeightLineIndex > 0)
          util::showError("multiple miniC2D weight lines: " + std::to_string(minic2dWeightLineIndex) + ", " + std::to_string(lineIndex));
        else minic2dWeightLineIndex = lineIndex;

        if (wordCount != 2 + declaredVarCount * 2)
          util::showError("wrong number of miniC2D literal weights -- line " + std::to_string(lineIndex));

        for (int_t var = 1; var <= declaredVarCount; var++) {
          literalWeights[var] = std::stod(words.at(var * 2));
          literalWeights[-var] = std::stod(words.at(var * 2 + 1));
        }
      }
    }
    else if (startWord == CACHET_WEIGHT_WORD) {
      if (weightFormat != WeightFormat::CACHET)
        util::showError("Cachet weight in non-Cachet-weight-format file -- line " + std::to_string(lineIndex));

      int_t var = std::stoi(words.at(1));
      if (var <= 0 || var > declaredVarCount)
        util::showError("var '" + std::to_string(var) + "' is inconsistent with declared var count '" + std::to_string(declaredVarCount) + "' -- line " + std::to_string(lineIndex));
      double weight = std::stod(words.at(2));
      literalWeights[var] = weight;
    }
    else { /* clause line */
      VectorT<int_t> clause;
      for (int_t i = 0; i < wordCount; i++) {
        int_t num = std::stoi(words.at(i));

        if (num > declaredVarCount || num < -declaredVarCount) {
          util::showError("literal '" + std::to_string(num) + "' is inconsistent with declared var count '" + std::to_string(declaredVarCount) + "' -- line " + std::to_string(lineIndex));
        }

        if (num == DIMACS_CLAUSE_LINE_END) {
          if (i != wordCount - 1) {
            util::showError("clause terminated mid-line by '" + std::to_string(DIMACS_CLAUSE_LINE_END) + "' -- line " + std::to_string(lineIndex));
          }

          addClause(clause);
          processedClauseCount++;
        }
        else { /* literal */
          if (i == wordCount - 1)
            util::showError("missing end-of-clause indicator '" + std::to_string(DIMACS_CLAUSE_LINE_END) + "' -- line " + std::to_string(lineIndex));

          clause.push_back(num);
        }
      }
    }
  }
  inputFileStream.close();

  if (weightFormat == WeightFormat::MINIC2D && minic2dWeightLineIndex == 0) {
    util::showError("miniC2D weight line not found");
  }

  if (weightFormat == WeightFormat::UNWEIGHTED) { /* populates literalWeights with 1s */
     for (int_t var = 1; var <= declaredVarCount; var++) {
       literalWeights[var] = 1;
       literalWeights[-var] = 1;
     }
   }

  if (weightFormat == WeightFormat::CACHET) { /* completes literalWeights */
    for (int_t var = 1; var <= declaredVarCount; var++) {
      double varWeight = DEFAULT_VAR_WEIGHT;
      if (literalWeights.find(var) != literalWeights.end()) varWeight = literalWeights.at(var);

      double negativeLiteralWeight = 1 - varWeight;
      if (varWeight == -1) varWeight = negativeLiteralWeight = 1;

      literalWeights[var] = varWeight;
      literalWeights[-var] = negativeLiteralWeight;
    }
  }

  util::printRow("declaredVarCount", declaredVarCount);
  util::printRow("apparentVarCount", apparentVars.size());

  util::printRow("declaredClauseCount", declaredClauseCount);
  util::printRow("apparentClauseCount", processedClauseCount);
}

Formula::Formula(const VectorT<VectorT<int_t>> &clauses) {
  cnf = clauses;

  for (const VectorT<int_t> &clause : clauses)
    for (int_t literal : clause) updateApparentVars(literal);
}

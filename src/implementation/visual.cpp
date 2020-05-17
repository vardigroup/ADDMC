/* inclusions *****************************************************************/

#include "../interface/visual.hpp"

/* global functions ***********************************************************/

/* prints a DD's summmary
 * pr = 0 : prints nothing
 * pr = 1 : prints counts of nodes and minterms
 * pr = 2 : prints counts + disjoint sum of product
 * pr = 3 : prints counts + list of nodes
 * pr > 3 : prints counts + disjoint sum of product + list of nodes */
void printDd(const Cudd &mgr, const ADD &dd, int n, int pr) {
  DdManager *gbm = mgr.getManager();
  DdNode *ddNode = dd.getNode();

  printf("DdManager nodes: %ld | ", Cudd_ReadNodeCount(gbm)); // reports number of live nodes in DDs
  printf("DdManager vars: %d | ", Cudd_ReadSize(gbm)); // returns number of DD vars in existence
  Cudd_PrintDebug(gbm, ddNode, n, pr); // prints to standard output a DD and its statistics: number of nodes, number of leaves, number of minterms
}

/* writes a dot file representing argument DDs */
void writeDd(const Cudd &mgr, const ADD &dd, const string &filePath) {
  DdManager *gbm = mgr.getManager();
  DdNode *ddNode = dd.getNode();
  const char *filename = filePath.c_str();

  FILE *outfile; // output file pointer for .dot file
  outfile = fopen(filename, "wb");
  DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*)); // initializes function array
  ddnodearray[0] = ddNode;
  Cudd_DumpDot(gbm, 1, ddnodearray, NULL, NULL, outfile); // dumps function to .dot file
  free(ddnodearray);
  fclose (outfile); // closes file

  cout << "Overwrote file " << filePath << "\n";
}

ADD projectDdVar(const Cudd &mgr, const ADD &dd, Int var) {
  return dd.Compose(mgr.addOne(), var) + dd.Compose(mgr.addZero(), var);
}

void mainVisual(int argc, char *argv[]) {
  Cudd mgr;
  ADD x1, x2, x3, dd1, dd2, dd3, dd4;

  x1 = mgr.addVar(1);
  x2 = mgr.addVar(2);
  x3 = mgr.addVar(3);

  dd1 = (x1 | x2) & (x1 | ~x3);
  dd2 = projectDdVar(mgr, dd1, 3);
  dd3 = projectDdVar(mgr, dd2, 2);
  dd4 = projectDdVar(mgr, dd3, 1);

  writeDd(mgr, dd1, DOT_DIR + "dd1.dot");
  writeDd(mgr, dd2, DOT_DIR + "dd2.dot");
  writeDd(mgr, dd3, DOT_DIR + "dd3.dot");
  writeDd(mgr, dd4, DOT_DIR + "dd4.dot");
}

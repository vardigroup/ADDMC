/* includes *******************************************************************/

#include "../interface/visual.hpp"

/* globals ********************************************************************/

/* prints a DD's summmary
 * pr = 0 : prints nothing
 * pr = 1 : prints counts of nodes and minterms
 * pr = 2 : prints counts + disjoint sum of product
 * pr = 3 : prints counts + list of nodes
 * pr > 3 : prints counts + disjoint sum of product + list of nodes */
void printDd(const Cudd &mgr, const ADD &add, int n, int pr) {
  DdManager *gbm = mgr.getManager();
  DdNode *dd = add.getNode();

  printf("DdManager nodes: %ld | ", Cudd_ReadNodeCount(gbm)); /* reports number of live nodes in DDs */
  printf("DdManager vars: %d | ", Cudd_ReadSize(gbm)); /* returns number of DD vars in existence */
  Cudd_PrintDebug(gbm, dd, n, pr); /* prints to standard output a DD and its statistics: number of nodes, number of leaves, number of minterms */
}

/* writes a dot file representing argument DDs */
void writeDd(const Cudd &mgr, const ADD &add, const std::string &filePath) {
  DdManager *gbm = mgr.getManager();
  DdNode *dd = add.getNode();
  const char *filename = filePath.c_str();

  FILE *outfile; /* output file pointer for .dot file*/
  outfile = fopen(filename, "wb");
  DdNode **ddnodearray = (DdNode**)malloc(sizeof(DdNode*)); /* initializes function array */
  ddnodearray[0] = dd;
  Cudd_DumpDot(gbm, 1, ddnodearray, NULL, NULL, outfile); /* dumps function to .dot file */
  free(ddnodearray);
  fclose (outfile); /* closes file */

  std::cout << "Overwrote file " << filePath << "\n";
}

ADD projectAddVar(const Cudd &mgr, const ADD &add, int_t var) {
  return add.Compose(mgr.addOne(), var) + add.Compose(mgr.addZero(), var);
}

void mainVisual(int argc, const char *argv[]) {
  Cudd mgr;
  ADD x1, x2, x3, dd1, dd2, dd3, dd4;

  x1 = mgr.addVar(1);
  x2 = mgr.addVar(2);
  x3 = mgr.addVar(3);

  dd1 = (x1 | x2) & (x1 | ~x3);
  dd2 = projectAddVar(mgr, dd1, 3);
  dd3 = projectAddVar(mgr, dd2, 2);
  dd4 = projectAddVar(mgr, dd3, 1);

  writeDd(mgr, dd1, DOT_DIR + "dd1.dot");
  writeDd(mgr, dd2, DOT_DIR + "dd2.dot");
  writeDd(mgr, dd3, DOT_DIR + "dd3.dot");
  writeDd(mgr, dd4, DOT_DIR + "dd4.dot");
}

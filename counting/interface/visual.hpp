#pragma once

/* includes *******************************************************************/

#include "util.hpp"

/* globals ********************************************************************/

void printDd(const Cudd &mgr, const ADD &add, int n, int pr);

void writeDd(const Cudd &mgr, const ADD &add, const std::string &filePath);

ADD projectAddVar(const Cudd &mgr, const ADD &add, int_t var);

void mainVisual(int argc, const char *argv[]);

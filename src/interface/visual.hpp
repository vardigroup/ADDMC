#pragma once

/* inclusions *****************************************************************/

#include "util.hpp"

/* uses ***********************************************************************/

using util::printComment;
using util::printThickLine;
using util::printThinLine;
using util::showError;
using util::showWarning;

/* global functions ***********************************************************/

void printDd(const Cudd &mgr, const ADD &dd, int n, int pr);

void writeDd(const Cudd &mgr, const ADD &dd, const string &filePath);

ADD projectDdVar(const Cudd &mgr, const ADD &dd, Int var);

void mainVisual(int argc, char *argv[]);

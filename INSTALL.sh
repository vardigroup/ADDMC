#!/bin/bash

################################################################################

ECHO="echo -e"

LINE="========================================================================="

ADDMC_BIN="counting/addmc"

################################################################################

function makeCudd {
  unzip libraries.zip

  cd libraries/cudd-3.0.0
  ./configure --enable-obj
  make
  # make check
  cd ../..
}

function makeAddmc {
  $ECHO "\n$LINE\nMaking $ADDMC_BIN\n"

  make -C counting

  $ECHO "\nTry:\n\t$ADDMC_BIN\n"
}

################################################################################

makeCudd
makeAddmc

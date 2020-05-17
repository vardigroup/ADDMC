#!/bin/bash

################################################################################

ECHO="echo -e"

LINE="############################################################"

BIN_DIR="." # repo root

################################################################################

function makeLib {
  unzip -o lib.zip #NOTE overwriting without prompting

  cd lib/cudd-3.0.0

  ./configure --enable-obj
  make
  # make check

  cd -
}

################################################################################

function makeBin {
  bin="$1"

  $ECHO "$LINE\nMaking $BIN_DIR/$bin ...\n"
  make $bin
  $ECHO "\n$LINE\nTry:\n\t$BIN_DIR/$bin -h\n"
}

function makeAddmc {
  makeBin addmc
}

################################################################################

makeLib
makeAddmc

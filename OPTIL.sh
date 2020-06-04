#!/usr/bin/bash

make lib
tar czf addmc.tgz lib src CMakeLists.txt

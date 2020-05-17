# ADDMC (Algebraic Decision Diagram Model Counter)

<!-- ####################################################################### -->

## Description
- ADDMC computes exact literal-weighted model counts of formulas in conjunctive normal form
  - Developer: [Vu H. N. Phan][url_homepage_vp]
- AAAI-2020 paper: **ADDMC: Weighted Model Counting with Algebraic Decision Diagrams**
  - Authors:
    - [Jeffrey M. Dudek][url_homepage_jd]
    - [Vu H. N. Phan][url_homepage_vp]
    - [Moshe Y. Vardi][url_homepage_mv]

<!-- ####################################################################### -->

## [Releases with Assets for Download](https://github.com/vardigroup/ADDMC/releases)

- 2020/02/02: [v1.0.0](https://github.com/vardigroup/ADDMC/releases/tag/v1.0.0)
  - Assets:
    - ADDMC source code (C++): [ADDMC.zip][url_v1_0_0_addmc_zip]
    - Benchmarks: [benchmarks.zip][url_v1_0_0_benchmarks_zip]
    - Experimental data: [experimenting.zip][url_v1_0_0_experimenting_zip]

<!-- ####################################################################### -->

## Installation (Linux)

### Prerequisites
- ADDMC source code
- g++ 6.4.0
- make
- unzip

### Commands
```
$ ./INSTALL.sh
```

<!-- ####################################################################### -->

## Examples

### Showing command-line options
```
$ counting/addmc

============================================================
ADDMC: exact Model Counter using Algebraic Decision Diagrams
Version v1.0.0, released on 2020/02/02

Usage:
  addmc [OPTION...]

 Required options:
      --cf arg  CNF file path

 Optional options:
      --wf arg  Weight format options:
                 1	UNWEIGHTED
                 2	MINIC2D        default: 2
                 3	CACHET
      --ch arg  Clustering heuristic options:
                 1	MONOLITHIC
                 2	LINEAR
                 3	BUCKET_LIST
                 4	BUCKET_TREE
                 5	BOUQUET_LIST
                 6	BOUQUET_TREE   default: 6
      --cv arg  Cluster variable order heuristic options (negate to invert):
                 1	APPEARANCE
                 2	DECLARATION
                 3	RANDOM
                 4	MCS
                 5	LEXP           default: 5
                 6	LEXM
      --dv arg  Diagram variable order heuristic options (negate to invert):
                 1	APPEARANCE
                 2	DECLARATION
                 3	RANDOM
                 4	MCS            default: 4
                 5	LEXP
                 6	LEXM
```

### Solving DIMACS file with weight format `UNWEIGHTED`
```
$ counting/addmc --cf examples/UNWEIGHTED.cnf --wf 1

============================================================
ADDMC: exact Model Counter using Algebraic Decision Diagrams
Version v1.0.0, released on 2020/02/02

Reading DIMACS CNF file:
* cnfFilePath                   examples/UNWEIGHTED.cnf
* declaredVarCount              2
* apparentVarCount              2
* declaredClauseCount           3
* apparentClauseCount           3

Constructing model counter:
* weightFormat                  UNWEIGHTED
* clustering                    BOUQUET_TREE
* clusterVarOrder               LEXP
* inverseClusterVarOrder        0
* diagramVarOrder               MCS
* inverseDiagramVarOrder        0

Counting models...
* modelCount                    1
* seconds                       0.015
```

### Solving DIMACS file with weight format `MINIC2D`
```
$ counting/addmc --cf examples/MINIC2D.cnf --wf 2 --ch 3

============================================================
ADDMC: exact Model Counter using Algebraic Decision Diagrams
Version v1.0.0, released on 2020/02/02

Reading DIMACS CNF file:
* cnfFilePath                   examples/MINIC2D.cnf
* declaredVarCount              2
* apparentVarCount              2
* declaredClauseCount           3
* apparentClauseCount           3

Constructing model counter:
* weightFormat                  MINIC2D
* clustering                    BUCKET_LIST
* clusterVarOrder               LEXP
* inverseClusterVarOrder        0
* diagramVarOrder               MCS
* inverseDiagramVarOrder        0

Counting models...
* modelCount                    2.2
* seconds                       0.004
```

### Solving DIMACS file with weight format `CACHET`
```
$ counting/addmc --cf examples/CACHET.cnf --wf 3 --ch 3 --cv 6 --dv -4

============================================================
ADDMC: exact Model Counter using Algebraic Decision Diagrams
v1.0.0 on 2020/02/02

Reading DIMACS CNF file:
* cnfFilePath                   examples/CACHET.cnf
* declaredVarCount              2
* apparentVarCount              2
* declaredClauseCount           3
* apparentClauseCount           3

Constructing model counter:
* weightFormat                  CACHET
* clustering                    BUCKET_LIST
* clusterVarOrder               LEXM
* inverseClusterVarOrder        0
* diagramVarOrder               MCS
* inverseDiagramVarOrder        1

Counting models...
* modelCount                    0.3
* seconds                       0.004
```

<!-- ####################################################################### -->

## Acknowledgment
- Fabio Somenzi: [CUDD package][url_cudd_package]
- Rob Rutenbar: [CUDD tutorial][url_cudd_tutorial]
- David Kebo: [CUDD visualization][url_cudd_visualization]
- Lucas Tabajara: [RSynth][url_rsynth]
- Jarryd Beck: [cxxopts][url_cxxopts]

<!-- ####################################################################### -->

[url_homepage_jd]:http://jmd11.web.rice.edu/
[url_homepage_vp]:https://vuphan314.github.io/
[url_homepage_mv]:https://www.cs.rice.edu/~vardi/

[url_v1_0_0_addmc_zip]:https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/ADDMC.zip
[url_v1_0_0_benchmarks_zip]:https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/benchmarks.zip
[url_v1_0_0_experimenting_zip]:https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/experimenting.zip

[url_cudd_package]:https://github.com/ivmai/cudd
[url_cudd_tutorial]:http://db.zmitac.aei.polsl.pl/AO/dekbdd/F01-CUDD.pdf
[url_cudd_visualization]:http://davidkebo.com/cudd#cudd6
[url_rsynth]:https://bitbucket.org/lucas-mt/rsynth
[url_cxxopts]:https://github.com/jarro2783/cxxopts

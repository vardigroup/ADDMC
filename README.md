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

## [Releases](https://github.com/vardigroup/ADDMC/releases)

- 2020/06/07: [mc-2020](https://github.com/vardigroup/ADDMC/releases/tag/mc-2020) [![DOI](https://zenodo.org/badge/264571656.svg)](https://zenodo.org/badge/latestdoi/264571656)
  - weighted model counting competition
- 2020/02/02: [v1.0.0](https://github.com/vardigroup/ADDMC/releases/tag/v1.0.0)
  - AAAI 2020 conference
  - Assets:
    - ADDMC source code (C++): [ADDMC.zip][url_v1_0_0_addmc_zip]
    - Benchmarks: [benchmarks.zip][url_v1_0_0_benchmarks_zip]
    - Experimental data: [experimenting.zip][url_v1_0_0_experimenting_zip]

<!-- ####################################################################### -->

## Installation
See `INSTALL.md`

<!-- ####################################################################### -->

## Examples

### Showing command-line options
#### Command
```bash
./addmc -h
```
#### Output
```
==================================================================
ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
Version mc-2020, released on 2020/06/07
==================================================================

Usage:
  addmc [OPTION...]

 Optional options:
  -h, --hi      help information
      --cf arg  cnf file path (to use stdin, type: '--cf -')      Default: -
      --wf arg  weight format in cnf file:
           1    UNWEIGHTED                                        
           2    MINIC2D                                           
           3    CACHET                                            
           4    MCC                                               Default: 4
      --ch arg  clustering heuristic:
           1    MONOLITHIC                                        
           2    LINEAR                                            
           3    BUCKET_LIST                                       
           4    BUCKET_TREE                                       
           5    BOUQUET_LIST                                      
           6    BOUQUET_TREE                                      Default: 6
      --cv arg  cluster variable order heuristic (negate to invert):
           1    APPEARANCE                                        
           2    DECLARATION                                       
           3    RANDOM                                            
           4    MCS                                               
           5    LEXP                                              Default: 5
           6    LEXM                                              
      --dv arg  diagram variable order heuristic (negate to invert):
           1    APPEARANCE                                        
           2    DECLARATION                                       
           3    RANDOM                                            
           4    MCS                                               Default: 4
           5    LEXP                                              
           6    LEXM                                              
      --rs arg  random seed                                       Default: 10
      --vl arg  verbosity level:
           0    solution only                                     Default: 0
           1    parsed info as well                               
           2    clusters as well                                  
           3    cnf literal weights as well                       
           4    input lines as well                               
```

### Computing model count given cnf file from stdin
#### Command
```bash
./addmc < examples/track2_000.mcc2020_wcnf
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c Version mc-2020, released on 2020/06/07
c ==================================================================

c Process ID of this main program:
c pid 208191

c Reading CNF formula...
c ==================================================================
c Getting cnf from stdin... (end input with 'Enter' then 'Ctrl d')
c Getting cnf from stdin: done
c ==================================================================

c Computing output...
c ------------------------------------------------------------------
s wmc 1.37729e-05
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.034          
c ==================================================================
```

### Computing model count given cnf file with weight format `UNWEIGHTED`
#### Command
```bash
./addmc --cf examples/UNWEIGHTED.cnf --wf 1
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c Version mc-2020, released on 2020/06/07
c ==================================================================

c Process ID of this main program:
c pid 358012

c Reading CNF formula...

c Computing output...
c ------------------------------------------------------------------
s mc 1
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.019          
c ==================================================================
```

### Computing model count given cnf file with weight format `MINIC2D`
#### Command
```bash
./addmc --cf examples/MINIC2D.cnf --wf 2
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c Version mc-2020, released on 2020/06/07
c ==================================================================

c Process ID of this main program:
c pid 358102

c Reading CNF formula...

c Computing output...
c ------------------------------------------------------------------
s wmc 2.2
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.018          
c ==================================================================
```

### Computing model count given cnf file with weight format `CACHET`
#### Command
```bash
./addmc --cf examples/CACHET.cnf --wf 3
```
#### Output
```
c ==================================================================
c ADDMC: Algebraic Decision Diagram Model Counter (help: 'addmc -h')
c Version mc-2020, released on 2020/06/07
c ==================================================================

c Process ID of this main program:
c pid 358118

c Reading CNF formula...

c Computing output...
c ------------------------------------------------------------------
s wmc 0.3
c ------------------------------------------------------------------

c ==================================================================
c seconds                       0.019          
c ==================================================================
```

<!-- ####################################################################### -->

## Acknowledgment
- Lucas Tabajara: [RSynth][url_rsynth]
- Fabio Somenzi: [CUDD package][url_cudd_package]
- Rob Rutenbar: [CUDD tutorial][url_cudd_tutorial]
- David Kebo: [CUDD visualization][url_cudd_visualization]
- Jarryd Beck: [cxxopts][url_cxxopts]
- Henry Kautz and Tian Sang: [Cachet][url_cachet]
- Markus Hecher and Johannes Fichte: [model counting competition][url_mcc]

<!-- ####################################################################### -->

[url_homepage_jd]:http://jmd11.web.rice.edu/
[url_homepage_vp]:https://vuphan314.github.io/
[url_homepage_mv]:https://www.cs.rice.edu/~vardi/

[url_v1_0_0_addmc_zip]:https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/ADDMC.zip
[url_v1_0_0_benchmarks_zip]:https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/benchmarks.zip
[url_v1_0_0_experimenting_zip]:https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/experimenting.zip

[url_rsynth]:https://bitbucket.org/lucas-mt/rsynth
[url_cudd_package]:https://github.com/ivmai/cudd
[url_cudd_tutorial]:http://db.zmitac.aei.polsl.pl/AO/dekbdd/F01-CUDD.pdf
[url_cudd_visualization]:http://davidkebo.com/cudd#cudd6
[url_cxxopts]:https://github.com/jarro2783/cxxopts
[url_cachet]:https://www.cs.rochester.edu/u/kautz/Cachet/Model_Counting_Benchmarks/index.htm
[url_mcc]:https://mccompetition.org/2020/mc_format

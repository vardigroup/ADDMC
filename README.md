# ADDMC (Algebraic-Decision-Diagram Model Counter)

--------------------------------------------------------------------------------

## Description
- ADDMC computes exact literal-weighted model counts of formulas in conjunctive normal form
  - Developer: Vu H. N. Phan
- M.S. thesis: [**Weighted Model Counting with Algebraic Decision Diagrams**](https://scholarship.rice.edu/handle/1911/107761)
  - Author: Vu H. N. Phan
- AAAI-2020 paper: [**ADDMC: Weighted Model Counting with Algebraic Decision Diagrams**](https://arxiv.org/abs/1907.05000)
  - Authors:
    - [Jeffrey M. Dudek](https://jmd11.web.rice.edu)
    - [Vu H. N. Phan](https://vuphan314.github.io)
    - [Moshe Y. Vardi](https://cs.rice.edu/~vardi)

--------------------------------------------------------------------------------

## [Releases](https://github.com/vardigroup/ADDMC/releases)

- 2020/06/07: [mc-2020](https://github.com/vardigroup/ADDMC/releases/tag/mc-2020) [![DOI](https://zenodo.org/badge/264571656.svg)](https://zenodo.org/badge/latestdoi/264571656)
  - Model Counting Competition, weighted track
- 2020/02/02: [v1.0.0](https://github.com/vardigroup/ADDMC/releases/tag/v1.0.0)
  - AAAI 2020 conference
  - Assets:
    - Benchmarks: [benchmarks.zip](https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/benchmarks.zip)
    - Experimental data: [experimenting.zip](https://github.com/vardigroup/ADDMC/releases/download/v1.0.0/experimenting.zip)

--------------------------------------------------------------------------------

## Installation
See `INSTALL.md`

### macOS (OS X) build

ADDMC builds on macOS with Apple clang. The sources were originally written for
Linux/g++; three changes were needed to compile on macOS, all confined to the build:

1. **`CMakeLists.txt` — no `-static` on macOS.** macOS has no static system
   libraries, so Apple's linker rejects fully static executables. The `-static`
   flag is now applied only on non-Apple platforms (`IF(APPLE)` branch).
2. **`CMakeLists.txt` — `-O3` instead of `-Ofast` on macOS.** `-Ofast` enables
   `-ffast-math`, which makes ADDMC's `-infinity` log-space sentinel undefined
   behavior under clang. The Apple branch uses `-O3` to keep the counts correct.
3. **`src/interface/util.hpp` — add `#include <sstream>`.** macOS clang's libc++
   no longer transitively includes `<sstream>` (used by `formula.cpp` and
   `join.cpp`); libstdc++ on Linux did, which is why upstream never needed it.

Two more things are environment-specific rather than source changes:

- **CMake 4.x** removed compatibility with the `CMAKE_MINIMUM_REQUIRED(VERSION
  2.8.12)` declared here. Configure with
  `cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..` (or use an older CMake).
- The bundled **CUDD 3.0.0** builds from `lib.tar` via its own `./configure`
  (driven by the CMake custom command); no changes are required there.

Build:
```bash
mkdir -p build && cd build
cmake -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
make -f Makefile && cp addmc ..
```

--------------------------------------------------------------------------------

## Numerical behavior

ADDMC exposes CUDD's terminal-merging epsilon as an option.
`Counter::getModelCount` (`src/implementation/counter.cpp`) calls
`mgr.SetEpsilon(cuddEpsilon)`, where `cuddEpsilon` comes from the **`--ep`**
command-line option and **defaults to `0`** (exact), rather than CUDD's built-in
default of `1e-12`.

CUDD merges algebraic-decision-diagram terminal values that are within epsilon of
each other, including merging tiny values into the `0` terminal. With small literal
weights, a weighted model count can legitimately be far below `1e-12`
(e.g. `exp(-69) ≈ 1e-30`), and CUDD's default epsilon would round such counts down to
exactly `0`. With epsilon `0` that merging is disabled, so the count is exact down to
ordinary double-precision underflow. Example, two variables with a single clause
`x1 ∨ x2` and `W(x1=1)=W(x2=1)=1e-30`:

```
--ep 1e-12 (CUDD's built-in default): s wmc 0
--ep 0     (ADDMC default):           s wmc 2e-30
```

Pass `--ep <e>` to trade exactness for the speed/memory of more terminal merging
(a larger `e` merges more nodes); `--ep 0` keeps full double precision.

--------------------------------------------------------------------------------

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

--------------------------------------------------------------------------------

## Acknowledgment
- Tabajara: [RSynth](https://bitbucket.org/lucas-mt/rsynth)
- Somenzi: [CUDD package](https://github.com/ivmai/cudd)
- Rutenbar: [CUDD tutorial](http://db.zmitac.aei.polsl.pl/AO/dekbdd/F01-CUDD.pdf)
- Kebo: [CUDD visualization](https://davidkebo.com/cudd#cudd6)
- Beck: [cxxopts](https://github.com/jarro2783/cxxopts)
- Kautz and Sang: [Cachet](https://henrykautz.com/Cachet/)
- Hecher and Fichte: [Model Counting Competition](https://mccompetition.org/2020/mc_format)

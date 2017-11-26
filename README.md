# 2017CAD PA2 - Technology Mapping with the Minumum Critical Path Delay

In this programming assignment, students are asked to write a C or C++ program
to perform technology mapping on a subject graph given a subject graph and a 
cell library.

## Prerequisites

* GNU 4.7.2
* GNU Make 3.81
or higher

## Build

Use `make` to build, a binary _hw2_ will be generated.

## Usage

```
hw2 <sub_ckt> <cell_lib> <map_ckt> <path_rpt>
```
1. sub_ckt: the circuit to map 
2. cell_lib: cell library
3. map_ckt: output circuit after mapping, which consists of cells in cell_lib
4. path_rpt: timing report of critical path and corresponding delay

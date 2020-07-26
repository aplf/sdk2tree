# sdk2tree graph data structure

## Description

This directory contains the source-code of the **sdk2tree**, the dynamic *k*<sup>2</sup>-tree structure described in [On Dynamic Succinct Graph Representations](https://doi.org/10.1109/DCC47342.2020.00029),
by Miguel E. Coimbra, Alexandre P. Francisco, Luís MS Russo, Guillermo De Bernardo, Susana Ladra and Gonzalo Navarro (2020).

## Usage examples

    make
    ./project 9 < ../../datasets/dataset-adds-lists.tsv

        Running dynamic k2tree (INESC-ID). 
        N[0] ->2
        N[1] ->2
        N[2] ->1
        N[3] ->1
        N[4] ->1
        N[5] ->2
        N[6] ->1
        N[7] ->1
        N[8] ->0
        > Exiting.
        Loop time: 0.000203

    #### Argument info:
    # 9 - number of vertices
    # ../../datasets/dataset-adds-lists.tsv - path to a graph operations file with edge additions followed by listings

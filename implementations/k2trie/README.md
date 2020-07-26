# k2trie graph data structure

## Description

This directory contains the source-code of the **k2trie**, the dynamic k<sup>2</sup>-trie structure described in the paper [Faster Dynamic Compressed d-ary Relations](https://doi.org/10.1007/978-3-030-32686-9_30),
by Diego Arroyuelo, Guillermo de Bernardo, Travis Gagie and Gonzalo Navarro (2019).

## Usage examples

    make

    ./project 96 96 1024 8 10 0.99 257 < ../../datasets/dataset-adds-checks.tsv
    
        Got 96 96 1024, 8 10, 0.990000, 257, 9
        1
        1
        1
        1
        1
        1
        1
        1
        1
        1
        1
        0
        > Exiting.
        Loop time: 86.000000

    #### Argument info:
    # 96 - S1: Currently not in use
    # 96 - S2: Default size for blocks at depths between L1 and L2
    # 1024 - S3: Default size for blocks at depths higher than L2 
    # 8 - L1: Maximum depth for pointer-based trie (the trie uses pointers up to this depth). The code assumes that at least L1 levels exist
    # 10 - L2: Depth separating block sizes S2 and S3
    # 0.99 - Alpha: Node filling rate
    # 257 - Number of nodes in the graph 
    # Assumptions: S3 > S2, L2 > L1, L1 < log(nNodes)
    ## For more information on these parameters, check
    ## [Faster Dynamic Compressed d-ary Relations]
    ## (https://doi.org/10.1007/978-3-030-32686-9_30)
    # ../../datasets/dataset-adds-lists.tsv - path to a graph operations file with 
    edge additions followed by listings

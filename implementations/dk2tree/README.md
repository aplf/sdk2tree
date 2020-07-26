# dk2tree graph data structure

## Description

This directory contains the source-code of the **dk2tree**, the dynamic k<sup>2</sup>-tree structure based on bit vectors and described in the paper [Compressed representation of dynamic binary relations with applications](https://doi.org/10.1016/j.is.2017.05.003),
by Nieves R. Brisaboa, Ana Cerdeira-Pena, Guillermo de Bernardo and Gonzalo Navarro (2017).

## Usage examples

    make
    ./project dataset 9 512 4 4 3 2 < ../../datasets/dataset-adds-lists.tsv

        > Running dynamic k2tree (UPC). 
        > Finished parsing arguments. 
        > Got kValues.
        > Created empty K2Tree.
        > K2Tree stack initiated.
        > Initial K2Tree info:
        N[0] ->2
        N[1] ->2
        N[2] ->1
        N[3] ->1
        N[4] ->1
        N[5] ->2
        N[6] ->1
        N[7] ->1
        N[8] ->0
        manager 0 2 blocks
        manager 1 0 blocks
        manager 2 0 blocks
        manager 3 0 blocks
        [INFO ][nodeManager-mem-complete.c:310]: sizes 545208 1024
        [INFO ][k2tree-common.c:1018]: mgr: 550328
        Memory usage : 550328
        Disk usage : 1824
        Destroying node manager.Creation time = -nan
        > Exiting.
        Loop time: 505.000000

    #### Argument info:
    # dataset - base name of the tested file.
    # 9 - number of vertices
    # 512 - block size
    # 4 - n-jumps
    # 4 - k-value
    # 3 - n-levels
    # 2 - last-k-value
    ## For more information on these parameters, check 
    ## [Compressed representation of dynamic binary relations with applications]
    ## (https://doi.org/10.1016/j.is.2017.05.003)
    # ../../datasets/dataset-adds-lists.tsv - path to a graph operations file with 
    edge additions followed by listings

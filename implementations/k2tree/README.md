# k2tree graph data structure

## Description

This directory contains the source-code of the **k2tree**, the static k<sup>2</sup>-tree structure which is described in the paper [Compact Representation of Web Graphs with Extended Functionality](https://doi.org/10.1016/j.is.2013.08.003),
by Nieves R. Brisaboa, Susana Ladra and Gonzalo Navarro (2014).

## Usage examples

    make

    # This is the static k2-tree version, need to generate .kt format file first.
    ./kt_buildTree ../../datasets/dataset.adj dataset
    mv dataset.kt ../../datasets/

    ./project ../../datasets/dataset dataset-lists_eval < ../../datasets/dataset-lists.tsv

        (2)
        (2)
        (1)
        (1)
        (1)
        (2)
        (0)
        (0)
        (0)
        > Exiting.
        Loop time: 70.000000

    #### Argument info:
    # ../../datasets/dataset - path to dataset file (written without .kt extension)
    # ../../datasets/dataset-lists.tsv - path to a graph operations 
    file with neighborhood listing operations exclusively

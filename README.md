# Dyanmic and static k2tree graph data structure implementations

## Description

We provide the source-code of several *k*<sup>2</sup>-tree implementations.
They are listed below, identified by the naming scheme provided in section *Experimental analysis* of the paper [On Dynamic Succinct Graph Representations](https://doi.org/10.1109/DCC47342.2020.00029),
by Miguel E. Coimbra, Alexandre P. Francisco, Luís MS Russo, Guillermo De Bernardo, Susana Ladra and Gonzalo Navarro (2020).
For each version we provide references to their respective publications.

## Implementations

* **sdk2tree** - the dynamic *k*<sup>2</sup>-tree structure described in [On Dynamic Succinct Graph Representations](https://doi.org/10.1109/DCC47342.2020.00029),
by Miguel E. Coimbra, Alexandre P. Francisco, Luís MS Russo, Guillermo De Bernardo, Susana Ladra and Gonzalo Navarro (2020) with an [online presentation available here](https://sigport.org/documents/dynamic-succinct-graph-representations).

Its source code is available in the directory:

        /implementations/sdk2tree


* **k2tree** - the static *k*<sup>2</sup>-tree structure which is described in the paper [Compact Representation of Web Graphs with Extended Functionality](https://doi.org/10.1016/j.is.2013.08.003),
by Nieves R. Brisaboa, Susana Ladra, and Gonzalo Navarro (2014).
Its source code is available in the directory:

        /implementations/k2tree

* **dk2tree** - dynamic *k*<sup>2</sup>-tree structure based on bit vectors and described in the paper [Compressed representation of dynamic binary relations with applications](https://doi.org/10.1016/j.is.2017.05.003),
by Nieves R. Brisaboa, Ana Cerdeira-Pena, Guillermo de Bernardo, and Gonzalo Navarro (2017).
Its source code is available in the directory:

        /implementations/dk2tree

* **k2trie** - dynamic *k*<sup>2</sup>-tries are described in the paper [Faster Dynamic Compressed d-ary Relations](https://doi.org/10.1007/978-3-030-32686-9_30),
by Diego Arroyuelo, Guillermo de Bernardo, Travis Gagie, and Gonzalo Navarro (2019).
Its source code is available in the directory:

        /implementations/k2trie


## Usage examples

### Dummy dataset to highlight how to use the interfaces.

We include dummy test files in:

        dataset/

### Executing each implementation.

All implementations are provided with working **Makefile** scripts to easily compile and execute as needed.

#### **sdk2tree**

        cd src
        make
        ./project 9 < ../dataset/dataset-adds-lists.tsv

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
        # ../dataset/dataset-adds-lists.tsv - path to a graph operations file with edge additions followed by listings

#### **k2tree**

        cd k2tree
        make

        # This is the static k2-tree version, need to generate .kt format file first.
        ./kt_buildTree ../dataset/dataset.adj dataset
        mv dataset.kt ../dataset/

        ./project ../dataset/dataset dataset-lists_eval < ../dataset/dataset-lists.tsv

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
        # ../dataset/dataset - path to dataset file (written without .kt extension)
        # ../dataset/dataset-lists.tsv - path to a graph operations 
		file with neighborhood listing operations exclusively


#### **dk2tree**

        cd dk2tree
        make
        cd src
        ./project dataset 9 512 4 4 3 2 < ../../dataset/dataset-adds-lists.tsv

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
        # ../../dataset/dataset-adds-lists.tsv - path to a graph operations file with 
		edge additions followed by listings

#### **k2trie**

        cd k2trie
        make

        ./project 96 96 1024 8 10 0.99 257 < ../dataset/dataset-adds-checks.tsv
		
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
        # ../../dataset/dataset-adds-lists.tsv - path to a graph operations file with 
		edge additions followed by listings

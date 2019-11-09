# sdk2tree



This repository contains the source-code of the **sdk2tree**, the dynamic k<sup>2</sup>-tree implementation used in the paper [On dynamic succinct graph representations](https://arxiv.org/).
Its source code is available in the directory:

    /src


We also provide the source-code of other k<sup>2</sup>-tree implementations compared against our **sdk2tree**.
They are listed below, identified by the naming scheme provided in section *Experimental analysis* of our paper.
For each version we provide references to their respective publications.


* **sdk2tree** - this implementation relies on ideas proposed in paper [Dynamic Data Structures for Document Collections and Graphs](https://doi.org/10.1145/2745754.2745778),
by J. Ian Munro, Yakov Nekrich, and Jeffrey Scott Vitter (2015), and it makes use of [Efficient Set Operations over k2-Trees](https://doi.org/10.1109/DCC.2015.9),
a paper by Nieves R. Brisaboa, Guillermo de Bernardo, Gilberto Gutiérrez, Susana Ladra, Miguel R. Penabad, and Brunny A. Troncoso (2015).

* **k2tree** - the static k<sup>2</sup>-trees are described in the paper [Compact Representation of Web Graphs with Extended Functionality](https://doi.org/10.1016/j.is.2013.08.003),
by Nieves R. Brisaboa, Susana Ladra, and Gonzalo Navarro (2014).

* **dk2tree** - Dynamic k<sup>2</sup>-trees are described in the paper [Compressed representation of dynamic binary relations with applications](https://doi.org/10.1016/j.is.2017.05.003),
by Nieves R. Brisaboa, Ana Cerdeira-Pena, Guillermo de Bernardo, and Gonzalo Navarro (2017).

* Dynamic k<sup>2</sup>-tries are described in the paper [Faster Dynamic Compressed d-ary Relations](https://doi.org/10.1007/978-3-030-32686-9_30),
by Diego Arroyuelo, Guillermo de Bernardo, Travis Gagie, and Gonzalo Navarro (2019).


# OsDNA

The aim of our project was the design and development of a data compression algorithm, dedicated to ondimensional inputs, in the specic text codification of the nucleotide sequences composing the DNA, generally known as Fasta format.

Further description and compression comparisons with other algorithms can be found [in the project pdf in ITA](https://github.com/mrneutro/osdna/blob/master/OsDNA.pdf).

## Compilation
Project is built with contains CmakeLists and can be built with next steps:

 * `cmake -DCMAKE_BUILD_TYPE=Release`
 * `make`

As the output will be compiled an executable file named osdna. We have tested on `Windows 10 with Cygwin🤮`, `Ubuntu 18.04` and `Mac OSX` and everything seems working. 

 
 
## Usage
 
To compress use:

`./osdna C <input_file> <output_file>`

Decompress:

`./osdna D <input_file> <output_file>`


## LICENSE

Copyright (c) 2019, Ferdinando D'Avino, Lino Sarto, Sergiy Shevchenko

[GPL3 License](https://github.com/mrneutro/osdna/blob/master/LICENSE)



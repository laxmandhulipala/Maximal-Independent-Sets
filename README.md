Implementations of Two Randomized Parallel Maximal Independent Set Algorithms
=====================

Organization
--------

The code is located in the apps/ directory, as Luby.C and Ghaffari.C
Example inputs are provided in the inputs/ directory. 

This project was built on top of Ligra: https://github.com/jshun/ligra. Please
look there for further exposition on framework internals.

Luby's algorithm: http://www.eecs.berkeley.edu/~jshun/simple-analysis.pdf
Ghaffari's algorithm: http://arxiv.org/abs/1506.05093

Compilation
--------

Compilation is done from within the apps/ directory. 

Requirements
--------

Compilers (one of the following)

* Intel icpc compiler
* g++ &gt;= 4.8.0 with support for Cilk+, 
* OpenMP

After the appropriate environment variables are set, to compile,
simply run

```
$ make -j 16  #compiles with 16 threads (thread count can be changed)
```

The following commands cleans the directory:
```
$ make clean #removes all executables
```

Running code 
-------
The applications take the input graph as input as well as an optional
flag "-s" to indicate a symmetric graph.  Symmetric graphs should be
called with the "-s" flag for better performance. For example:

```
$ ./Luby -s ../inputs/rMatGraph_J_5_100
``` 

Input Format 
-----------
The input format of unweighted graphs should be in the following format
 
 The adjacency graph format from the Problem Based Benchmark Suite
 (http://www.cs.cmu.edu/~pbbs/benchmarks/graphIO.html). The adjacency
 graph format starts with a sequence of offsets one for each vertex,
 followed by a sequence of directed edges ordered by their source
 vertex. The offset for a vertex i refers to the location of the start
 of a contiguous block of out edges for vertex i in the sequence of
 edges. The block continues until the offset of the next vertex, or
 the end if i is the last vertex. All vertices and offsets are 0 based
 and represented in decimal. The specific format is as follows:

AdjacencyGraph  
&lt;n>  
&lt;m>  
&lt;o0>  
&lt;o1>  
...  
&lt;o(n-1)>  
&lt;e0>  
&lt;e1>  
...  
&lt;e(m-1)>  

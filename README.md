SYMMETRIC MULTIPROCESSOR CACHE SIMULATOR:
-----------------------------------------
This SMP simulator implements the following cache coherence protocols:
1. MESI
2. MOESI
3. DRAGON UPDATE

HOW TO RUN THE SIMULATOR:
-------------------------
$ make

Inputs Required:

1.cache_size 
2.assoc 
3.block_size 
4.num_processors 
5.protocol 
6.trace_file

$./smp_cache cache_size assoc block_size num_processors protocol trace_file 

Above command executes the simulator with the parameters provided
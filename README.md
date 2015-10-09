SYMMETRIC MULTIPROCESSOR CACHE SIMULATOR:
-----------------------------------------
This SMP simulator implements the following cache coherence protocols:
1. MESI
2. MOESI
3. DRAGON UPDATE

HOW TO RUN THE SIMULATOR:
-------------------------
$ make
  Above command makes the project
  Inputs Required:
  <cache_size> 
  <assoc> 
  <block_size> 
  <num_processors> 
  <protocol> 
  <trac\
  e_file>

$./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> 
   Above command executes the simulator with the parameters provided
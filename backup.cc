void Cache::Access(ulong addr,uchar op,int proc_number,int num_processors,Cache* p,int protocol)
{
	currentCycle++;/*per cache global counter to maintain LRU order 
			among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	//Need to enter IF loop for protocol
	cacheLine * line = findLine(addr);
	if(line == NULL)/*miss*/
	{
		if(op == 'w') writeMisses++;
		else readMisses++;

		cacheLine *newline = fillLine(addr);
   		if(op == 'w') newline->setFlags(DIRTY);    
		
	}
	else
	{
		/**since it's a hit, update LRU and update dirty flag**/
		updateLRU(line);
		if(op == 'w') line->setFlags(DIRTY);
	}
}


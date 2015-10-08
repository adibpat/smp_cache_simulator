/*******************************************************
                          cache.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include "cache.h"
#include <iomanip>
#include <math.h>
using namespace std;

Cache::Cache(int s,int a,int b)
{
   ulong i, j;
   reads = readMisses = writes = 0; 
   writeMisses = writeBacks = currentCycle = 0;
   interventions=invalidations=c2c=c2m=m2i=s2i=m2s=s2m=missRate=i2s=i2m=flushes=flushopt,e2i=0;

   size       = (ulong)(s);
   lineSize   = (ulong)(b);
   assoc      = (ulong)(a);   
   sets       = (ulong)((s/b)/a);
   numLines   = (ulong)(s/b);
   log2Sets   = (ulong)(log2(sets));   
   log2Blk    = (ulong)(log2(b));   
  
   //*******************//
   //initialize your counters here//
   //*******************//
 
   tagMask =0;
   for(i=0;i<log2Sets;i++)
   {
		tagMask <<= 1;
        tagMask |= 1;
   }
   
   /**create a two dimentional cache, sized as cache[sets][assoc]**/ 
   cache = new cacheLine*[sets];
   for(i=0; i<sets; i++)
   {
      cache[i] = new cacheLine[assoc];
      for(j=0; j<assoc; j++) 
      {
	   cache[i][j].invalidate();
      }
   }      
   
}

/**you might add other parameters to Access()
since this function is an entry point 
to the memory hierarchy (i.e. caches)**/
void Cache::Access(ulong addr,uchar op,int proc_number,int num_processors,Cache** p,int protocol)
{
	if(protocol==0) // This is for MSI Protocol
	{	
	currentCycle++; /*per cache global counter to maintain LRU order among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else reads++;
	
	
	cacheLine * line = findLine(addr); // Check if its a hit or a miss
	if(line == NULL)/*miss*/
	{
		cacheLine *newLine = fillLine(addr); // Allocate new line // Update already done in fillLine function
   		if(op == 'w')
		{
			i2m++;
			writeMisses++;
			newLine->setFlags(MODIFIED);
			BusAccess(addr,proc_number,num_processors,protocol,2,p); // This is BusRdX				
		}
		else
		{
			i2s++;
			readMisses++;
			newLine->setFlags(SHARED);
			BusAccess(addr,proc_number,num_processors,protocol,1,p); // This is BusRd
		}    
						
	}
	else //its a hit
	{
		if (line->getFlags()==SHARED)//shared state
		{ 
			if(op == 'w') //write operation
			{
				s2m++;
				line->setFlags(MODIFIED);		
				updateLRU(line);
				BusAccess(addr,proc_number,num_processors,protocol,2,p); // This is BusRdX 					
					
			}    
			else //read operation
			{
				updateLRU(line);				
			}
		}
		else // Modified State
		{ 
		updateLRU(line);
		}  	
	}
		
	}
	else if(protocol==1) // This is MESI protocol
	  {
	currentCycle++; /*per cache global counter to maintain LRU order among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	
	
	cacheLine * line = findLine(addr); // Check if its a hit or a miss
	if(line == NULL)/*miss*/
	{
		cacheLine *newLine = fillLine(addr); // Allocate new line // Update already done in fillLine function
   		if(op == 'w')
		{
			i2m++;
			for(int i=0;i<num_processors;i++)
			{
			if(proc_number!=i)
			{
			cacheLine *temp=p[i]->findLine(addr);
			if(temp!=NULL){c2c++;break;}
			}
			}
			writeMisses++;
			newLine->setFlags(MODIFIED);
			BusAccess(addr,proc_number,num_processors,protocol,2,p); // This is BusRdX				
		}
		else
		{
			cacheCounter=0;
			for(int i=0;i<num_processors;i++)
			{
			if(proc_number!=i)
			{
			cacheLine *temp=p[i]->findLine(addr);
			if(temp!=NULL){c2c++;break;}
			}
			}
			for(int i=0;i<num_processors;i++)
			{
			if(i!=proc_number)
			{
			cacheLine *checkLine=p[i]->findLine(addr);
			if(checkLine!=NULL)cacheCounter++;
			}
			}
			if(cacheCounter!=0) {newLine->setFlags(SHARED);}  
			else {newLine->setFlags(EXCLUSIVE);}
			readMisses++;
			BusAccess(addr,proc_number,num_processors,protocol,1,p); // This is BusRd
		}    
						
	}
	else //its a hit
	{
		if (line->getFlags()==SHARED)//shared state
		{ 
			if(op == 'w') //write operation
			{
				s2m++;
				line->setFlags(MODIFIED);		
				updateLRU(line);
				BusAccess(addr,proc_number,num_processors,protocol,3,p); // This is BusUpgr 			
			}    
			else //read operation
			{
				updateLRU(line);				
			}
		}
		else if (line->getFlags()==MODIFIED) // Modified State
		{ 
		updateLRU(line);
		}
		else // Exclusive State
		{
		  if(op=='w')
		    {
		line->setFlags(MODIFIED); updateLRU(line);
		    }
		  else 
		    {
		 updateLRU(line);
		    }
		}  	
	}
	  }
  else if(protocol==2)
   {
	currentCycle++; /*per cache global counter to maintain LRU order among cache ways, updated on every cache access*/
        	
	if(op == 'w') writes++;
	else          reads++;
	
	
	cacheLine * line = findLine(addr); // Check if its a hit or a miss
	if(line == NULL)/*miss*/
	{
		cacheLine *newLine = fillLine(addr); // Allocate new line // Update already done in fillLine function
   		if(op == 'w')
		{
			cacheCounter=0;writeMisses++;
			BusAccess(addr,proc_number,num_processors,protocol,1,p); // This is BusRd 
			BusAccess(addr,proc_number,num_processors,protocol,4,p); // This is BusUpd
			for(int i=0;i<num_processors;i++)
			{
			if(i!=proc_number)
			{
			cacheLine *checkLine=p[i]->findLine(addr);
			if(checkLine!=NULL)cacheCounter++;
			}
			}
			if(cacheCounter!=0) 
			{
			newLine->setFlags(SM);
						
			 }  
			else 
			{
			newLine->setFlags(MODIFIED);
			}
							
		}
		else
		{
			cacheCounter=0;readMisses++;
			BusAccess(addr,proc_number,num_processors,protocol,1,p); // This is a BusRd

			for(int i=0;i<num_processors;i++)
			{
			if(i!=proc_number)
			{
			cacheLine *checkLine=p[i]->findLine(addr);
			if(checkLine!=NULL)cacheCounter++;
			}
			}
			
			if(cacheCounter!=0) 
			{
			newLine->setFlags(SC); 
			}  
			else 
			{
			newLine->setFlags(EXCLUSIVE); 
			}
		}    
						
	}
	else //its a hit
	{
		if (line->getFlags()==SM)//shared-Modified state
		{ 
			if(op == 'w') //write operation
			{
			BusAccess(addr,proc_number,num_processors,protocol,4,p); // This is BusUpd
			cacheCounter=0;
			for(int i=0;i<num_processors;i++)
			{
			if(i!=proc_number)
			{
			cacheLine *checkLine=p[i]->findLine(addr);
			if(checkLine!=NULL)cacheCounter++;
			}
			}
			if(cacheCounter==0) 
			{
			line->setFlags(MODIFIED); updateLRU(line);
			}  
			else 
			{
			updateLRU(line); 
			}		
			}    
			else //read operation
			{
				updateLRU(line);				
			}
		}
		else if (line->getFlags()==MODIFIED) // Modified State
		{ 
		updateLRU(line);
		}
		else if(line->getFlags()==EXCLUSIVE)// Exclusive State
		{
		  if(op=='w')
		    {
		    line->setFlags(MODIFIED); 
		    updateLRU(line);
		    }
		  else 
		    {
		   updateLRU(line);
		    }
		}
		else // Shared Clean
		{
		  if(op == 'w') //write operation
			{
			BusAccess(addr,proc_number,num_processors,protocol,4,p); // This is BusUpd
			cacheCounter=0;
			for(int i=0;i<num_processors;i++)
			{
			if(i!=proc_number)
			{
			cacheLine *checkLine=p[i]->findLine(addr);
			if(checkLine!=NULL)cacheCounter++;
			}
			}
			
			if(cacheCounter==0) 
			{
			line->setFlags(MODIFIED); updateLRU(line);
			}  
			else 
			{
			line->setFlags(SM); updateLRU(line);
			}		
			}    
		  else //read operation
			{
				updateLRU(line);				
			}
		}  	
	}
   }	
}

/*look up line*/
cacheLine * Cache::findLine(ulong addr)
{
   ulong i, j, tag, pos;
   
   pos = assoc;
   tag = calcTag(addr);
   i   = calcIndex(addr);
  
   for(j=0; j<assoc; j++)
	if(cache[i][j].isValid())
	        if(cache[i][j].getTag() == tag)
		{
		     pos = j; break; 
		}
   if(pos == assoc)
	return NULL;
   else
	return &(cache[i][pos]); 
}

/*upgrade LRU line to be MRU line*/
void Cache::updateLRU(cacheLine *line)
{
  line->setSeq(currentCycle);  
}

/*return an invalid line as LRU, if any, otherwise return LRU line*/
cacheLine * Cache::getLRU(ulong addr)
{
   ulong i, j, victim, min;

   victim = assoc;
   min    = currentCycle;
   i      = calcIndex(addr);
   
   for(j=0;j<assoc;j++)
   {
      if(cache[i][j].isValid() == 0) return &(cache[i][j]);     
   }   
   for(j=0;j<assoc;j++)
   {
	 if(cache[i][j].getSeq() <= min) { victim = j; min = cache[i][j].getSeq();}
   } 
   assert(victim != assoc);
   
   return &(cache[i][victim]);
}

/*find a victim, move it to MRU position*/
cacheLine *Cache::findLineToReplace(ulong addr)
{
   cacheLine * victim = getLRU(addr);
   updateLRU(victim);
  
   return (victim);
}

/*allocate a new line*/
cacheLine *Cache::fillLine(ulong addr)
{ 
   ulong tag;
  
   cacheLine *victim = findLineToReplace(addr);
   assert(victim != 0);
   if((victim->getFlags() == MODIFIED) | (victim->getFlags() == SM)) writeBack(addr);
    	
   tag = calcTag(addr);   
   victim->setTag(tag);
   victim->setFlags(SHARED);    
   /**note that this cache line has been already 
      upgraded to MRU in the previous function (findLineToReplace)**/

   return victim;
}

/* Bus Operation Function*/
void Cache::BusAccess(ulong addr,int proc_number,int num_processors,int protocol,int bus,Cache **p)
{
/*1:BusRd,2:BusRdX,3:BusUpgr,4:BusUpd*/
  if(protocol==0)
	{
	if(bus==1) // BusRd
	  {
	for(int i=0;i<num_processors;i++)
	  if(i!=proc_number)
		{
		cacheLine *checkLine= p[i]->findLine(addr);
		if(checkLine!=NULL)
		{
		if(checkLine->getFlags()==MODIFIED){checkLine->setFlags(SHARED); p[i]->m2s++;p[i]->flushes++;p[i]->interventions++;}
		else if(checkLine->getFlags()==SHARED){}
		}
		}	
	  }
	  else // BusRdX
		{
		for(int i=0;i<num_processors;i++)
	  if(i!=proc_number)
		{
		cacheLine *checkLine= p[i]->findLine(addr);
		if(checkLine!=NULL)
		{
		if(checkLine->getFlags()==MODIFIED){checkLine->setFlags(INVALID); p[i]->m2i++; p[i]->invalidations++;p[i]->flushes++;}
		else if(checkLine->getFlags()==SHARED){checkLine->setFlags(INVALID); p[i]->s2i++;p[i]->invalidations++;}
		}
		}	
		}
	}
  else if(protocol==1) // MESI Protocol BusAccess
	{
	if(bus==1) // BusRd
	  {
	for(int i=0;i<num_processors;i++)
	  if(i!=proc_number)
		{
		cacheLine *checkLine= p[i]->findLine(addr);
		if(checkLine!=NULL)
		{
		if(checkLine->getFlags()==MODIFIED){checkLine->setFlags(SHARED); p[i]->m2s++;p[i]->flushes++;p[i]->interventions++;}
		else if(checkLine->getFlags()==EXCLUSIVE){checkLine->setFlags(SHARED);p[i]->flushopt++;p[i]->interventions++;}
		else if(checkLine->getFlags()==SHARED) {p[i]->flushopt++;}
		else {}
		}
		}	
	  }
	else if(bus==2) // BusRdX
	  {
	for(int i=0;i<num_processors;i++)
	  if(i!=proc_number)
		{
		cacheLine *checkLine= p[i]->findLine(addr);
		if(checkLine!=NULL)
		{
		if(checkLine->getFlags()==MODIFIED){checkLine->setFlags(INVALID); p[i]->m2i++;p[i]->flushes++; p[i]->invalidations++;}
		else if(checkLine->getFlags()==EXCLUSIVE){checkLine->setFlags(INVALID);p[i]->e2i++;p[i]->flushopt++; p[i]->invalidations++;}
		else if(checkLine->getFlags()==SHARED){checkLine->setFlags(INVALID);p[i]->s2i++;p[i]->flushopt++; p[i]->invalidations++;}
		else {}
		}
		}	
	  }
	else // BusUpgr for MESI
	  {
	for(int i=0;i<num_processors;i++)
	  if(i!=proc_number)
		{
		cacheLine *checkLine= p[i]->findLine(addr);
		if(checkLine!=NULL)
		{
		if(checkLine->getFlags()==SHARED){checkLine->setFlags(INVALID); p[i]->s2i++; p[i]->invalidations++;}
		else {}
		}
		}	
	  }
	}
  else if(protocol==2)
    {
	if(bus==1) // BusRd for Dragon
	  {
	for(int i=0;i<num_processors;i++)
	  if(i!=proc_number)
		{
		cacheLine *checkLine= p[i]->findLine(addr);
		if(checkLine!=NULL)
		{
		if(checkLine->getFlags()==EXCLUSIVE){checkLine->setFlags(SC); p[i]->e2sc++;p[i]->interventions++;}
		else if(checkLine->getFlags()==SC){}
		else if(checkLine->getFlags()==MODIFIED){checkLine->setFlags(SM);p[i]->flushes++;p[i]->m2sm++;p[i]->interventions++;}
		else if(checkLine->getFlags()==SM){p[i]->flushes++;}
		}
		}	
	  }
	  else if(bus==4)// BusUpd for Dragon
		{
		for(int i=0;i<num_processors;i++)
	  if(i!=proc_number)
		{
		cacheLine *checkLine= p[i]->findLine(addr);
		if(checkLine!=NULL)
		{
		if(checkLine->getFlags()==SM){checkLine->setFlags(SC); p[i]->sm2sc++;}
		}
		}	
		}
    }
}

void Cache::printStatsMSI()
{ 
ulong writeBacksMSI = writeBacks+flushes;
cout<<" 01.number of reads:				"<<reads<<endl;
cout<<" 02.number of read misses:			"<<readMisses<<endl;
cout<<" 03.number of writes:				"<<writes<<endl;
cout<<" 04.number of write misses:			"<<writeMisses<<endl;
cout<<" 05.total miss rate:	                        "<<setprecision(3)<<(float)(readMisses+writeMisses)*100/(reads+writes)<<"%"<<endl;
cout<<" 06.number of write backs:			"<<writeBacksMSI<<endl;
cout<<" 07.number of cache-to-cache transfers:	        "<<c2c<<endl;
cout<<" 08.number of memory transactions:	        "<<readMisses+writeMisses+writeBacksMSI+s2m-c2c<<endl;
cout<<" 09.number of interventions:			"<<interventions<<endl;
cout<<" 10.number of invalidations:			"<<invalidations<<endl;
cout<<" 10.number of flushes:			        "<<flushes<<endl;

}

void Cache::printStatsMESI()
{ 
ulong writeBacksMESI = writeBacks+flushes;
cout<<" 01.number of reads:				"<<reads<<endl;
cout<<" 02.number of read misses:			"<<readMisses<<endl;
cout<<" 03.number of writes:				"<<writes<<endl;
cout<<" 04.number of write misses:			"<<writeMisses<<endl;
cout<<" 05.total miss rate:	                        "<<setprecision(3)<<(float)(readMisses+writeMisses)*100/(reads+writes)<<"%"<<endl;
cout<<" 06.number of write backs:			"<<writeBacksMESI<<endl;
cout<<" 07.number of cache-to-cache transfers:	        "<<c2c<<endl;
cout<<" 08.number of memory transactions:	        "<<readMisses+writeMisses+writeBacksMESI-c2c<<endl;
cout<<" 09.number of interventions:			"<<interventions<<endl;
cout<<" 10.number of invalidations:			"<<invalidations<<endl;
cout<<" 10.number of flushes:			        "<<flushes<<endl;

}

void Cache::printStatsDrg()
{ 
ulong writeBacksDrag = writeBacks+flushes;
cout<<" 01.number of reads:				"<<reads<<endl;
cout<<" 02.number of read misses:			"<<readMisses<<endl;
cout<<" 03.number of writes:				"<<writes<<endl;
cout<<" 04.number of write misses:			"<<writeMisses<<endl;
cout<<" 05.total miss rate:	                        "<<setprecision(3)<<(float)(readMisses+writeMisses)*100/(reads+writes)<<"%"<<endl;
cout<<" 06.number of write backs:			"<<writeBacksDrag<<endl;
cout<<" 07.number of cache-to-cache transfers:	        "<<c2c<<endl;
cout<<" 08.number of memory transactions:	        "<<readMisses+writeMisses+writeBacksDrag<<endl;
cout<<" 09.number of interventions:			"<<interventions<<endl;
cout<<" 10.number of invalidations:			"<<invalidations<<endl;
cout<<" 10.number of flushes:			        "<<flushes<<endl;

}

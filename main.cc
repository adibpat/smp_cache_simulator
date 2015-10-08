/*******************************************************
                          main.cc
                  Ahmad Samih & Yan Solihin
                           2009
                {aasamih,solihin}@ece.ncsu.edu
********************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fstream>
using namespace std;

#include "cache.h"

int main(int argc, char *argv[])
{
	
	ifstream trace;
	//FILE * pFile;

	if(argv[1] == NULL){
		 printf("input format: ");
		 printf("./smp_cache <cache_size> <assoc> <block_size> <num_processors> <protocol> <trace_file> \n");
		 exit(0);
        }

	
	int cache_size = atoi(argv[1]);
	int cache_assoc= atoi(argv[2]);
	int blk_size   = atoi(argv[3]);
	int num_processors = atoi(argv[4]);/*1, 2, 4, 8*/
	int protocol   = atoi(argv[5]);	 /*0:MSI, 1:MESI, 2:Dragon*/
	char *trace_file=argv[6];
	int proc_number;
	char op;
	unsigned long int addr;

	cout<<"===== 506 Personal information ====="<<endl;
	cout<<"Aditya Bhushan Patwardhan"<<endl;
	cout<<"UnityID abpatwar"<<endl;
	cout<<"Section ECE/CSC 506-002"<<endl;
	
	
	
	// Simulator Configuration //
	cout<<"===== Simulator configuration =====\n";
	cout<<"L1_SIZE:	"; cout<<cache_size;
	cout<<"\nL1_ASSOC:	"; cout<<cache_assoc;
	cout<<"\nL1_BLOCKSIZE:	"; cout<<blk_size;
	cout<<"\nNUMBER OF PROCESSORS:	"; cout<<num_processors;
	cout<<"\nCOHERENCE PROTOCOL:	"; if(protocol==0) cout<<"MSI"; else if(protocol==1) cout<<"MESI"; else cout<<"Dragon";
	cout<<"\nTRACE FILE:	"; cout<<trace_file;
	
        //*****create an array of caches here**********//
	
	Cache **p;	
	p= new Cache*[num_processors];
	  for (int i=0;i<num_processors;i++)
            {
		//cout<<i;
		p[i]=new Cache(cache_size,cache_assoc,blk_size);
            }

	// Read Trace File //

	trace.open(argv[6]);
	if(trace.is_open())
	{
	while(trace>>dec>>proc_number>>op>>hex>>addr)
	{//cout<<"inside while";
	if(op=='r') p[proc_number]->Access(addr,op,proc_number,num_processors,p,protocol);
	else p[proc_number]->Access(addr,op,proc_number,num_processors,p,protocol);
	}
	}


	//********************************//
	//print out all caches' statistics //
	//********************************//
	for(int i=0;i<num_processors;i++)
{
cout<<"\n===== Simulation results (Cache_";
cout<<i<<") ====="<<endl;
if(protocol==0)
p[i]->printStatsMSI();
else if(protocol==1)
p[i]->printStatsMESI();
else
p[i]->printStatsDrg();
}

	trace.close();
	return 0;
}

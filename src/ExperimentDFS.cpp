/*
 * Experiment1.cpp
 *
 *  Created on: Oct 4, 2012
 *      Author: carolinux
 */

#include <iostream>
using namespace std;
#include<string>
#include "Util.h"
#include "Database.h"
#include "Query.h"
#include "Synopsis.h"
#include "Bloom.h"
#include <assert.h>
#include <fstream> //for parsing config file
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "MultiTrie.h"
#include "Synopsis/SynopsisDFS.h"
#include <sstream>
#define CFG_PARAMS 24
//EXP MODE//
bool VARYINT = false; // vary interval size
bool VARYFS = false; //vary filter size
bool VARYDBS = false;
bool VARYPART = false;
bool SKIPBL = false;


//  Database Parameters
uint DBDOMAIN = 1048575;  // domain of the keys of the database
uint DBSIZE = 1000;    // number of keys in the cold store of the database
bool ZIPFDATA = 0;
double ZIPFDATAFACTOR = 1.2;  // zipf factor for database

int PARTITION_MODE = 0; //AGNOSTIC
int REPL_POLICY = CLOCK;
//  Query Workload Parameters

int EXPQUERIES = 10000;

int TRAININGQUERIES = 1000;    // number of range queries used for training
bool USEOLDZIPFINTERVALT = false;
double MEANT = 3000;
double STDDEVT = 1000;

int MINRANGE;      // minimum length of range
int JUSTPOINTQUERIES = 1;

double MEAN = 30;
double STDDEV = 10;
bool USEOLDZIPFINTERVAL = 0;

bool ZIPFQUERIES = 0;
double ZIPFQUERIESFACTOR = 1.2;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE = 100;
// Book-keeping
char * OUTPUTFILE;


//MultiTrie

int PARTITIONS = 1;
bool NOTRUNCATE = false;

void print_config();
int main(int argc, char* argv[]) {


	RANGEFILTERSIZE = atoi(argv[1]);
	JUSTPOINTQUERIES = atoi(argv[2]);
	if(JUSTPOINTQUERIES!=1)
	{
		MEAN = JUSTPOINTQUERIES;
		STDDEV = MEAN/3.0;
		JUSTPOINTQUERIES = 0;

	}
	cpu_set_t  mask;
		CPU_ZERO(&mask);
		CPU_SET(0, &mask);
		int result = sched_setaffinity(0, sizeof(mask), &mask);
		assert(result == 0);


  // populate and define database
  Database db(DBDOMAIN+1);

  if(ZIPFDATA)
  {
	  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
	  db.populate(DBSIZE, &zipf);
  }

  else
  {
	  Uniform unif(DBDOMAIN+1);
	  db.populate(DBSIZE, &unif);
  }

  db.plot();



  SynopsisIntClock synStart(DBDOMAIN,RANGEFILTERSIZE,true,&db);
   synStart.init();
   synStart.perfect(&db);

   cout<<"start experiment"<<endl;

   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);


   Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
 		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
 		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
   for (int i=0; i<TRAININGQUERIES/3; i++) {
     Query::Query_t q = qTrain1.nextQuery();
     if(JUSTPOINTQUERIES)
        	   q.left=q.right;
     bool qR = db.rangeQuery(q);
     synStart.handleQuery(q,&db,true,qR);



   }

   cout<<"2nd training.."<<endl;

   Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
 		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
 		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
   for (int i=0; i<2*TRAININGQUERIES/3; i++) {
     Query::Query_t q = qTrain2.nextQuery();
     bool qR = db.rangeQuery(q);
     synStart.handleQuery(q,&db,true,qR);

   }


   cout<<"end"<<endl;



   SynopsisIntClock synStart0(synStart);
   synStart0.set(0);




   synStart0.truncateClock(RANGEFILTERSIZE);


   synStart0.resetTime();


   synStart0.convertToCompact(0);

   Synopsis syn(synStart0);
   syn.set(0);
   SynopsisDFS synd(syn);
   syn.setDatabase(&db);
   //syn.exportGraphViz("bfs.txt");
   //synd.exportGraphViz("dfs.txt");
   //return 0;



  // return 0;
  //JUSTPOINTQUERIES = 1;
   cout<<"--------------END OF TRAINING----------------------"<<endl;

  /* Real experiment - Tries are frozen (no adaptation) */
   Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV,
		   ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);

  int fpBloom = 0,tnBloom=0;
  int tp = 0;
  int tn = 0;

    for (int i=0; i<EXPQUERIES; i++)
   {

		Query::Query_t q = qGen.nextQuery();

		if(JUSTPOINTQUERIES)
			q.right=q.left;

		bool qR = db.rangeQuery(q);

		if(qR)
		  tp++;
		else
		  tn++;


		bool bfs = syn.handleQuery(q,&db,false,qR);
		bool dfs = synd.handleQuery(q);
		assert(bfs == dfs);

		bool sR;
		if(JUSTPOINTQUERIES)
			sR = bl.pointQuery(q.left);
		else
			sR = bl.rangeQuery(q);

		if(sR && !qR)
			 fpBloom++;
		if(!sR && !qR)
			tnBloom++;
    }

  struct stats bfst = calculate(list2vector(syn.lookupt));
  struct stats dfst = calculate(list2vector(synd.lookupt));
  printf("BFS MEAN\t BFS STDDEV\t DFS MEAN\t DFS STDDEV\t \n");
  FILE * t = fopen("temp_plot.txt","a");
  printf("%llu\t %llu\t  %llu\t %llu\t \n",bfst.avg,bfst.std,dfst.avg,dfst.std);
  fprintf(t,"%d %llu\t %llu\t  %llu\t %llu\t \n",RANGEFILTERSIZE,bfst.avg,bfst.std,dfst.avg,dfst.std);
  fclose(t);


  print_config();
  return 0;
}  // main


void print_config()
{

	cout<<"--------------SETTINGS--------------"<<endl;
	cout<<"Domain: "<<DBDOMAIN<<endl;
	cout<<"Size of cold store: "<<DBSIZE<<endl;
	cout<<"Range/Bloom filter size: "<<RANGEFILTERSIZE<<endl;
	//cout<<"outpt file: "<<OUTPUTFILE<<endl;
	cout<<"MEAN :"<<MEAN<<endl;
	cout<<"POINTQUERIES?"<<JUSTPOINTQUERIES<<endl;
	cout<<"VARY FILTER SIZE? ="<<VARYFS<<endl;
	cout<<"VARY INTERVAL SIZE? ="<<VARYINT<<endl;
	cout<<"Zipfdata? "<<ZIPFDATA<<endl;
	cout<<"Zipfqueries? "<<ZIPFQUERIES<<endl;



	cout<<"------------------------------------"<<endl;
}



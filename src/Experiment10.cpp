/*
 * Experiment3.cpp
 *
 *  Created on: Oct 8, 2012
 *      Author: carolinux
 */

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
#include "Synopsis/FastSynopsis.h"
#include "Bloom.h"
#include <assert.h>
#include <fstream> //for parsing config file
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "MultiTrie.h"
#include <sstream>

#define CFG_PARAMS 24
//EXP MODE//
bool VARYINT = false; // vary interval size
bool VARYFS = false; //vary filter size
bool VARYDBS = false;
bool VARYPART = false;
bool SKIPBL = false;
bool ZIPFUPDATES = false;
bool ZIPFCOLD = false;

//  Database Parameters
uint DBDOMAIN = 1048575;  // domain of the keys of the database
uint DBSIZE = 1000;    // number of keys in the cold store of the database
bool ZIPFDATA;
double ZIPFDATAFACTOR =1.2;  // zipf factor for database

int PARTITION_MODE = 0; //AGNOSTIC
int REPL_POLICY = CLOCK;
//  Query Workload Parameters

int UPDATEBATCH;    // number of updates per round
int EXPQUERIES = 300000;
bool DODELETES;

int TRAININGQUERIES = 51000;    // number of range queries used for training
bool USEOLDZIPFINTERVALT = false;
double MEANT = 3000;
double STDDEVT = 1000;

int MINRANGE;      // minimum length of range
int JUSTPOINTQUERIES;
double MEAN=30;
double STDDEV=10;
bool USEOLDZIPFINTERVAL = false;

bool ZIPFQUERIES;
double ZIPFQUERIESFACTOR=1.2;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE;  // size of synopsis in nodes
//int BLOOMFILTERSIZE;//  2 * RANGEFILTERSIZE  // size of poor man's Bloom filter in bits


// Book-keeping
char * OUTPUTFILE;


//MultiTrie

int PARTITIONS = 1;
bool NOTRUNCATE = false;

int main(int argc, char* argv[]) {

  cout<<"Experiment 10 (measuring workload change behavior) started.."<<endl;

  int SEED1 = 42;
  int SEED2 = 4042;
  ZIPFDATA = false;
  ZIPFQUERIES = true;
  
  // populate and define database


  
  
  DBSIZE = atoi(argv[2]);
RANGEFILTERSIZE = atoi(argv[3])* DBSIZE;
  uint P = atoi(argv[1]);
 DBDOMAIN = (2<<P)-1;

  Database db(DBDOMAIN+1);
  
  Uniform unif(DBDOMAIN+1);
db.populate(DBSIZE, &unif);

  db.plot();

FastSynopsis fsyn(0, DBDOMAIN, &db);
	fsyn.perfect(&db);

        FastSynopsis fsyn1(1, DBDOMAIN, &db);
        fsyn1.perfect(&db);

      FastSynopsis fsyn0(0, DBDOMAIN, &db);
        fsyn0.perfect(&db);





  Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);



  Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT,SEED1);

/*  Query qTrain1A(DBDOMAIN, 0,MEAN,STDDEV,
 		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
 		  ZIPFQUERIES,USEOLDZIPFINTERVALT,42);

  Query qTrain1B(DBDOMAIN, 0,MEAN,STDDEV,
 		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
 		  ZIPFQUERIES,USEOLDZIPFINTERVALT,33);
  return 1;*/


  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();
    if(JUSTPOINTQUERIES == 1)
    	q.left = q.right;
    bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);
		  fsyn1.handle_query(q.left, q.right, qR, true);
fsyn0.handle_query(q.left, q.right, qR, true);

  }

 Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT,SEED1);


  for (int i=0; i<2*TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain2.nextQuery();
    bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);
		  fsyn1.handle_query(q.left, q.right, qR, true);
fsyn0.handle_query(q.left, q.right, qR, true);
  }





  


  cout<<"--------------END OF TRAINING----------------------"<<endl;

		fsyn.reset_training_phase();
	fsyn.truncate(RANGEFILTERSIZE);
	fsyn.end_training_phase();
                     fsyn0.reset_training_phase();
        fsyn0.truncate(RANGEFILTERSIZE);
        fsyn0.end_training_phase();
                fsyn1.reset_training_phase();
        fsyn1.truncate(RANGEFILTERSIZE);
        fsyn1.end_training_phase();

  /* Real experiment - Tries adapt */

  cout<<"upd\t syn\t\t syn0\t syn1\t bloom\t"<<endl;
  int fpBloom = 0;
  int tnBloom = 0;
  int tp = 0;
  int tn = 0;
  Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,false,SEED1);
  Query qGen2(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,false,SEED2);
  for(int j =0;j<2;j++)
  {
  // --- RESET queries --- //

  qGen.reset();
  qGen2.reset();
    for (int i=0; i<=EXPQUERIES; i++)
   {

		Query::Query_t q = qGen.nextQuery();
		if(j ==1)
			q = qGen2.nextQuery();
		//cout<<q.left<<"-"<<q.right<<endl;
	bool dR = db.rangeQuery(q);
			bool qR = bl.handleQuery(q, dR);
			 fsyn.handle_query(q.left, q.right, dR, false);
			fsyn1.handle_query(q.left, q.right, dR, true);
			bool res1 = fsyn0.handle_query(q.left, q.right, dR, true);
			fsyn0.truncate(RANGEFILTERSIZE);
			fsyn1.truncate(RANGEFILTERSIZE);

	


  /* output results to file */

  if(i%30000 ==0 && i!=0)
  {
	    //cout<<"Writing to fiel: "<<OUTPUTFILE<<endl;
		FILE * results = fopen(OUTPUTFILE,"a");

		int pt = (j*EXPQUERIES) + i;

		  fprintf(stderr,"%d\t%f\t%f\t%f\t%f\n",
		    			pt,fsyn.stats.getFpr(),
		    			fsyn0.stats.getFpr(),
		    			fsyn1.stats.getFpr(),
		    			bl.stats.getFpr());


        bl.stats.reset();
	fsyn0.stats.reset();
	fsyn.stats.reset();
	fsyn1.stats.reset();


  }

} /* end of one round of updates */

   	   //qGen.generateHotZipf(ZIPFQUERIESFACTOR,MEAN,STDDEV);

    //qGen = Query(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,false,SEED2);



  } //end of all updates


  cout<<"--------END OF BENCHMARK 4--------"<<endl;

 // free(OUTPUTFILE);

  return 0;
}  // main






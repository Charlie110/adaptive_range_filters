/*
 * Experiment2r.cpp
 *
 *  Created on: Feb 12, 2013
 *      Author: carolinux
 */

/*
 * Experiment2.cpp
 *
 *  Created on: Oct 4, 2012
 *      Author: carolinux
 */

/* This benchmark measures time */

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
#include <sstream>

//#define _GNU_SOURCE //for using the GNU CPU affinity
// (works with the appropriate kernel and glibc)
// Set affinity mask
#include <sched.h>

//EXP MODE//
bool VARYINT = false; // vary interval size
bool VARYFS = false; //vary filter size
bool VARYDBS = false;
bool VARYPART = false;
bool SKIPBL = false;


//  Datasyn Parameters
uint DBDOMAIN = 1048575;  // domain of the keys of the datasyn
uint DBSIZE = 1000;    // number of keys in the cold store of the datasyn
bool ZIPFDATA = 0;

bool ZIPFQUERIES = 0;
double ZIPFDATAFACTOR = 1.2;  // zipf factor for datasyn

int PARTITION_MODE = 0; //AGNOSTIC
int REPL_POLICY = CLOCK;
//  Query Workload Parameters

int UPDATEBATCH;    // number of updates per round
int EXPQUERIES = 10000;
int TRAININGQUERIES = 5100;
bool DOUPDATES;
bool DODELETES;
bool ZIPFUPDATES;
  // number of range queries used for training
bool USEOLDZIPFINTERVALT = false;
double MEANT = 3000;
double STDDEVT= 1000;

int MINRANGE;      // minimum length of range
int JUSTPOINTQUERIES = 0;
double MEAN = 10;
double STDDEV = 30;
bool USEOLDZIPFINTERVAL = false;

double ZIPFQUERIESFACTOR = 1.2;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE = 1000;  // size of synopsis in nodes
//int BLOOMFILTERSIZE;//  2 * RANGEFILTERSIZE  // size of poor man's Bloom filter in bits


// Book-keeping
char * OUTPUTFILE;


//MultiTrie

int PARTITIONS = 1;
bool NOTRUNCATE = false;


int main(int argc, char* argv[]) {


	if(argc==1)
	{
		cout<<"Usage: ZIPF FILTERSIZE OUTPUT RANGELEN"<<endl;
		return 0;
	}

	/* run this on one core */
	ZIPFDATA = atoi(argv[1]);
	cout<<"arfc"<<argc<<endl;
	cout<<"zipf"<<endl;
	ZIPFQUERIES = atoi(argv[1]);
	RANGEFILTERSIZE = atoi(argv[2]);
	cout<<"sizxe"<<endl;
	OUTPUTFILE = (argv[3]);
	cout<<"outputfile"<<endl;
	MEAN = atoi(argv[4]);
	JUSTPOINTQUERIES = 0;
	STDDEV = 0;
	cout<<"mEAN: "<<MEAN<<endl;
	//getchar();
	cout<<"parsed input"<<endl;

	cpu_set_t  mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	int result = sched_setaffinity(0, sizeof(mask), &mask);
	assert(result == 0);

	/* thank you */


  cout<<"Experiment 2 (measuring trie lookup time) started ..."<<endl;


  // populate and define datasyn
  Database db(DBDOMAIN+1);
  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
  Uniform unif(DBDOMAIN+1);
  if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf);
  else
	db.populate(DBSIZE, &unif);
 // print_config();
  //db.plot();


  SynopsisIntClock synStart(DBDOMAIN,RANGEFILTERSIZE,true,&db);
     synStart.init();
     synStart.perfect(&db);


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

     Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
   		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
   		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
     for (int i=0; i<2*TRAININGQUERIES/3; i++) {
       Query::Query_t q = qTrain2.nextQuery();
       bool qR = db.rangeQuery(q);
       synStart.handleQuery(q,&db,true,qR);
     }



     cout<<"size:"<<RANGEFILTERSIZE<<endl;

     Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR,
    		 EXPQUERIES, ZIPFQUERIES,false);

     SynopsisIntClock synStart0(synStart);
     synStart0.set(0);

     synStart.truncateClock(RANGEFILTERSIZE);

     synStart0.truncateClock(RANGEFILTERSIZE);

     synStart.resetTime();

     synStart0.resetTime();

     synStart.convertToCompact(1);
     synStart0.convertToCompact(0);



     Synopsis syn0(synStart0);
     syn0.set(0);

     Synopsis syn1(synStart);
     syn1.set(1);

     Synopsis syn(syn0);


     cout<<"syn0: "<<syn0.size()<<endl;
     cout<<"syn1: "<<syn1.size()<<endl;
     cout<<"syn non-adaptive: "<<syn.size()<<endl;


     syn.resetTime();
     syn0.resetTime();
     syn1.resetTime();








     cout<<"--------------END OF TRAINING----------------------"<<endl;
  /* training complete */
  /* now we start measuring */
  /* Real experiment - Tries are frozen (no adaptation) */


  int fpBloom = 0;
  int tnBloom = 0;
  int tp = 0;
  int tn = 0;
  int LOOPS = 1;
  for(int j=0;j<LOOPS;j++)
  {

	  qGen.reset();
for (int i=0; i<EXPQUERIES; i++)
{

	Query::Query_t q = qGen.nextQuery();

	if(JUSTPOINTQUERIES)
		q.right=q.left;

	if(i%100 == 0)
		cout<<i<<"th benchmark OUT OF "<<EXPQUERIES<<endl;

	bool qR = db.rangeQuery(q);
	syn0.handleQuery(q,&db,true,qR);
	syn1.handleQuery(q,&db,true,qR);
	syn0.truncateClock(RANGEFILTERSIZE);
	syn1.truncateClock(RANGEFILTERSIZE);

	syn.handleQuery(q,&db,false,qR);

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
  }

cout<<"------end of benchmark---"<<endl;



/*

  assert(syn0.size()<=RANGEFILTERSIZE);
  assert(syn1.size()<=RANGEFILTERSIZE);
  assert(syn.size()<=RANGEFILTERSIZE);
  assert(syn1.lookupt.size() == EXPQUERIES*LOOPS);
  assert(syn0.lookupt.size() == EXPQUERIES*LOOPS);
  assert(syn.lookupt.size() == EXPQUERIES*LOOPS);
//  assert(syn1.falsepos.size() == syn1.truncatet.size());
 // assert(syn1.falsepos.size() == syn1.fp);
  //assert(syn0.falsepos.size() == syn0.truncatet.size());
  // gia ola ta kala paidakia edw twra :) //
  cout<<"SYN\t FPSYN0\t SYN1\t BLM"<<endl;
  cout<<syn.fp<<"\t"<<syn0.fp<<"\t"<<syn1.fp<<"\t"<<fpBloom<<endl;

  vector<uint64_t> fp1 = fptime(syn1.falsepos,syn1.truncatet);
  sort(fp1.begin(),fp1.end());

  plot(fp1);

  vector<uint64_t> fp0 = fptime(syn0.falsepos,syn0.truncatet);
   sort(fp0.begin(),fp0.end());
   plot(fp0);


   stats time1 = calculate(fptime(syn1.falsepos,syn1.truncatet));
   stats time0 = calculate(fptime(syn0.falsepos,syn0.truncatet));
   stats time = calculate(list2vector(syn.lookupt));
   stats timeb = calculate(list2vector(bl.lookupt));
   stats time1a = calculate(list2vector(syn1.falsepos));
   stats time1t = calculate(list2vector(syn1.truncatet));

   cout<<"Syn1 avg: "<<time1.avg<<endl;
   cout<<"Syn1 std: "<<time1.std<<endl;

   vector<uint64_t> synl = list2vector(syn.lookupt);
   sort(synl.begin(),synl.end());
   plot(synl);

*/




  /* output results to file */


  FILE * results = fopen(OUTPUTFILE,"a");




	/*fprintf(results,"%f\t %llu\t %llu\t  %llu\t %llu\t %llu\t %llu\t  \n",
  			MEAN,
  			time.avg,time.mean,time.std,
  			/*time0.avg,time0.avg,time0.std,
  			time1.avg,time1.mean,time1.std,
  			timeb.avg,timeb.mean,timeb.std);*/


  fprintf(results,"%f %f %f %f %f \n",
  	    			MEAN,syn.fp*100.0/(syn.tn+syn.fp),
  	    			syn0.fp*100.0/(syn0.tn+syn0.fp),
  	    			syn1.fp*100.0/(syn1.tn+syn1.fp),
  	    			fpBloom*100.0/(fpBloom+tnBloom));



	fflush(results);


	/*fprintf(results,"%d\t  %llu\t %llu\t %llu\t  %llu\t %llu\t %llu\t %llu\t %llu\t \n",
			RANGEFILTERSIZE,
			time0.avg,time0.mean,time0.std,
			time1.avg,time1.mean,time1.std,
			time1a.mean,time1t.mean);*/




  fclose(results);
 // free(OUTPUTFILE);

  return 0;
}  // main



void verify_config()
{

	//assert(VARYFS);
	assert(DBDOMAIN == closestPower2(DBDOMAIN)-1);
	assert(DBSIZE>0);
	assert(DBDOMAIN>0);
/*#ifdef SYNDEBUG
	assert(1==0); //we don't run the timing benchmark with debugging enabled
#endif*/

#ifndef TICKTOCK
	assert(1==0); //we don't run the timing benchmark without measuring time
#endif

#ifdef DOTRACE
	assert(1==0); //we don't run the timing benchmark while tracing
#endif

#ifdef SYNDEBUG
	assert(1==0); //we don't run the timing benchmark while tracing
#endif

}








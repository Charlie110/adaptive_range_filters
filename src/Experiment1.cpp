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
#include <sstream>
#include "Synopsis/FastSynopsis.h"
#define CFG_PARAMS 24
//EXP MODE//
bool VARYINT = false; // vary interval size
bool VARYFS = false; //vary filter size
bool VARYDBS = false;
bool VARYPART = false;
bool SKIPBL = false;


//  Database Parameters
uint DBDOMAIN;  // domain of the keys of the database
uint DBSIZE;    // number of keys in the cold store of the database
bool ZIPFDATA;
double ZIPFDATAFACTOR;  // zipf factor for database

int PARTITION_MODE = 0; //AGNOSTIC
int REPL_POLICY = CLOCK;
//  Query Workload Parameters

int UPDATEBATCH;    // number of updates per round
int EXPQUERIES;
bool DOUPDATES;
bool DODELETES;
bool ZIPFUPDATES;
int TRAININGQUERIES;    // number of range queries used for training
bool USEOLDZIPFINTERVALT = false;
double MEANT;
double STDDEVT;

int MINRANGE;      // minimum length of range
bool JUSTPOINTQUERIES;
double MEAN;
double STDDEV;
bool USEOLDZIPFINTERVAL;

bool ZIPFQUERIES;
double ZIPFQUERIESFACTOR;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE;  // size of synopsis in nodes
//int BLOOMFILTERSIZE;//  2 * RANGEFILTERSIZE  // size of poor man's Bloom filter in bits


// Book-keeping
char * OUTPUTFILE;


//MultiTrie

int PARTITIONS = 1;
bool NOTRUNCATE = false;

void parse_args(char * file);
void print_config();
void verify_config();
void getTimes(vector<double> &times,double * avg, double * avg90, double * maxx);

int main(int argc, char* argv[]) {

  assert(closestPower2(15)==16);
  assert(closestPower2(9)==16);
  if(argc<2)
  {
	cout<<"No config file specified. Exiting.."<<endl;
	return -1;
  }
  parse_args(argv[1]); //parses config file
  verify_config();
  cout<<"Experiment 1 (measuring trie accuracy) started.."<<endl;


  // populate and define database
  Database db(DBDOMAIN+1);
  cout<<"Zipf?"<<endl;

  cout<<"zipf..."<<endl;
  ZIPFQUERIESFACTOR = 1.2;
  int NEWDM = DBDOMAIN;

  if(ZIPFDATA)
  {
	  //Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
	  Zipf zipf(NEWDM+1,ZIPFDATAFACTOR);
	 // if(!ZIPFDATA && ZIPFQUERIES && JUSTPOINTQUERIES)
		//  zipf = Zipf(DBDOMAIN+1,ZIPFDATAFACTOR,true);

	  db.populate(DBSIZE, &zipf);
  }

  else
  {

	//  Uniform unif(DBDOMAIN+1);
	  Uniform unif( (NEWDM+1));
	  db.populate(DBSIZE, &unif);
  }
 // print_config();
  db.plot();






  SynopsisIntClock synStart(DBDOMAIN,RANGEFILTERSIZE,true,&db);

   synStart.init();
   synStart.perfect(&db);
   synStart.setDatabase(&db);
   //synStart.takeSnapshot();
   cout<<"Bits needed to represent syn:"<<synStart.size()<<endl;
   //getchar();

   cout<<"start experiment"<<endl;

   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);
   Bloom    blr(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE, 10);


   Query qTrain1(NEWDM, 0,MEAN,STDDEV,
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

  Query qTrain2(NEWDM, 0,MEANT,STDDEVT,
 		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
 		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
   for (int i=0; i<2*TRAININGQUERIES/3; i++) {
     Query::Query_t q = qTrain2.nextQuery();
     //if(JUSTPOINTQUERIES)
      //       	   q.left=q.right; //FIXME here
     bool qR = db.rangeQuery(q);
     synStart.handleQuery(q,&db,true,qR);

   }


   cout<<"end"<<endl;



   SynopsisIntClock synStart0(synStart);
   synStart0.set(0);

   synStart.truncateClock(RANGEFILTERSIZE);


   synStart0.truncateClock(RANGEFILTERSIZE);


  // synStart0.takeSnapshot();
   //synStart0.takeSnapshotPartial(2000,10);
   //getchar();

   synStart.resetTime();

   synStart0.resetTime();

   synStart.convertToCompact(1);
   synStart0.convertToCompact(0);

   Synopsis syn1(synStart);
   syn1.set(1);
   //syn1.sanityCheckUsed();
   Synopsis syn0(synStart0);
   syn0.set(0);

   Synopsis syn(syn0);

   cout<<"syn0: "<<syn0.size()<<endl;
   cout<<"syn1: "<<syn1.size()<<endl;
   cout<<"syn non-adaptive: "<<syn.size()<<endl;

   cout<<"lsyn0: "<<syn0.Numleaves()<<endl;
      cout<<"lsyn1: "<<syn1.Numleaves()<<endl;
      cout<<"lsyn non-adaptive: "<<syn.Numleaves()<<endl;

  // return 0;
  //JUSTPOINTQUERIES = 1;
   cout<<"--------------END OF TRAINING----------------------"<<endl;

  /* Real experiment - Tries are frozen (no adaptation) */
 /*  Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV,
		   ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);*/

   Query qGen(NEWDM, MINRANGE,MEAN,STDDEV,
  		   ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,false);


  int fpBloom = 0,tnBloom=0;
  int tp = 0;
  int tn = 0;

    for (int i=0; i<EXPQUERIES; i++)
   {

		Query::Query_t q = qGen.nextQuery();

		if(JUSTPOINTQUERIES)
			q.right=q.left;

		//cout<<"Pt:"<<q.right<<endl;

		if(i %100 == 0)
		{
			cout<<"benchmark "<<i<<endl;
		}

		bool qR = db.rangeQuery(q);

		if(qR)
		  tp++;
		else
		  tn++;

		syn0.handleQuery(q,&db,true,qR);

		syn1.handleQuery(q,&db,true,qR);
		syn.handleQuery(q,&db,false,qR);
		syn0.truncateClock(RANGEFILTERSIZE);
		syn1.truncateClock(RANGEFILTERSIZE);
	/*	if(i%100==0)
		{
		cout<<"synScore"<<syn.getScore(0,DBDOMAIN,&db)<<endl;
		cout<<"syn0Score"<<syn0.getScore(0,DBDOMAIN,&db)<<endl;
		cout<<"syn1Score"<<syn1.getScore(0,DBDOMAIN,&db)<<endl;
		getchar();
		}*/
		bool sR;
		if(JUSTPOINTQUERIES) {
			sR = bl.pointQuery(q.left);
		}
		else
			sR = bl.rangeQuery(q);

		blr.handleQueryRanged(q.left, q.right, qR);

		if(sR && !qR)
			 fpBloom++;
		if(!sR && !qR)
			tnBloom++;
    }
  /* end of benchmark */
  cout<<"--------END OF BENCHMARK 1--------"<<endl;

  cout<<"syn0: "<<syn0.size()<<endl;
  cout<<"syn1: "<<syn1.size()<<endl;
  cout<<"syn non-adaptive: "<<syn.size()<<endl;
  assert(syn0.size()<=RANGEFILTERSIZE);
  assert(syn1.size()<=RANGEFILTERSIZE);
  assert(syn.size()<=RANGEFILTERSIZE);
  /* output results to file */

  FILE * results = fopen(OUTPUTFILE,"a");
 if(VARYFS)
  {

  	/*fprintf(results,"%d %d %d %d %d %d \n",
  			RANGEFILTERSIZE,base0.fp,base1.fp,
  			syn0.fp,syn1.fp,
  			fpBloom);*/
	  fprintf(results,"%d %f %f %f %f \n",
	    			RANGEFILTERSIZE,syn.fp*100.0/(syn.tn+syn.fp),
	    			syn0.fp*100.0/(syn0.tn+syn0.fp),
	    			syn1.fp*100.0/(syn1.tn+syn1.fp),
	    			fpBloom*100.0/(fpBloom+tnBloom));
	  printf("%d %d %d %d %d \n",
	  	    			RANGEFILTERSIZE,syn.fp,
	  	    			syn0.fp,syn1.fp,
	  	    			fpBloom);

	  printf("%d %f %f %f %f \n",
	  	    			RANGEFILTERSIZE,syn.fp*100.0/(syn.tn+syn.fp),
	  	    			syn0.fp*100.0/(syn0.tn+syn0.fp),
	  	    			syn1.fp*100.0/(syn1.tn+syn1.fp),
	  	    			fpBloom*100.0/(fpBloom+tnBloom));
  }


 cout<<"Bloom actual error rate:"<<fpBloom*1.0/(fpBloom+tnBloom)<<endl;
 cout<<"Bloom-r error rate:"<<endl;
 blr.stats.print();

 cout<<"----"<<endl;
 cout<<"expected error rate from math:"<<bl.fpRate(MEAN)<<endl;
 cout<<"math error rate for 1 pt"<<bl.fpRate()<<endl;
 //getchar();

  fclose(results);
  free(OUTPUTFILE);


  print_config();
  return 0;
}  // main

void parse_args(char * file)
{
	ifstream cfg(file);
	//for(int i=0;i<CFG_PARAMS;i++)
	while(!cfg.eof())
	{
		string type;
		cfg >> type;

		string val;
		cfg >> val;

		if(!type.compare("RANGEFILTERSIZE"))
			RANGEFILTERSIZE = atoi(val.c_str());
  		if(!type.compare("ZIPFQUERIESFACTOR"))
			ZIPFQUERIESFACTOR = atof(val.c_str());
		if(!type.compare("ZIPFDATAFACTOR"))
			ZIPFDATAFACTOR = atof(val.c_str());
		if(!type.compare("MINRANGE"))
			MINRANGE = atoi(val.c_str());
		if(!type.compare("TRAININGQUERIES"))
			TRAININGQUERIES = atoi(val.c_str());
		if(!type.compare("EXPQUERIES"))
			EXPQUERIES = atoi(val.c_str());
		if(!type.compare("UPDATEBATCH"))
			UPDATEBATCH = atoi(val.c_str());
		if(!type.compare("DBSIZE"))
			DBSIZE = atoi(val.c_str());
		if(!type.compare("DBDOMAIN"))
			DBDOMAIN = atoi(val.c_str());
		if(!type.compare("OUTPUTFILE"))
		{
			OUTPUTFILE = (char *) malloc(1+sizeof(char)*strlen(val.c_str()));
			strcpy(OUTPUTFILE,val.c_str());
		}
		if(!type.compare("ZIPFDATA"))
			ZIPFDATA = atoi(val.c_str());
		if(!type.compare("ZIPFQUERIES"))
			ZIPFQUERIES = atoi(val.c_str());
		if(!type.compare("JUSTPOINTQUERIES"))
			JUSTPOINTQUERIES = atoi(val.c_str());
		if(!type.compare("DOUPDATES"))
			DOUPDATES = atoi(val.c_str());
		if(!type.compare("ZIPFUPDATES"))
			ZIPFUPDATES = atoi(val.c_str());
		if(!type.compare("DOUPDATES"))
			DODELETES = atoi(val.c_str());
		if(!type.compare("USEOLDZIPFINTERVAL"))
			USEOLDZIPFINTERVAL = atoi(val.c_str());
		if(!type.compare("MEAN"))
			MEAN = atof(val.c_str());
		if(!type.compare("STDDEV"))
			STDDEV = atof(val.c_str());
		if(!type.compare("USEOLDZIPFINTERVALT"))
			USEOLDZIPFINTERVALT = atoi(val.c_str());
		if(!type.compare("MEANT"))
			MEANT = atof(val.c_str());
		if(!type.compare("STDDEVT"))
			STDDEVT = atof(val.c_str());
		if(!type.compare("VARYINT"))
			VARYINT = atoi(val.c_str());
		if(!type.compare("VARYFS"))
			VARYFS = atoi(val.c_str());
		if(!type.compare("VARYDBS"))
			VARYDBS = atoi(val.c_str());
		if(!type.compare("VARYPART"))
			VARYPART = atoi(val.c_str());
		if(!type.compare("SKIPBL"))
			SKIPBL = atoi(val.c_str());
		if(!type.compare("PARTITIONS"))
			PARTITIONS = atoi(val.c_str());
		if(!type.compare("PARTITION_MODE"))
			PARTITION_MODE = atoi(val.c_str());
		if(!type.compare("REPL_POLICY"))
			REPL_POLICY = atoi(val.c_str());

	}

	cfg.close();
}

void verify_config()
{

	//assert(VARYFS);
	assert(DBDOMAIN == closestPower2(DBDOMAIN)-1);
	assert(DBSIZE>0);
	assert(DBDOMAIN>0);
	/* We merge only empty leaves */
#ifndef ONLYEMPTY
	assert(1==0)
#endif

/*#ifndef MERGE
	assert(1==0);
#endif
*/

}

void print_config()
{

	cout<<"--------------SETTINGS--------------"<<endl;
	cout<<"Domain: "<<DBDOMAIN<<endl;
	cout<<"Size of cold store: "<<DBSIZE<<endl;
	cout<<"Range/Bloom filter size: "<<RANGEFILTERSIZE<<endl;
	cout<<"outpt file: "<<OUTPUTFILE<<endl;
	cout<<"MEAN :"<<MEAN<<endl;
	cout<<"POINTQUERIES?"<<JUSTPOINTQUERIES<<endl;
	cout<<"VARY FILTER SIZE? ="<<VARYFS<<endl;
	cout<<"VARY INTERVAL SIZE? ="<<VARYINT<<endl;
	cout<<"Zipfdata? "<<ZIPFDATA<<endl;
	cout<<"Zipfqueries? "<<ZIPFQUERIES<<endl;



	cout<<"------------------------------------"<<endl;
}

void getTimes(vector<double> &times,double * avg, double * avg90, double * maxx)
{
	if(times.size() == 0)
		return;
	double factor  = 1000000;
	sort(times.begin(),times.end());
	//assert(times.size() == EXPQUERIES);
	*maxx = factor*times.back();
	*avg90 = factor*times[0.9 * times.size()];
	double sum = 0;
	for(int i=0;i<times.size();i++)
		sum+=times[i];
	*avg = factor*sum/times.size();
	return;

}



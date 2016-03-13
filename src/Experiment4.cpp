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

  if(argc<2)
  {
	cout<<"No config file specified. Exiting.."<<endl;
	return -1;
  }
  parse_args(argv[1]); //parses config file
  verify_config();
  cout<<"Experiment 4 (measuring workload change behavior) started.."<<endl;


  // populate and define database
  Database db(DBDOMAIN+1);
  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
  Uniform unif(DBDOMAIN+1);

  db.populate(DBSIZE, &zipf);

  db.plot();




  SynopsisIntClock synStart(DBDOMAIN,RANGEFILTERSIZE,true,&db);
  synStart.init();
  synStart.perfect(&db);

  SynopsisIntClock synStartBase(DBDOMAIN,RANGEFILTERSIZE,true,&db);
  synStartBase.init();
  synStartBase.perfect(&db);

  Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);

  Synopsis0bit syn0adapt(DBDOMAIN, RANGEFILTERSIZE, true,&db);
  syn0adapt.init();
  syn0adapt.perfect(&db);


  Query qTrain1(DBDOMAIN, 0,300,100,
		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
   qTrain1.generateHotZipf(ZIPFQUERIESFACTOR,300,100);
   if(!ZIPFCOLD)
	   qTrain1.generateUniform(300,100);
  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();
    bool qR = db.rangeQuery(q);
    synStart.handleQuery(q,&db,true,qR);
    syn0adapt.handleQuery(q,&db,true,qR);

  }

 Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
  qTrain2.generateHotZipf(ZIPFQUERIESFACTOR,MEANT,STDDEVT);
  if(!ZIPFCOLD)
	  qTrain2.generateUniform(MEANT,STDDEVT);
  for (int i=0; i<2*TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain2.nextQuery();
    bool qR = db.rangeQuery(q);
    synStart.handleQuery(q,&db,true,qR);
    syn0adapt.handleQuery(q,&db,true,qR);
  }





  Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);
/* train baseline with exact workload */
  qGen.generateHotZipf(ZIPFQUERIESFACTOR,MEAN,STDDEV);
  if(!ZIPFCOLD)
	  qGen.generateUniform(MEAN,STDDEV);



  for (int i=0; i<EXPQUERIES; i++) {
      Query::Query_t q = qGen.nextQuery();
      bool qR = db.rangeQuery(q);

      synStartBase.handleQuery(q,&db,true,qR);

    }

  SynopsisIntClock synStart0(synStart);


  synStart.truncateClock(RANGEFILTERSIZE);

  synStartBase.truncateClock(RANGEFILTERSIZE);
  synStart0.truncateClock(4*RANGEFILTERSIZE/3);
  syn0adapt.truncateClock(RANGEFILTERSIZE);

  cout<<"SynStart0 nodes:"<<synStart0.nodes()<<endl;
  cout<<"SynStart nodes:"<<synStart.nodes()<<endl;

  synStart.resetTime();
  synStartBase.resetTime();
  synStart0.resetTime();

  synStart.convertToCompact(1);
  synStart0.convertToCompact(0);
  synStartBase.convertToCompact(1);

  Synopsis syn1adapt(synStart);
  syn1adapt.set(1);
  cout<<"Leaves of 1-bit: "<<syn1adapt.Numleaves()<<endl;
  cout<<"NODES of 1-bit: "<<syn1adapt.nodes()<<endl;
  /*Synopsis syn0adapt(synStart0);
  syn0adapt.set(0);
  cout<<"Leaves of 0-bit: "<<syn0adapt.Numleaves()<<endl;
  cout<<"NODES of 0-bit: "<<syn0adapt.nodes()<<endl;*/

  Synopsis syn1(synStart);
  syn1.set(1);
  Synopsis syn0(synStart0);
  syn0.set(0);

  Synopsis baseadapt(synStartBase);
  baseadapt.set(1);

  Synopsis base(synStartBase);
  base.set(1);



  cout<<"--------------END OF TRAINING----------------------"<<endl;
  FILE * results = fopen(OUTPUTFILE,"a");
  fprintf(results,"upd\t base\t baseA\t syn0\t syn0A\t syn1\t syn1A\t bloom\t trueneg \n");

  fflush(results);
  fclose(results);

  /* Real experiment - Tries adapt */

  cout<<"upd\t base0\t\t synNoTp\t syn0\t\t syn1\t\t bloom\t"<<endl;
  int fpBloom = 0;
  int tp = 0;
  int tn = 0;

  for(int j =0;j<2;j++)
  {
  // --- RESET queries --- //

  qGen.reset();
    for (int i=0; i<EXPQUERIES; i++)
   {

		Query::Query_t q = qGen.nextQuery();
		//cout<<q.left<<"-"<<q.right<<endl;

		if(JUSTPOINTQUERIES)
			q.right=q.left;

		bool qR = db.rangeQuery(q);

		if(qR)
		  tp++;
		else
		  tn++;

 		syn0.handleQuery(q,&db,false,qR);
 		syn1.handleQuery(q,&db,false,qR);
 		base.handleQuery(q,&db,false,qR);

 		syn0adapt.handleQuery(q,&db,true,qR);
		syn1adapt.handleQuery(q,&db,true,qR);
		baseadapt.handleQuery(q,&db,true,qR);

 	    syn0.truncateClock(RANGEFILTERSIZE);
 	    syn1.truncateClock(RANGEFILTERSIZE);
 	    base.truncateClock(RANGEFILTERSIZE);
 	    syn0adapt.truncateClock(RANGEFILTERSIZE);
	    syn1adapt.truncateClock(RANGEFILTERSIZE);
	    baseadapt.truncateClock(RANGEFILTERSIZE);

		bool sR;
		if(JUSTPOINTQUERIES)
			sR = bl.pointQuery(q.left);
		else
			sR = bl.rangeQuery(q);

		if(sR && !qR)
			 fpBloom++;


  /* output results to file */

  if(i%1000 ==0)
  {
	    //cout<<"Writing to fiel: "<<OUTPUTFILE<<endl;
		FILE * results = fopen(OUTPUTFILE,"a");

		int pt = (j*EXPQUERIES) + i;

	 	  fprintf(results,"%d\t %d\t %d\t %d\t %d\t %d\t %d\t %d\t %d\n",
	 	    			pt,base.fp,baseadapt.fp,
	 	    			syn0.fp,syn0adapt.fp,
	 	    			syn1.fp,syn1adapt.fp,
	 	    			fpBloom,tn);


		fclose(results);
		assert(base.size()<=RANGEFILTERSIZE);
		assert(syn1.size()<=RANGEFILTERSIZE);
		assert(syn0.size()<=RANGEFILTERSIZE);
		syn1.resetFp();
	   syn0.resetFp();
	   syn1adapt.resetFp();
	   syn0adapt.resetFp();
	   base.resetFp();
	   baseadapt.resetFp();
	   fpBloom = 0;
	   tp = 0;
	   tn = 0;
  }

} /* end of one round of updates */
   if(ZIPFCOLD)
   {
	   qGen.reset();
   	   //qGen.generateHotZipf(ZIPFQUERIESFACTOR,MEAN,STDDEV);
   }
   else
   {


	   qGen.reset();
	   //qGen.generateUniform(MEAN,STDDEV);
   }



  } //end of all updates


  cout<<"--------END OF BENCHMARK 4--------"<<endl;

  free(OUTPUTFILE);

  /*
  double base1_avg,syn1_avg,syn0_avg,base1_lavg,syn1_lavg,syn0_lavg;

  base1_avg = (base1.getQueryTime(ADAPT)+ base1.getQueryTime(TRUNCATE));
  syn0_avg = (syn0.getQueryTime(ADAPT)+ syn0.getQueryTime(TRUNCATE));
  syn1_avg = (syn1.getQueryTime(ADAPT)+ syn1.getQueryTime(TRUNCATE));

  base1_lavg = base1.getQueryTime(LOOKUP);
  syn1_lavg = syn1.getQueryTime(LOOKUP);
  syn0_lavg = syn0.getQueryTime(LOOKUP);


  FILE * time = fopen("time_results.txt","a");



  	  fprintf(time,"%.10f %.10f %.10f %.10f %.10f %.10f \n",
  	    			base1_avg,
  	    			syn0_avg,
  	    			syn1_avg,
  	    			base1_lavg,
  	    			syn0_lavg,
  	    			syn1_lavg);


    fclose(time);*/

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
		if(!type.compare("ZIPFCOLD"))
					ZIPFCOLD = atoi(val.c_str());

	}

	cfg.close();
}

void verify_config()
{

	//assert(VARYFS);
	assert(DBDOMAIN == closestPower2(DBDOMAIN)-1);
	assert(DBSIZE>0);
	assert(DBDOMAIN>0);
	assert(DOUPDATES);
	assert(UPDATEBATCH>0);
	/* We merge only empty leaves */
#ifndef ONLYEMPTY
	assert(1==0);
#endif

	/*

#ifndef MERGE
	assert(1==0)
#endif*/


}

void print_config()
{

	cout<<"--------------SETTINGS--------------"<<endl;
	cout<<"Domain: "<<DBDOMAIN<<endl;
	cout<<"Size of cold store: "<<DBSIZE<<endl;
	cout<<"Range/Bloom filter size: "<<RANGEFILTERSIZE<<endl;
	cout<<"outpt file: "<<OUTPUTFILE<<endl;
	cout<<"MEAN :"<<MEAN<<endl;
	cout<<"VARY FILTER SIZE? ="<<VARYFS<<endl;
	cout<<"VARY INTERVAL SIZE? ="<<VARYINT<<endl;
	cout<<"Zipfdata? "<<ZIPFDATA<<endl;
	cout<<"Zipfqueries? "<<ZIPFQUERIES<<endl;
	cout<<"Zipf Cold to ZipfHot?"<<ZIPFCOLD<<endl;



	cout<<"------------------------------------"<<endl;
}




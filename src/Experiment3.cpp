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
  cout<<"Experiment 3 (measuring update behavior) started.."<<endl;


  // populate and define database
  Database db(DBDOMAIN+1);
  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
  Uniform unif(DBDOMAIN+1);
  if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf);
  else
	db.populate(DBSIZE, &unif);
 // print_config();
  db.plot();


  SynopsisIntClock synStart(DBDOMAIN,RANGEFILTERSIZE,true,&db);
  synStart.init();
  synStart.perfect(&db);

  SynopsisIntClock synStartBase(DBDOMAIN,RANGEFILTERSIZE,true,&db);
  synStartBase.init();
  synStartBase.perfect(&db);

  Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);


  Query qTrain1(DBDOMAIN, 0,300,100,
		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();
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





  Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);
/* train baseline with exact workload */



  for (int i=0; i<EXPQUERIES; i++) {
      Query::Query_t q = qGen.nextQuery();
      bool qR = db.rangeQuery(q);

      synStartBase.handleQuery(q,&db,true,qR);

    }

  Synopsis synStart0(synStart);
  synStart0.set(0);

  synStart.truncateClock(RANGEFILTERSIZE);

  synStartBase.truncateClock(RANGEFILTERSIZE);
  synStart0.truncateClock(RANGEFILTERSIZE);

  synStart.resetTime();
  synStartBase.resetTime();


  synStart.convertToCompact(1);
  synStartBase.convertToCompact(1);
  synStart0.convertToCompact(0);

  Synopsis syn1adapt(synStart);
  syn1adapt.set(1);
  cout<<"Leaves of 1-bit: "<<syn1adapt.Numleaves()<<endl;
  cout<<"NODES of 1-bit: "<<syn1adapt.nodes()<<endl;
  Synopsis syn0adapt(synStart0);
  syn0adapt.set(0);
  cout<<"Leaves of 0-bit: "<<syn0adapt.Numleaves()<<endl;
  cout<<"NODES of 0-bit: "<<syn0adapt.nodes()<<endl;

  Synopsis syn1(synStart);
  syn1.set(1);
  Synopsis syn0(synStart0);
  syn0.set(0);

  Synopsis baseadapt(synStartBase);
  baseadapt.set(1);

  Synopsis base(synStartBase);
  base.set(1);





  cout<<"--------------END OF TRAINING----------------------"<<endl;


  /* Real experiment - Tries adapt */
/*
  FILE * results = fopen(OUTPUTFILE,"a");

  fprintf(results,"upd\t base\t baseAdpt\t syn0\t syn0Adpt\t syn1\t syn1Adpt\t  bloom\t\n");
  fflush();
  fclose(results);*/

  cout<<"upd\t base\t\t baseAdpt\t syn0\t\t syn0Adpt\t\t syn1\t\t syn1Adpt\t\t  bloom\t"<<endl;
   int fpBloom = 0;
   int tp = 0;
   int tn = 0;


   int updates=0;
   for(int upd =0;upd<=DBSIZE;upd+=UPDATEBATCH)
   {
   // --- RESET STATS --- //
   syn1.resetFp();
   syn0.resetFp();
   syn1adapt.resetFp();
   syn0adapt.resetFp();
   base.resetFp();
   baseadapt.resetFp();
   fpBloom = 0;
   tp = 0;
   tn = 0;
   qGen.reset();
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
     }



   /* output results to file */
   cout<<"Writing to "<<OUTPUTFILE<<endl;
   FILE * results = fopen(OUTPUTFILE,"a");



 	  fprintf(results,"%d %d %d %d %d %d %d %d %d\n",
 	    			upd,base.fp,baseadapt.fp,
 	    			syn0.fp,syn0adapt.fp,
 	    			syn1.fp,syn1adapt.fp,
 	    			fpBloom,tn);


   cout<<upd<< "\t, "<<base.fp<<"\t\t"
		   <<baseadapt.fp<<"\t\t, "<<
		   syn0.fp << "\t\t, " <<syn0adapt.fp << "\t\t, "
		   << syn1.fp << "\t\t, " << syn1adapt.fp << "\t\t, " << fpBloom << "\t\t, "
 		  << tp << "\t\t, "<< tn << "\t\t, "<<  endl;
   fclose(results);
   int prev_size = db.size();
   /* new data */

   uint* newKeys = NULL;

   if(ZIPFUPDATES)
 	 newKeys = db.addKeys(UPDATEBATCH,&zipf);
   else
 	  newKeys = db.addKeys(UPDATEBATCH,&unif);

   base.recordNewKeys(newKeys, UPDATEBATCH);
   syn0.recordNewKeys(newKeys, UPDATEBATCH);
   syn1.recordNewKeys(newKeys, UPDATEBATCH);

   baseadapt.recordNewKeys(newKeys, UPDATEBATCH);
   syn0adapt.recordNewKeys(newKeys, UPDATEBATCH);
   syn1adapt.recordNewKeys(newKeys, UPDATEBATCH);

   bl.recordNewKeys(newKeys, UPDATEBATCH);
   free(newKeys);
   db.deleteKeys(UPDATEBATCH);
   assert(db.size()==prev_size);
   db.sanity_check();

   } //end of all updates

   cout<<"--------END OF BENCHMARK 3--------"<<endl;

   free(OUTPUTFILE);

   /* Tieming rezolts */

   FILE * time = fopen("results_time.txt","a");
   FILE * time2 = fopen("results_time2.txt","a");


       //fprintf(time,"Adapt Bl\t AdaptSyn0\t Adapt Syn1 LookupBl \t Lookup Syn0\t Lookup Syn1\t\n");

   uint64_t base1_avg,syn0_avg,syn1_avg,bloom_avg,bloom90,syn190,syn090,base190;

   process_time(baseadapt.truncatet,base1_avg,base190);
   process_time(syn0adapt.truncatet, syn0_avg,syn090);
   process_time(syn1adapt.truncatet, syn1_avg,syn190);

      fprintf(time,"Base\t Syn0\t Syn1\t \n");
   	  fprintf(time,"%llu %llu %llu \n",
   	    			base1_avg,
   	    			syn0_avg,
   	    			syn1_avg);

   process_time(baseadapt.falsepos, base1_avg,base190);
   process_time(syn0adapt.falsepos, syn0_avg,syn090);
   process_time(syn1adapt.falsepos, syn1_avg,syn190);
   	   	   fprintf(time2,"Base\t Syn0\t Syn1\t \n");
   	   	  fprintf(time2,"%llu %llu %llu %d \n",
   	   	    			base1_avg,
   	   	    			syn0_avg,
   	   	    			syn1_avg,tn);


     fclose(time);
     fclose(time2);

   print_config();
   return 0;



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
		if(!type.compare("ZIPFUPDATES"))
				ZIPFUPDATES = atoi(val.c_str());

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
#ifdef DOTRACE
	assert(1==0);
#endif

#ifdef SYNDEBUG
	assert(1==0);
#endif

/*#ifndef MERGE
	assert(1==0)
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
	cout<<"VARY FILTER SIZE? ="<<VARYFS<<endl;
	cout<<"VARY INTERVAL SIZE? ="<<VARYINT<<endl;
	cout<<"Zipfdata? "<<ZIPFDATA<<endl;
	cout<<"Zipfqueries? "<<ZIPFQUERIES<<endl;
	cout<<"Zipf updates:"<<ZIPFUPDATES<<endl;



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



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
#include <sched.h>
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
  cout<<"Experiment 0 - tracing stuff..."<<endl;
  cpu_set_t  mask;
   	CPU_ZERO(&mask);
   	CPU_SET(0, &mask);
   	int result = sched_setaffinity(0, sizeof(mask), &mask);
   	assert(result == 0);


  // populate and define database
  Database db(DBDOMAIN+1);
  //FastDB db(DBDOMAIN +1);
  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
  Uniform unif(DBDOMAIN+1);
  if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf);
  else
	db.populate(DBSIZE, &unif);
 // print_config();
  db.plot();



  Synopsis syn0(DBDOMAIN, RANGEFILTERSIZE, true,NULL,CLOCK);
  Synopsis syn1(DBDOMAIN, RANGEFILTERSIZE,true,NULL,CLOCK);


  syn0.init();
  syn0.setDatabase(&db);

  syn0.perfect(&db);
  syn0.takeSnapshot();
  cout<<"took snapshot"<<endl;
  int num =atoi(argv[2]);
  syn0.takeSnapshotPartial(num);
  //syn1.init();
  //syn1.perfect(&db);

/*
  Query::Query_t q;
  q.left = 1;
  q.right = 9;
  //uint64_t time0 = rdtscp();

  syn0.learn_from_fp(q);
//uint64_t time1 = rdtscp();
  //cout<<"Clock cycles for learning from fp:"<<time1-time0<<endl;
 // time0=rdtscp();
  q.right=52;
  syn0.learn_from_fp(q);
 // time1 = rdtscp();
   // cout<<"Clock cycles for learning from fp:"<<time1-time0<<endl;

  return 0;
*/

  Query qTrain1(DBDOMAIN, 0,MEANT,STDDEVT,
		  ZIPFQUERIESFACTOR, TRAININGQUERIES,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();
    bool qR = db.rangeQuery(q);
    syn0.handleQuery(q,&db,true,qR);
   // syn1.handleQuery(q,&db,true,qR);


  }


  syn0.truncateClock(RANGEFILTERSIZE);
  syn0.sanityCheckUsed();
  //syn1.truncateClock(RANGEFILTERSIZE);
  //synKnowDb1.truncateClock(RANGEFILTERSIZE);
  //synKnowDb0.truncateClock(RANGEFILTERSIZE);
 return 0;
}

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

	/* We merge only empty leaves */
#ifndef ONLYEMPTY
	assert(1==0)
#endif

/*#ifndef DOTRACE
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




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
#include <sched.h>
#include <sstream>
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
bool USEOLDZIPFINTERVALT;
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
  print_config();
  cout<<"Experiment 5 (measuring partition accuracy) started.."<<endl;

  cpu_set_t  mask;
  	CPU_ZERO(&mask);
  	CPU_SET(0, &mask);
  	int result = sched_setaffinity(0, sizeof(mask), &mask);
  	assert(result == 0);


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



  assert(RANGEFILTERSIZE == 3000 || RANGEFILTERSIZE == 1000);

  MultiTrie multi(PARTITIONS,0,DBSIZE,DBDOMAIN, RANGEFILTERSIZE,&db,CLOCK,-1);
  MultiTrie multi0(PARTITIONS,0,DBSIZE,DBDOMAIN, RANGEFILTERSIZE,&db,CLOCK,-1);
  MultiTrie multi1(PARTITIONS,0,DBSIZE,DBDOMAIN, RANGEFILTERSIZE,&db,CLOCK,-1);

  /* TODO: Make a non-partitioned adaptive trie, see what happens */

  Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);

int a = 3;
Query qTrain1(DBDOMAIN,0,MEAN,STDDEV,ZIPFQUERIESFACTOR,TRAININGQUERIES/3,ZIPFQUERIES);

  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();

    if(JUSTPOINTQUERIES)
    			q.right=q.left;
    bool qR = db.rangeQuery(q);
    multi.handleQuery(q,&db,true);
    multi1.handleQuery(q,&db,true);
    multi0.handleQuery(q,&db,true);


   // syn1.handleQuery(q,&db,true,qR);
  }
  cout<<"training 2"<<endl;
  Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
		  ZIPFQUERIES,false);
  for (int i=0; i<2*TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain2.nextQuery();
    bool qR = db.rangeQuery(q);
   // syn1.handleQuery(q,&db,true,qR);
    multi.handleQuery(q,&db,true);
    multi1.handleQuery(q,&db,true);
    multi0.handleQuery(q,&db,true);


  }
  qTrain2.reset();




 // cout<<"0bit: multi: "<<multi0.size()<<" "<<syn0.size()<<endl;
  cout<<"1bit: multi: "<<multi.size()<<endl;

  multi.convert(0);
  multi.set(0);
  multi.truncateClock(RANGEFILTERSIZE);

  multi0.convert(0);
  multi0.set(0);

  multi0.truncateClock(RANGEFILTERSIZE);

  multi1.convert(1);
  multi1.set(1);
  multi1.sanityCheckUsed();

 // cout<<"---paseed firset---"<<endl;
  multi1.truncateClock(RANGEFILTERSIZE);
  multi1.convert(1);


 // multi.convert(0);


/*
  cout<<"Multi trie size:"<<multi.size()<<endl;

  cout<<"Multi trie0 size:"<<multi0.size()<<endl;

   cout<<"Multi trie1 size:"<<multi1.size()<<endl;

   cout<<"Multi trie leafe:"<<multi.leaves()<<endl;

     cout<<"Multi trie0 leadf:"<<multi0.leaves()<<endl;

      cout<<"Multi trie1 leaf:"<<multi1.leaves()<<endl;

      cout<<"Multi trie ndoes:"<<multi.nodes()<<endl;

          cout<<"Multi trie0 leadf:"<<multi0.leaves()<<endl;

           cout<<"Multi trie1 nodes:"<<multi1.nodes()<<endl;
*/

        /*   multi.sanityCheckUsed();
           multi1.sanityCheckUsed();
           multi0.sanityCheckUsed();*/
//return 0;


  // --- RESET STATS --- //


  multi0.resetTime();

  multi1.resetTime();

  multi.resetTime();


  cout<<"--------------!END OF TRAINING----------------------"<<endl;

  /* Real experiment - Tries are frozen (no adaptation) */

  Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);
   int tp = 0;
   int tn = 0;
   int fpBloom = 0;

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


		int b1 = multi.handleQuery(q,&db,false);
		bool m1 = multi1.handleQuery(q,&db,true);
		multi1.truncateClock(RANGEFILTERSIZE);
		bool m0 = multi0.handleQuery(q,&db,true);

		/*if(m1 == m0)
		{
			if(!qR && m1)
			{
				cout<<"both had a false positive"<<endl;
				cout<<"adapt time for 0:"<<multi0.adaptt.front()<<endl;
				cout<<"adapt time for 1:"<<multi1.adaptt.front()<<endl;
			}
		}*/

		multi0.truncateClock(RANGEFILTERSIZE);
		bool sR;
		if(JUSTPOINTQUERIES)
			sR = bl.pointQuery(q.left);
		else
			sR = bl.rangeQuery(q);

		if(sR && !qR)
			 fpBloom++;


    }
  /* end of benchmark */
  cout<<"--------END OF BENCHMARK 5 (PARTITIONED TRIES)--------"<<endl;


  stats time = calculate(list2vector(multi.lookupt));
  stats timeb = calculate(list2vector(bl.lookupt));
/*
  stats time1 = calculate(fptime(multi1.adaptt,multi1.truncatet));
  stats time0 = calculate(fptime(multi0.adaptt,multi0.truncatet));
*/
  stats time1a = calculate(list2vector(multi1.adaptt));
  stats time1t = calculate(list2vector(multi1.truncatet));

  stats time0a = calculate(list2vector(multi0.adaptt));
   stats time0t = calculate(list2vector(multi0.truncatet));



  FILE * results = fopen(OUTPUTFILE,"a");

    cout<<"wirting to file"<<endl;
  	fprintf(results,"%d %d %d  %llu %llu  %llu %llu  %llu %llu  %llu %llu  %llu %llu %llu %llu\n",
  			PARTITIONS,multi.getFp(),fpBloom,
  			time.avg,time.std,
  			timeb.avg,timeb.std,
  			time0a.avg,time0a.std,
  			time0t.avg,time0t.std,
  			time1a.avg,time1a.std,
  			time1t.avg,time1t.std);






  fclose(results);
  free(OUTPUTFILE);

  print_config();

  cout<<"Byeee!"<<endl;
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
#ifndef TICKTOCK /* we need the tick tock thing to measure the tries */
	assert(1==0);
#endif
/*#ifdef SYNDEBUG
	assert(1==0);
#endif*/

/*#ifndef ONLYEMPTY
	assert(1==0)
#endif
*/
	/*
#ifdef MERGE
	assert(1==0);
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
	cout<<"Just point queries? "<<JUSTPOINTQUERIES<<endl;
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



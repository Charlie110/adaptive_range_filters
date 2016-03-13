/*
 * Experiment5.cpp
 *
 *  Created on: Oct 4, 2012
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
  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
  Uniform unif(DBDOMAIN+1);
  if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf);
  else
	db.populate(DBSIZE, &unif);
 // print_config();
  db.plot();



  Synopsis0bit syn0(DBDOMAIN, RANGEFILTERSIZE, true,NULL);
  Synopsis syn1(DBDOMAIN, RANGEFILTERSIZE,true,NULL,CLOCK);
  //MultiTrie multi0(PARTITIONS,0,DBSIZE,DBDOMAIN, RANGEFILTERSIZE,&db,RANDOM,0);
  MultiTrie multi1(PARTITIONS,0,DBSIZE,DBDOMAIN, RANGEFILTERSIZE,&db,CLOCK,1);
  //syn0.init();
  syn1.setDatabase(&db);
  syn1.init();




  // Train syn0 & syn1 with 1/3 large queries (median = 300)
  // & 2/3 very large queries (median =3000)


  Query qTrain1(DBDOMAIN, 0,300,100,
		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
		  ZIPFQUERIES,false);
  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();
    //cout<<i<<endl;
    //cout<<q.left<<"-"<<q.right<<endl;
    bool qR = db.rangeQuery(q);
   // syn0.handleQuery(q,&db,true,qR);
    syn1.handleQuery(q,&db,true,qR);
    //multi0.handleQuery(q,&db,true);
    multi1.handleQuery(q,&db,true);

  }

  Query qTrain2(DBDOMAIN, 0,3000,1000,
		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
		  ZIPFQUERIES,false);
  for (int i=0; i<2*TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain2.nextQuery();
    bool qR = db.rangeQuery(q);
    //cout<<q.left<<"-"<<q.right<<endl;
    //int a =syn0.handleQuery(q,&db,true,qR);
    int a1 = syn1.handleQuery(q,&db,true,qR);
    //int b =multi0.handleQuery(q,&db,true);
    int b1 = multi1.handleQuery(q,&db,true);



  }


 // cout<<"0bit: multi: "<<multi0.size()<<" "<<syn0.size()<<endl;
  cout<<"1bit: multi: "<<multi1.size()<<" "<<syn1.size()<<endl;



  //syn0.truncateClock(RANGEFILTERSIZE);
  syn1.truncateClock(RANGEFILTERSIZE);
 // multi0.truncateClock(RANGEFILTERSIZE);
  multi1.truncateClock(RANGEFILTERSIZE);



  // --- RESET STATS --- //
  syn1.resetTime();
 // syn0.resetTime();
 // multi0.resetTime();
  multi1.resetTime();


  cout<<"--------------END OF TRAINING----------------------"<<endl;

  /* Real experiment - Tries are frozen (no adaptation) */

  Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);
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

		//syn0.handleQuery(q,&db,false,qR);
		int a1 = syn1.handleQuery(q,&db,false,qR);
		//multi0.handleQuery(q,&db,false);
		int b1 = multi1.handleQuery(q,&db,false);
		 /*if(!a1 && b1)
		    {
			    string fa ="multi.txt";
			    string fb ="plain.txt";
			 	 cout<<"!"<<endl;
		    }*/

    }
  /* end of benchmark */
  cout<<"--------END OF BENCHMARK 5 (PARTITIONED TRIES)--------"<<endl;
  /* output results to file */

  long double multi1_avg,syn1_avg;

  multi1_avg = multi1.getQueryTime(LOOKUP);
  syn1_avg = syn1.getQueryTime(LOOKUP);

   cout<<"Multi1 avvg:"<<multi1.getQueryTime(LOOKUP)<<endl;

  FILE * results = fopen(OUTPUTFILE,"a");
  if(VARYPART)
  {

  	fprintf(results,"%d %d %d %d %d %.10Lf %.10Lf \n",
  			PARTITIONS,/*multi0.getFp()*/0,/*syn0.fp*/0,multi1.getFp(),
  			syn1.fp,multi1_avg,syn1_avg);
  }
  if(VARYFS)
  {
	  /* do things */
  }


  fclose(results);
  free(OUTPUTFILE);
  cout<<"\t multi0 \t syn0 \t multi1 \t\t syn1 \t true pos \t true neg \t avg len"<<endl;
  cout<< "\t, "<</*multi0.getFp()*/0<<"\t\t" <</*syn0.fp*/0<<"\t\t, "<< multi1.getFp() << "\t\t, "
	<< syn1.fp << "\t\t, "
		  << tp << "\t\t, "<< tn << "\t\t, "<< qGen.getAverageLength()<< endl;

  print_config();
  //multi1.printFps();
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



	cout<<"------------------------------------"<<endl;
}





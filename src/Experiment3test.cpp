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

bool ADAPTTRIE = false;
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



  //Synopsis0bit syn0(DBDOMAIN, RANGEFILTERSIZE, true,NULL);
  Synopsis syn1(DBDOMAIN, RANGEFILTERSIZE,true,NULL,CLOCK);
  //Synopsis //synNo(DBDOMAIN, RANGEFILTERSIZE,false,NULL,CLOCK);

  //Synopsis //base1(DBDOMAIN, RANGEFILTERSIZE, true,&db,CLOCK);

  Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);

  //syn0.init();
  syn1.init();
  syn1.setDatabase(&db);
  //syn0.perfect(&db);
  //syn1.perfect(&db);

  //synNo.init();
  //base1.init();




  // Train //syn0 & syn1 with 1/3 large queries (median = 300)
  // & 2/3 small queries (median =30)
  // Train //syn0 & syn1 with 1/3 large queries (median = 300)
  // & 2/3 small queries (median =30)


  Query qTrain1(DBDOMAIN, 0,300,100,
		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();
    bool qR = db.rangeQuery(q);
    //syn0.handleQuery(q,&db,true,qR);
    syn1.handleQuery(q,&db,true,qR);
    //synNo.handleQuery(q,&db,true,qR);

  }

  Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
  for (int i=0; i<2*TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain2.nextQuery();
    bool qR = db.rangeQuery(q);
    //syn0.handleQuery(q,&db,true,qR);
    syn1.handleQuery(q,&db,true,qR);
    //synNo.handleQuery(q,&db,true,qR);

  }




  //syn0.truncateClock(RANGEFILTERSIZE);
  syn1.truncateClock(RANGEFILTERSIZE);
  //synNo.truncateClock(RANGEFILTERSIZE);

  Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);
/* train baseline with exact workload */



  for (int i=0; i<EXPQUERIES; i++) {
      Query::Query_t q = qGen.nextQuery();
      bool qR = db.rangeQuery(q);

      //base1.handleQuery(q,&db,true,qR);

    }

  //base1.truncateClock(RANGEFILTERSIZE);



  cout<<"--------------END OF TRAINING----------------------"<<endl;

  /* Real experiment - Tries adapt */

  cout<<"upd\t base0\t\t //synNoTp\t //syn0\t\t syn1\t\t bloom\t"<<endl;
  int fpBloom = 0;
  int tp = 0;
  int tn = 0;

  syn1.resetTime();
  //syn0.resetTime();
  //synNo.resetTime();
  //base1.resetTime();

  int updates=0;
  for(int upd =0;upd<=DBSIZE;upd+=UPDATEBATCH)
  {
  // --- RESET STATS --- //
  syn1.resetFp();
  //syn0.resetFp();
  //synNo.resetFp();
  //base1.resetFp();
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

		//syn0.handleQuery(q,&db,true,qR);
		syn1.handleQuery(q,&db,ADAPT,qR);
		//synNo.handleQuery(q,&db,true,qR);
		//base1.handleQuery(q,&db,true,qR);

	    //syn0.truncateClock(RANGEFILTERSIZE);
	    syn1.truncateClock(RANGEFILTERSIZE);
	    //synNo.truncateClock(RANGEFILTERSIZE);
	    //base1.truncateClock(RANGEFILTERSIZE);

		bool sR;
		if(JUSTPOINTQUERIES)
			sR = bl.pointQuery(q.left);
		else
			sR = bl.rangeQuery(q);

		if(sR && !qR)
			 fpBloom++;
		if(i % 100 == 0)
		{
			vector< pair<int,int> > v;
			syn1.takeSnapshot(v,TRACEEVAL);
			cout<<"Score: "<<syn1.getScore(0,DBDOMAIN,&db)<<endl;
		}
    }



  /* output results to file */
  cout<<"Writing to "<<OUTPUTFILE<<endl;
  FILE * results = fopen(OUTPUTFILE,"a");
/*


	  fprintf(results,"%d %d %d %d %d %d \n",
	    			upd,//base1.fp,
	    			//syn0.fp,syn1.fp,
	    			fpBloom,//synNo.fp);*/

  cout<<upd<< "\t, "<<0<<"\t\t" <<0<<"\t\t, "<< 0 << "\t\t, "
  << syn1.fp << "\t\t, " << fpBloom << "\t\t, "
		  << tp << "\t\t, "<< tn << "\t\t, "<<  endl;
  fclose(results);
  int prev_size = db.size();
  /* new data */
  db.deleteKeys(UPDATEBATCH);
  uint* newKeys = NULL;

  if(ZIPFUPDATES)
	 newKeys = db.addKeys(UPDATEBATCH,&zipf);
  else
	  newKeys = db.addKeys(UPDATEBATCH,&unif);

  //base1.recordNewKeys(newKeys, UPDATEBATCH);
  //syn0.recordNewKeys(newKeys, UPDATEBATCH);
  syn1.recordNewKeys(newKeys, UPDATEBATCH);
  //synNo.recordNewKeys(newKeys, UPDATEBATCH);
  bl.recordNewKeys(newKeys, UPDATEBATCH);
  free(newKeys);
  //assert(db.size()==prev_size);
  db.sanity_check();

  } //end of all updates

  cout<<"--------END OF BENCHMARK 3--------"<<endl;

  free(OUTPUTFILE);

/*
  double //base1_avg,syn1_avg,//syn0_avg,//base1_lavg,syn1_lavg,//syn0_lavg;

  //base1_avg = (//base1.getQueryTime(ADAPT)+ //base1.getQueryTime(TRUNCATE))/2;
  //syn0_avg = (//syn0.getQueryTime(ADAPT)+ //syn0.getQueryTime(TRUNCATE))/2;
  syn1_avg = (syn1.getQueryTime(ADAPT)+ syn1.getQueryTime(TRUNCATE))/2;

  //base1_lavg = //base1.getQueryTime(LOOKUP);
  syn1_lavg = syn1.getQueryTime(LOOKUP);
  //syn0_lavg = //syn0.getQueryTime(LOOKUP);


  FILE * time = fopen("results_time.txt","a");


      fprintf(time,"Adapt Bl\t Adapt//syn0\t Adapt Syn1 LookupBl \t Lookup //syn0\t Lookup Syn1\t\n");

  	  fprintf(time,"%.10f %.10f %.10f %.10f %.10f %.10f \n",
  	    			//base1_avg,
  	    			//syn0_avg,
  	    			syn1_avg,
  	    			//base1_lavg,
  	    			//syn0_lavg,
  	    			syn1_lavg);


    fclose(time);
*/
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
		if(!type.compare("ADAPTTRIE"))
				ADAPTTRIE = atoi(val.c_str());

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
	assert(1==0)
#endif

#ifndef MERGE
	assert(1==0)
#endif


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



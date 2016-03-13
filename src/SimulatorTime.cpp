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
#define CFG_PARAMS 24
//EXP MODE//
bool VARYINT = false; // vary interval size
bool VARYFS = false; //vary filter size
bool SKIPBL = false;


//  Database Parameters
uint DBDOMAIN;  // domain of the keys of the database
uint DBSIZE;    // number of keys in the cold store of the database 
bool ZIPFDATA;
double ZIPFDATAFACTOR;  // zipf factor for database

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
  cout<<"Simulation started..."<<endl;
  
 
  // populate and define database
  Database db(DBDOMAIN+1);
  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
  Uniform unif(DBDOMAIN+1);
  if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf); 
  else
	db.populate(DBSIZE, &unif); 
  print_config();
  db.plot();
 
 
  Synopsis0bit syn(DBDOMAIN, RANGEFILTERSIZE, true,NULL);
  //Synopsis0bit syn2(DBDOMAIN, RANGEFILTERSIZE, true,NULL); //learn from tn, pointer to db
  Synopsis base(DBDOMAIN, RANGEFILTERSIZE, true,&db,CLOCK); //does baseline learn from true negs?
  Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);
  //MultiTrie multi(PARTITIONS,DBDOMAIN, RANGEFILTERSIZE,&db);
  double fprate = bl.fpRate();
  syn.init();
  base.init();
  cout<<"False positive rate of bloom filter is: "<<fprate<<endl;
  

  int tp = 0;  int tp2 = 0;         // true positives during training
  int tn = 0;  int tn2=0;         // true negatives during training
  int fp = 0;  int fp2 =0;
  int fpb =0; int tnb = 0; int tpb = 0;

  // run range queries
  Query qTrain(DBDOMAIN, MINRANGE,MEANT,STDDEVT, ZIPFQUERIESFACTOR, TRAININGQUERIES, ZIPFQUERIES,USEOLDZIPFINTERVALT);
  for (int i=0; i<TRAININGQUERIES; i++) {
    Query::Query_t q = qTrain.nextQuery(); 
    bool qR = db.rangeQuery(q);
   
    if(i % 100 == 0)
    {  	
	syn.truncateClock(RANGEFILTERSIZE);	
   }
	
    syn.handleQuery(q,&db,true,qR);
    if(!SKIPBL)
    	base.handleQuery(q,&db,true,qR);
     	
  }  



 cout<<"False positives: "<<syn.fp<<"("<<100.0*syn.fp/(TRAININGQUERIES+0.0)<<" %)"<<endl;
 cout<<"True positives: "<<syn.tp<<"("<<100.0*syn.tp/(TRAININGQUERIES+0.0)<<" %)"<<endl;
 cout<<"True negatives: "<<syn.tn<<"("<<100.0*syn.tn/(TRAININGQUERIES+0.0)<<" %)"<<endl;
 cout<<"Size of synopsis: "<<syn.size()<<endl;
 cout<<"Average length of intervals: "<<qTrain.getAverageLength()<<endl;
 //syn.exportGraphviz(OUTPUTFILE);
   
  cout<<"--------------END OF TRAINING----------------------"<<endl;

  // --- RESET STATS --- //
  syn.truncateClock(RANGEFILTERSIZE);
  syn.resetTime();
  base.resetTime();
    Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,USEOLDZIPFINTERVAL);
 
for (int i=0; i<EXPQUERIES; i++)
   {
    
      Query::Query_t q = qGen.nextQuery(); 
      if(JUSTPOINTQUERIES)
      	q.right=q.left; 
      bool qR = db.rangeQuery(q);     
    //  base.handleQuery(q,&db,true,qR);

    }
    qGen.reset();


 
  int fpBloom = 0; 
  int fpSyn = 0;
  int TIMELOOPS = 1;
  long double query,syn_sum,bloom_sum;
  query = 0;
  syn_sum = 0;
  bloom_sum = 0;
  for(int j=0;j <TIMELOOPS;j++)
{ 
 fpBloom = 0;
 fpSyn = 0;
  struct timeval start,end;
  


tick(start); // SYNOPSIS //
    for (int i=0; i<EXPQUERIES; i++)
   {
    
      Query::Query_t q = qGen.nextQuery(); 
      if(JUSTPOINTQUERIES)
      	q.right=q.left; 
      bool qR = db.rangeQuery(q);     
      bool sR = syn.handleQuery(q,&db,true,qR);
      syn.truncateClock(RANGEFILTERSIZE);
      //bool sR = syn.pointQuery(q.left);
      if(sR && !qR)
		fpSyn++;


    }
   
   syn_sum+= tock(start,end,false); 
   qGen.reset();   
   tick(start);

    for (int i=0; i<EXPQUERIES; i++) // BLOOM //
   {
    
      Query::Query_t q = qGen.nextQuery(); 
    
      if(JUSTPOINTQUERIES)
      	q.right=q.left; 

      bool qR = db.rangeQuery(q);
      bool sR = bl.rangeQuery(q);
      if(sR && !qR)
    	  	 fpBloom++;
   }
   bloom_sum+= tock(start,end,false);       
   qGen.reset();
   
   tick(start); // PLAIN LOOP //
	for (int i=0; i<EXPQUERIES; i++)
	{

		Query::Query_t q = qGen.nextQuery(); 

		if(JUSTPOINTQUERIES)
		q.right=q.left; 

		bool qR = db.rangeQuery(q);
		
	}
          
   query+= tock(start,end,false); 
  //write information about fp and stuff to file :)
}

  long double syn_avg,bloom_avg,syn_avg2;
 
  bloom_avg = 1000000.0 * (bloom_sum - query) /(EXPQUERIES*TIMELOOPS);
  syn_avg2 = 1000000.0 * (syn_sum - query) /(EXPQUERIES*TIMELOOPS);
  cout<<1000000 * (bloom_sum - query)<<endl;
  cout<<EXPQUERIES*TIMELOOPS<<endl;
  long double sum = 0.0;
  for(int i=0;i<syn.queryTimes.size();i++)
   sum+=syn.queryTimes[i];
  syn_avg = (1000000.0 * sum) / (EXPQUERIES*TIMELOOPS);
  cout<<"Vecotr size: "<<syn.queryTimes.size()<<endl;
  assert(syn.queryTimes.size() == (EXPQUERIES*TIMELOOPS));
  cout<<"---SYNOPSIS SIZE: "<<syn.size()<<endl;

  
  FILE * results = fopen(OUTPUTFILE,"a");
  if(VARYFS)
  {	
	
  	fprintf(results,"%d %d %d %d %.10Lf %.10Lf \n",RANGEFILTERSIZE,base.fp,fpSyn,fpBloom,syn_avg,bloom_avg);
	printf("%d %d %d %d %.10Lf %.10Lf\n",RANGEFILTERSIZE,base.fp,fpSyn,fpBloom,syn_avg,bloom_avg);
  }

  if(VARYINT)
  {
	
  	fprintf(results,"%f %d %d %d %.10Lf %.10Lf  %.10Lf \n",MEAN,base.fp,fpSyn,fpBloom,syn_avg,bloom_avg,syn_avg2);
	//printf("%d %d %d %d %.10Lf %.10Lf \n",RANGEFILTERSIZE,base.fp,fpSyn,fpBloom,syn_avg,bloom_avg);
  }
  

  fclose(results);
  print_config();

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
		if(!type.compare("SKIPBL"))
			SKIPBL = atoi(val.c_str());
		
	}
	
	cfg.close();
}

void verify_config()
{

	assert((VARYINT || VARYFS) && !(VARYINT && VARYFS));
	assert(DBSIZE>0);
	assert(DBDOMAIN>0);


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




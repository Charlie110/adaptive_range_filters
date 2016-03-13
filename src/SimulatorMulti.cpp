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
  cout<<"MultiTrie Simulation started..."<<endl;


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


  //Synopsis syn(DBDOMAIN, RANGEFILTERSIZE, true,NULL,CLOCK);
  Synopsis0bit syn(DBDOMAIN, RANGEFILTERSIZE, true,NULL);
  //Synopsis0bit ////syn2(DBDOMAIN, RANGEFILTERSIZE, true,NULL);
  //Synopsis0bit ////syn2(DBDOMAIN, RANGEFILTERSIZE, true,NULL); //learn from tn, pointer to db
  Synopsis base(DBDOMAIN, RANGEFILTERSIZE, true,&db,RANDOM); //does baseline learn from true negs?
  Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);
  MultiTrie multi(PARTITIONS,PARTITION_MODE,DBSIZE,DBDOMAIN, RANGEFILTERSIZE,&db,RANDOM,0);
  MultiTrie multi(PARTITIONS,PARTITION_MODE,DBSIZE,DBDOMAIN, RANGEFILTERSIZE,&db,RANDOM,0);
  double fprate = bl.fpRate();
  syn.init();
  base.init();
  ////syn2.init();
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
   	//cout<<"Query: "<<i<<endl;
	//cout<<"Range: "<<q.left<<","<<q.right<<endl;
	syn.truncateClock(RANGEFILTERSIZE);
	////syn2.truncateClock(RANGEFILTERSIZE);
	multi.truncateClock(RANGEFILTERSIZE);
	//assert(syn.size() == multi.tries[0]->size());
	//////syn2.truncateClock(RANGEFILTERSIZE);
   }

    syn.handleQuery(q,&db,true,qR);
    ////syn2.handleQuery(q,&db,true,qR);
    multi.handleQuery(q,&db,true);
   // cout<<syn.size()<<"vs "<<multi.size()<<endl;
    //getchar();

    //////syn2.handleQuery(q,&db,&tn2,&tp2,&fp2);
    if(!SKIPBL)
    	base.handleQuery(q,&db,true,qR);

  }
if(!NOTRUNCATE)
{
  syn.truncateClock(RANGEFILTERSIZE);
  multi.truncateClock(RANGEFILTERSIZE);
}


 cout<<"False positives: "<<syn.fp<<"("<<100.0*syn.fp/(TRAININGQUERIES+0.0)<<" %)"<<endl;
 cout<<"True positives: "<<syn.tp<<"("<<100.0*syn.tp/(TRAININGQUERIES+0.0)<<" %)"<<endl;
 cout<<"True negatives: "<<syn.tn<<"("<<100.0*syn.tn/(TRAININGQUERIES+0.0)<<" %)"<<endl;
 cout<<"Size of synopsis: "<<syn.size()<<endl;
 cout<<"Average length of intervals: "<<qTrain.getAverageLength()<<endl;
 //syn.exportGraphviz(OUTPUTFILE);

  cout<<"--------------END OF TRAINING----------------------"<<endl;

  // --- RESET STATS --- //
  syn.resetTime();
  base.resetTime();
  multi.resetTime();
  ////syn2.resetTime();
  // experimental result format
  cout << "updates\t, fpBase\t fpSyn\t,fpSynMulti\t fpBloom\t, true pos\t, true neg\t, avg len\t"  << endl;

  int fpBloom = 0;


    tp = 0;           // true positives
    tn = 0;           // true negatives

         // false positives of (poor man's) Bloom filter

    // qGen.reset();
    vector<int> fps_par(PARTITIONS);
    fill(fps_par.begin(),fps_par.end(),0);
    Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV, ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,USEOLDZIPFINTERVAL);
    for (int i=0; i<EXPQUERIES; i++)
   {

      Query::Query_t q = qGen.nextQuery();


     //cout<<"Query "<<i<<endl;
     //cout<<"Range: "<<q.left<<","<<q.right<<endl;
      if(JUSTPOINTQUERIES)
      	q.right=q.left;

      //cout<<"Left: "<<q.left<<endl;
      bool qR = db.rangeQuery(q);

      if(qR)
	tp++;
      else
	tn++;

      if(/*i%100 == 0 &&*/ !NOTRUNCATE)
	{
    	  syn.truncateClock(RANGEFILTERSIZE);
		  ////syn2.truncateClock(RANGEFILTERSIZE);
		  multi.truncateClock(RANGEFILTERSIZE);
      }

     bool a = syn.handleQuery(q,&db,false,qR);
      bool b =multi.handleQuery(q,&db,false);

      if(a && !qR)
      {
    	  for(int i=0;i<multi.bounds.size();i++)
    	  {
    		  if(q.left>=multi.bounds[i].low && q.left<=multi.bounds[i].high)
    		  {
    			  fps_par[i]++;
    		  	  break;
    		  }

    	  }

      }
      ////syn2.handleQuery(q,&db,true,qR);
      //a = syn.handleQuery(q,&db,true,qR);
      // b =multi.handleQuery(q,&db,true);
      if((b && !qR) && (!a && !qR))
      {

//    	  assert(1==0);
      }
     //assert((PARTITIONS ==1 && a==b) || PARTITIONS!=1); //sanity check :)
      if(!SKIPBL)
      	base.handleQuery(q,&db,true,qR);
     // else
	//syn.pointQuery(q.left);
      //////syn2.handleQuery(q,&db,&tnSyn,&tpSyn,&fp////syn2);



      bool sR = bl.rangeQuery(q);
      if(sR && !qR)
	fpBloom++;
    }
   // assert(1==0);

   cout<<"------------Truncating-----------"<<endl;
  syn.truncateClock(RANGEFILTERSIZE);
  ////syn2.truncateClock(RANGEFILTERSIZE);
  multi.truncateClock(RANGEFILTERSIZE);


    // report results
    cout<<0<< "\t, " <<base.fp<<"\t\t, "<< syn.fp << "\t\t, " /*syn2.fp << "\t\t, "*/<< multi.getFp() << "\t\t, " << fpBloom << "\t\t, " << tp << "\t\t, "<<tn << "\t\t, "
	 << qGen.getAverageLength()<< endl;

    cout<<"size of single trie :"<<syn.size()<<endl;
    cout<<"Leaves of single trie :"<<syn.Numleaves()<<endl;
    int sum = 0;
    for(int i=0;i<fps_par.size();i++)
    {
    	cout<<"partition "<<i<<" fps: "<<fps_par[i]<<endl;
    	sum+=fps_par[i];
    }
    assert(sum==syn.fp);
    cout<<"--- DETAILED MULTITRIE REPORT ----- "<<endl;
      multi.printFps();
      cout<<"-------------------------------------"<<endl;
      print_config();
      cout<<"Baseline size:"<<base.size()<<endl;
      cout<<"Baseline leaves:"<<base.Numleaves()<<endl;
     /* for(int i=0;i<multi.tries.size();i++)
      {
    	  stringstream ss;
    	  ss<<"foo"<<i<<".txt";


    	  multi.tries[i]->exportGraphViz(ss.str());
      }*/

  cout.width(12);

  cout.precision(12);


  //write information about fp and stuff to file :)

  double avg,avg90,maxx,bloom_avg;
  //getTimes(syn.queryTimes,&avg,&avg90,&maxx);
  //bloom_avg = 1000000.0 * bl.handleSum/EXPQUERIES;
  long double syn_avg,multi_avg;
  syn_avg =syn.getQueryTime(LOOKUP);
  multi_avg = multi.getQueryTime(LOOKUP);
  cout<<"-------TIME-----------------"<<endl;
  cout<<"Synopsis avg lookup time in ms: "<<syn_avg<<endl;
  cout<<"MultiSynopsis avg lookup time in ms: "<<multi_avg<<endl;
  cout<<"----------------------------"<<endl;
  cout<<"-------TIME-----------------"<<endl;
	cout<<"Synopsis avg adapt time in ms: "<<syn.getQueryTime(ADAPT)<<endl;
	cout<<"MultiSynopsis avg adapt time in ms: "<<multi.getQueryTime(ADAPT)<<endl;
	cout<<"----------------------------"<<endl;
	 cout<<"-------TIME-----------------"<<endl;
		cout<<"Synopsis avg truncate time in ms: "<<syn.getQueryTime(TRUNCATE)<<endl;
		cout<<"MultiSynopsis avg truncate time in ms: "<<multi.getQueryTime(TRUNCATE)<<endl;
		cout<<"----------------------------"<<endl;


  FILE * results = fopen(OUTPUTFILE,"a");
  if(VARYFS)
  {

  	fprintf(results,"%d %d %d %d %d %d %f %f %f %.10f %d \n",RANGEFILTERSIZE,base.fp,syn.fp,fpBloom,tp,tn,avg,avg90,maxx,bloom_avg,bl.getK());
  }
  if(VARYINT)
 {
	//fprintf(results,"%f %d %d %d %d %d %f %f %d \n",MEAN,fpb,fpSyn,fpBloom,tp,tn,syn.h//+syn.truncateSum,bl.handleSum,syn.maxLoops);
	printf("%f %d %d %d \n",MEAN,base.fp,syn.fp,fpBloom);
 }
 if(VARYDBS)
 {

	fprintf(results,"%d %d %d %d \n",DBSIZE,base.fp,syn.fp,fpBloom);
 }

 if(VARYPART)
  {
    cout<<"varying partitions!"<<endl;
 	fprintf(results,"%d %d %d %d %d %.10Lf %.10Lf %.10Lf %.10Lf %.10Lf %.10Lf %d \n",
 			PARTITIONS,base.fp,syn.fp,multi.getFp(),fpBloom,
 			syn.getQueryTime(LOOKUP),multi.getQueryTime(LOOKUP),
 			syn.getQueryTime(ADAPT),multi.getQueryTime(ADAPT),
 			syn.getQueryTime(TRUNCATE),multi.getQueryTime(TRUNCATE),
 			multi.multiple*10
 			);
  }



  fclose(results);
  free(OUTPUTFILE);

  cout<<0<< "\t, " <<base.fp<<"\t\t, "<< syn.fp << "\t\t, " /*syn2.fp << "\t\t, "*/<< multi.getFp() << "\t\t, " << fpBloom << "\t\t, " << tp << "\t\t, "<<tn << "\t\t, "
  	 << qGen.getAverageLength()<< endl;
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

	assert((VARYINT ^ VARYFS ^ VARYDBS ^ VARYPART) && !(VARYINT && VARYFS && VARYDBS));
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



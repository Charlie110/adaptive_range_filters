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
#include "Synopsis/SynopsisMD2.h"
#include <sstream>
#include <sched.h>
#include "curves/Curve.h"
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
uint DBSIZE = 1000;    // number of keys in the cold store of the database
bool ZIPFDATA = 0 ;
double ZIPFDATAFACTOR = 1.2;  // zipf factor for database



int EXPQUERIES = 100;


int TRAININGQUERIES = 10;    // number of range queries used for training

double MEANT;
double STDDEVT;

bool JUSTPOINTQUERIES = 1;
double MEAN;
double STDDEV;
bool USEOLDZIPFINTERVAL;

bool ZIPFQUERIES = 0;
double ZIPFQUERIESFACTOR = 1.2;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE = 1000;  // size of synopsis in nodes
//int BLOOMFILTERSIZE;//  2 * RANGEFILTERSIZE  // size of poor man's Bloom filter in bits


// Book-keeping
char * OUTPUTFILE;

int main(int argc, char* argv[]) {


  cout<<"Experiment 0 - tracing stuff..."<<endl;

  int dim = 2;
  int MAX = 63;

    DBDOMAIN = ((MAX+1) * (MAX+1))-1;

    vector<Curve::attribute> attr(dim);
    for(int i=0;i<dim;i++)
    {
    	attr[i].lowerb = 0;
    	attr[i].higherb = MAX;
    }

    //attr[1].higherb = 7;
    //attr[2].higherb = 127;

    vector<int> lowp(dim);
	for(int i=0;i<dim;i++)
	   lowp[i] = attr[i].lowerb;
	vector<int> highp(dim);
	for(int i=0;i<dim;i++)
		   lowp[i] = attr[i].higherb;

  Database db(attr,'c');
  //FastDB db(DBDOMAIN +1);
 // Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
 // Uniform unif(DBDOMAIN+1);
   Zipf zipf(MAX+1,ZIPFDATAFACTOR);
    Uniform unif(MAX+1);
  if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf);
  else
	db.populate(DBSIZE, &unif);
 // print_config();
  db.plot();


  SynopsisMD2 syn(dim,&db,lowp,highp,32);
  syn.setAlternate();

  syn.printDims();

  return 0;
  syn.takeSnapshot("begin");
  vector<int> left(2);
  vector<int> right(2);

  left[0] = 4;
  left[1] = 8;


  right = left;
  syn.learn_from_fp(left,right);
  syn.takeSnapshot("after point split");

  left[0] = 3;
  left[1] = 5;

  //right[0] = atoi(argv[1]);
  //right[1] = atoi(argv[2]);
  right[0] = 4;
   right[1] = 6;

 //left = right;

  //syn.learn_from_fp(left,right);
  syn.takeSnapshot("after range split");



  Query::QueryMD_t q; q.low = left; q.high = right;
  syn.handleQuery(q,true,false);

  return 0;


  left[0] = 3;
  left[1] = 7;

  right[0] = 5;
  right[1] = 9;
  q.low = left;
  q.high = right;

  syn.handleQuery(q,true,false);

 // assert(!(syn.handleQuery(q,true,false)));
  //syn.takeSnapshot("after mark used (hopefulliez)");

  ///oookay -> nao debug deescalashun//
/*
  syn.print_clock();
  syn.truncate(20);

  syn.takeSnapshot("after truncaet");


  //syn0.perfect(&db);
  //syn1.init();
  //syn1.perfect(&db);*/

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

  /*Query qTrain1(DBDOMAIN, 0,MEANT,STDDEVT,
		  ZIPFQUERIESFACTOR, TRAININGQUERIES,
		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
  for (int i=0; i<TRAININGQUERIES/3; i++) {
    Query::Query_t q = qTrain1.nextQuery();
    bool qR = db.rangeQuery(q);
    syn0.handleQuery(q,&db,true,qR);
   // syn1.handleQuery(q,&db,true,qR);


  }*/


 return 0;
}





/*
 * Trace3.cpp
 *
 *  Created on: Dec 19, 2012
 *      Author: carolinux
 */

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
#include "Synopsis/QuadSynopsis.h"
#include "Synopsis/SynopsisMD2.h"
#include "Synopsis/SynopsisMD.h"
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
uint DBSIZE = 3;    // number of keys in the cold store of the database
bool ZIPFDATA = 0 ;
double ZIPFDATAFACTOR = 1.2;  // zipf factor for database



int EXPQUERIES = 1000;


int TRAININGQUERIES = 5000;    // number of range queries used for training

double MEANT = 50;
double STDDEVT = 17;

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
  int MAX = 3;

    DBDOMAIN = ((MAX+1) * (MAX+1))-1;

    vector<Curve::attribute> attr(dim);
    for(int i=0;i<dim;i++)
    {
    	attr[i].lowerb = 0;
    	attr[i].higherb = MAX;
    }


    vector<int> lowp(dim);
	for(int i=0;i<dim;i++)
	   lowp[i] = attr[i].lowerb;
	vector<int> highp(dim);
	for(int i=0;i<dim;i++)
		   lowp[i] = attr[i].higherb;

  Database db(attr,'c');

   Zipf zipf(MAX+1,ZIPFDATAFACTOR);
    Uniform unif(MAX+1);
 /* if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf);
  else
	db.populate(DBSIZE, &unif);

  db.plot();*/

    vector<int> p1 = vector<int>(2);
    p1[0] = 2;
    p1[1] = 1;
    vector<int> p3 = vector<int>(2);
       p3[0] = 2;
       p3[1] = 0;
       vector<int> p2 = vector<int>(2);
          p2[0] = 2;
          p2[1] = 2;
  db.addPoint(p1);
 // db.addPoint(p2);
  db.addPoint(p3);

  SynopsisMD2 multi_alt(dim,&db,lowp,highp,32);
   multi_alt.setAlternate();
   multi_alt.perfect(&db);
   multi_alt.takeSnapshot("KD ARF");
  QuadSynopsis quad(dim,&db,lowp,highp,32);
  quad.perfect(&db);
  quad.takeSnapshot("QUAD ARF");
  return 0;

  //multi_alt.setAlternate();
  //cout<<"dim bits:"<<multi_alt.dim_bits<<endl;
  //Bloom bl(db.curve->getDomain(), DBSIZE, &db,RANGEFILTERSIZE,db.curve);



  vector<int> a(2),b(2);
  a[0] =394;
  a[1] = 168;
  b[0] = 426;
  b[1] = 223;

  //assert(db.rangeQuery(a,b));

  //return 0;
  Query qTrain1(dim, MAX,TRAININGQUERIES/3,
  		  MEANT,STDDEVT,ZIPFQUERIES,ZIPFQUERIESFACTOR);
    for (int i=0; i<TRAININGQUERIES/3; i++) {
      Query::QueryMD_t q = qTrain1.nextQueryMD();

      if(JUSTPOINTQUERIES)
    	  q.low = q.high;
      bool qR = db.rangeQuery(q.low,q.high);
      cout<<i<<" th query"<<endl;


      quad.handleQuery(q,true,qR);




    }

    Query qTrain2(dim, MAX,2*TRAININGQUERIES/3,
      		  MEANT,STDDEVT,ZIPFQUERIES,ZIPFQUERIESFACTOR);
        for (int i=0; i<2*TRAININGQUERIES/3; i++) {
          Query::QueryMD_t q = qTrain2.nextQueryMD();
          cout<<i<<" th query, range q"<<endl;
          bool qR = db.rangeQuery(q.low,q.high);

         quad.handleQuery(q,true,qR);




        }


        quad.used_bit_size = 0;

        quad.truncate(RANGEFILTERSIZE);


        quad.convert(0);











 return 0;
}





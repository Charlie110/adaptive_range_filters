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
#include "Synopsis/Statistics.h"
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



int EXPQUERIES = 10000;


int TRAININGQUERIES = 5000;    // number of range queries used for training

double MEANT = 50;
double STDDEVT = 17;

int JUSTPOINTQUERIES = 0;
double MEAN=2;
double STDDEV=1;
bool USEOLDZIPFINTERVAL;

bool ZIPFQUERIES = 0;
double ZIPFQUERIESFACTOR = 1.2;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE = 1000;  // size of synopsis in nodes
//int BLOOMFILTERSIZE;//  2 * RANGEFILTERSIZE  // size of poor man's Bloom filter in bits


// Book-keeping
char * OUTPUTFILE;

int main(int argc, char* argv[]) {

	JUSTPOINTQUERIES = atoi(argv[1]);


  cout<<"Experiment 0 - tracing stuff..."<<endl;

  int dim = 3;
  int MAX = 127;

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
  if(ZIPFDATA)
  	db.populate(DBSIZE, &zipf);
  else
	db.populate(DBSIZE, &unif);

  db.plot();


  QuadSynopsis quad(dim,&db,lowp,highp,32);
  SynopsisMD2 multi(dim,&db,lowp,highp,32);
  SynopsisMD2 multi_alt(dim,&db,lowp,highp,32);
  SynopsisMD sync(attr,'c',RANGEFILTERSIZE,&db);
  SynopsisMD synz(attr,'z',RANGEFILTERSIZE,&db);
  SynopsisMD synh(attr,'h',RANGEFILTERSIZE,&db);
  multi_alt.setAlternate();
  Bloom bl(db.curve->getDomain(), DBSIZE, &db,RANGEFILTERSIZE,db.curve);

 // getchar();



  Query qTrain1(dim, MAX,TRAININGQUERIES/3,
  		  MEANT,STDDEVT,ZIPFQUERIES,ZIPFQUERIESFACTOR);
    for (int i=0; i<TRAININGQUERIES/3; i++) {
      Query::QueryMD_t q = qTrain1.nextQueryMD();

      if(JUSTPOINTQUERIES)
    	  q.low = q.high;
      bool qR = db.rangeQuery(q.low,q.high);
      cout<<i<<" th query"<<endl;
      quad.handleQuery(q,true,qR);
      multi.handleQuery(q,true,qR);

      multi_alt.handleQuery(q,true,qR);

      /* space filling */
      sync.handleQuery(q.low,q.high,true);
      synz.handleQuery(q.low,q.high,true);
      //synh.handleQuery(q.low,q.high,true);





    }

    Query qTrain2(dim, MAX,2*TRAININGQUERIES/3,
      		  MEANT,STDDEVT,ZIPFQUERIES,ZIPFQUERIESFACTOR);
        for (int i=0; i<2*TRAININGQUERIES/3; i++) {
          Query::QueryMD_t q = qTrain2.nextQueryMD();
          cout<<i<<" th query, range q"<<endl;
          bool qR = db.rangeQuery(q.low,q.high);
          quad.handleQuery(q,true,qR);
          multi.handleQuery(q,true,qR);
         multi_alt.handleQuery(q,true,qR);

         /* space filling */
         sync.handleQuery(q.low,q.high,true);
         synz.handleQuery(q.low,q.high,true);
         //synh.handleQuery(q.low,q.high,true);






        }

        quad.set(0);
        multi.set(0);
        multi_alt.set(0);

        sync.set(0);
        synh.set(0);
        synz.set(0);

        quad.truncate(RANGEFILTERSIZE);
        multi.truncate(RANGEFILTERSIZE);

        sync.truncate(RANGEFILTERSIZE);
        synz.truncate(RANGEFILTERSIZE);
        synh.truncate(RANGEFILTERSIZE);
        multi_alt.truncate(RANGEFILTERSIZE);

        quad.convert(0);
        multi.convert(0);
        multi_alt.convert(0);
        sync.convert(0);
		synh.convert(0);
		synz.convert(0);

		quad.stats.reset();
		multi.stats.reset();
		multi_alt.stats.reset();
		sync.stats.reset();
		synz.stats.reset();


		int tp = 0;
		int tn = 0;

		printf("SIZE \t\tQD \t\tKD \t\tKDA \t\tCC \t\tZC \t\tHC \t\tBL \n");

		 Query qGen(dim, MAX,EXPQUERIES,
		  		  MEANT,STDDEVT,ZIPFQUERIES,ZIPFQUERIESFACTOR);
		 for (int i=0; i<EXPQUERIES; i++)
		   {

				Query::QueryMD_t q = qGen.nextQueryMD();

				if(JUSTPOINTQUERIES)
					q.high=q.low;

				//cout<<"Pt:"<<q.right<<endl;

				bool qR = db.rangeQuery(q.low,q.high);

				if(qR)
				  tp++;
				else
				  tn++;

				 /* explicit multi dimensional trees*/

				 quad.handleQuery(q,false,qR); /* quad tree */
				 multi.handleQuery(q,false,qR); /* k-d tree with specified split dimension */
				 multi_alt.handleQuery(q,false,qR); /* k-d tree with alternating
				  	  	  	  	  	  	  	  	  	  	split dimension */

				 /* space filling curves */
				 sync.handleQuery(q,false); /* composite key */
				 synz.handleQuery(q,false); /* Z-order (bit interleaving) */
				// synh.handleQuery(q,false); /* Hilbert curve */

				 /* Bloom filter*/

				 bl.handleQuery(q,qR);


				 if(i %100 == 0)
				 {
					 printf("%d \t%f \t%f \t%f \t%f \t%f \t%f \t%f  \n",
												RANGEFILTERSIZE,
												quad.stats.getFpr(),
												multi.stats.getFpr(),
												multi_alt.stats.getFpr(),
												sync.stats.getFpr(),
												synz.stats.getFpr(),
												synh.stats.getFpr(),
												bl.stats.getFpr());
				 }

		   }


		  printf("SIZE \t\tQD \t\tKD \t\tKDA \t\tCC \t\tZC \t\tHC \t\tBL \n");
		  printf("%d \t%f \t%f \t%f \t%f \t%f \t%f \t%f  \n",
		  	    			RANGEFILTERSIZE,
		  	    			quad.stats.getFpr(),
		  	    			multi.stats.getFpr(),
		  	    			multi_alt.stats.getFpr(),
		  	    			sync.stats.getFpr(),
		  	    			synz.stats.getFpr(),
		  	    			synh.stats.getFpr(),
		  	    			bl.stats.getFpr());










 return 0;
}





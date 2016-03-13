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
#include "Database/DataqueryViz.h"
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



int EXPQUERIES = 5000;


int TRAININGQUERIES = 5000;    // number of range queries used for training

double MEANT = 18;
double STDDEVT = 0;

int JUSTPOINTQUERIES = 1;
//double MEAN=7;
//double STDDEV=4;
double MEAN = 6;
double STDDEV = 0;
bool USEOLDZIPFINTERVAL;

bool ZIPFQUERIES = 0;
double ZIPFQUERIESFACTOR = 1.2;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE = 5000;  // size of synopsis in nodes
//int BLOOMFILTERSIZE;//  2 * RANGEFILTERSIZE  // size of poor man's Bloom filter in bits

int MAX_FSIZE = 20000;
// Book-keeping
char * OUTPUTFILE;

int main(int argc, char* argv[]) {


  cout<<"Experiment - real data: Fiji"<<endl;

  int dim = 2;
  int MAX = 2047;
  dim = atoi(argv[5]);
  bool useH = false;
  if(argc==7)
  		useH = atoi(argv[6]);




  JUSTPOINTQUERIES = atoi(argv[2]);

  bool useCorr = atoi(argv[1]);

  char query_corr;
  if(useCorr)
  {
	  query_corr = argv[3][0];
  }
  else
	  ZIPFQUERIES = atoi(argv[3]);
  char * folder = argv[4];

    DBDOMAIN = ((MAX+1) * (MAX+1))-1;



    vector<Curve::attribute> attr(dim);
    vector<int> domains(dim);
    for(int i=0;i<dim;i++)
    {
    	attr[i].lowerb = 0;
    	attr[i].higherb = MAX;
    	domains[i] = MAX;
    }


	vector<int> cols(dim);
	cols[0] =2;
	cols[1] =3;
	if(dim==3)
		cols[2] = 5;

   Database db(cols,"md/real/fiji.csv",'c',false,10);
   Database dbz(cols,"md/real/fiji.csv",'z',false,10);
   Database dbh(cols,"md/real/fiji.csv",'h',false,10);

	//Database db(attr,'c');
	//db.populateMD(DBSIZE,domains,argv[2][0]);


  db.plot();
  vector<Distribution *> distr(dim);

  if(!ZIPFQUERIES)
  {


	  for(int i=0;i<dim;i++)
		  distr[i] = new Uniform(db.curve->getDomainOfDim(i)+1);

  }
  else
  {
	  for(int i=0;i<dim;i++)
		  distr[i] = new Zipf(db.curve->getDomainOfDim(i)+1,ZIPFQUERIESFACTOR);


  }

  for(int i=0;i<dim;i++)
	  domains[i] = db.curve->getDomainOfDim(i);


	 Query * qGen;

	 if(!useCorr)
	 {
		 qGen = new Query(dim, distr,EXPQUERIES,MEAN,STDDEV,ZIPFQUERIES);

	 }
	 else
		 qGen = new Query(domains,EXPQUERIES,
	  	  		  MEAN,STDDEV,query_corr);

	   vector<int> lowp(dim);
		for(int i=0;i<dim;i++)
		   lowp[i] = db.attr[i].lowerb;
		vector<int> highp(dim);
		for(int i=0;i<dim;i++)
			   lowp[i] = db.attr[i].higherb;



  /* start outputting stuffs */

  stringstream s0,s1,s2,s3,s4;

  s0<<"rm -r "<<folder;

  system(s0.str().c_str());
  system("sleep 5");

  s1<<"mkdir "<<folder;
  system(s1.str().c_str());

  s2<<"cp pre/gnuplot.txt "<<folder<<"/gnuplot.txt";


         system(s2.str().c_str());
         system("sleep 10");
         s2.clear();

  if(JUSTPOINTQUERIES)
  {
 	 s2<<"Point queries";
  }
  else
 	 s2<<"Range queries";

  s2<<"Zipf data?: "<<ZIPFDATA<<", Zipf queries?:"<<ZIPFQUERIES<<" \n";



  s3<<folder<<"/results.txt";

  FILE * results = fopen(s3.str().c_str(),"w");
  fprintf(results,"SIZE \t\tQD \t\tKD \t\tKDA \t\tCC \t\tZC \t\tHC \t\tBL Correlated data \n");
  fprintf(results,"%s",s2.str().c_str());

  fflush(results);

  s4<<folder<<"/data_queries.bmp";

  DataqueryViz plotter(&db,qGen,JUSTPOINTQUERIES);

  plotter.plot2D(s4.str());



  /* END OF EXP STUFF */


  QuadSynopsis quad(dim,&db,lowp,highp,32);
  SynopsisMD2 multi(dim,&db,lowp,highp,32);
  SynopsisMD2 multi_alt(dim,&db,lowp,highp,32);
  SynopsisMD sync(db.attr,'c',RANGEFILTERSIZE,&db);
  SynopsisMD synz(dbz.attr,'z',RANGEFILTERSIZE,&dbz);
  SynopsisMD synh(dbh.attr,'h',RANGEFILTERSIZE,&dbh);
  multi_alt.setAlternate();

  quad.perfect(&db);
  multi.perfect(&db);
  multi_alt.perfect(&db);
  sync.perfect();
  synz.perfect();
  if(useH)
	  synh.perfect();




  Query * qTrain2;
  Query * qTrain1;

  if(!useCorr)
  qTrain2 = new Query(dim, distr,2*TRAININGQUERIES/3,
     		  MEANT,STDDEVT,ZIPFQUERIES);
  else
	  qTrain2 = new Query(domains,2*TRAININGQUERIES/3,
	  	  		  MEANT,STDDEVT,query_corr);


       for (int i=0; i<2*TRAININGQUERIES/3; i++) {
         Query::QueryMD_t q = qTrain2->nextQueryMD();
         if(i%100 == 0)
                 	  cout<<i<<" th query (1st training)"<<endl;


        /* cout<<"Query "<<i<<endl;
         db.curve->printp(q.low);
         db.curve->printp(q.high);
         getchar();*/

         bool qR = db.rangeQuery(q.low,q.high);
         quad.handleQuery(q,true,qR);
         multi.handleQuery(q,true,qR);
       multi_alt.handleQuery(q,true,qR);

        /* space filling */
        sync.handleQuery(q.low,q.high,true);
        synz.handleQuery(q.low,q.high,true);
        if(useH)
        	synh.handleQuery(q.low,q.high,true);

       }



       if(!useCorr)
        qTrain1 = new Query(dim, distr,TRAININGQUERIES/3,
           		  MEAN,STDDEV,ZIPFQUERIES);
        else
      	  qTrain1 = new Query(domains,TRAININGQUERIES/3,
      	  	  		  MEAN,STDDEV,query_corr);


     for (int i=0; i<TRAININGQUERIES/3; i++) {
       Query::QueryMD_t q = qTrain1->nextQueryMD();

       if(JUSTPOINTQUERIES)
     	  q.low = q.high;
       bool qR = db.rangeQuery(q.low,q.high);
       if(i%100 == 0)
               	  cout<<i<<" th query (2nd training)"<<endl;
       quad.handleQuery(q,true,qR);
      multi.handleQuery(q,true,qR);

       multi_alt.handleQuery(q,true,qR);

       /* space filling */
      sync.handleQuery(q.low,q.high,true);
      synz.handleQuery(q.low,q.high,true);
      if(useH)
    	  synh.handleQuery(q.low,q.high,true);





     }



         quad.set(0);
         multi.set(0);
         multi_alt.set(0);

         sync.set(0);
         synh.set(0);
         synz.set(0);

 for(RANGEFILTERSIZE=MAX_FSIZE;RANGEFILTERSIZE>=0;RANGEFILTERSIZE-=2000)
		{


	     if(RANGEFILTERSIZE==0)
	    	 RANGEFILTERSIZE=1000;

	 	 Bloom bl(db.curve->getDomain(), db.num_keys, &db,RANGEFILTERSIZE,db.curve);
	 	 qGen->reset();

	 	 quad.convert(32);
	 	        multi.convert(32);
	 	        multi_alt.convert(32);

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
 		multi_alt.stats.reset();
 		multi.stats.reset();
 		synh.stats.reset();
 		sync.stats.reset();
 		synz.stats.reset();
 		bl.stats.reset();



		int tp = 0;
		int tn = 0;

		 printf("SIZE \t\tQD \t\tKD \t\tKDA \t\tCC \t\tZC \t\tHC \t\tBL \n");

		 for (int i=0; i<EXPQUERIES; i++)
		   {

				Query::QueryMD_t q = qGen->nextQueryMD();

				if(JUSTPOINTQUERIES)
					q.high=q.low;

				//cout<<"Pt:"<<q.right<<endl;

				bool qR = db.rangeQuery(q.low,q.high);

				if(qR)
				  tp++;
				else
				  tn++;
				if(i%100 == 0)
				        	  cout<<i<<" th query (benchmark)"<<endl;

				 /* explicit multi dimensional trees*/

				 quad.handleQuery(q,false,qR); /* quad tree */
				 multi.handleQuery(q,false,qR); /* k-d tree with specified split dimension */
				multi_alt.handleQuery(q,false,qR); /* k-d tree with alternating
				  	  	  	  	  	  	  	  	  	  	//split dimension */

				 /* space filling curves */
				 sync.handleQuery(q,false); /* composite key */
				 synz.handleQuery(q,false); /* Z-order (bit interleaving) */
				 if(useH)
					 synh.handleQuery(q,false); /* Hilbert curve */

				 /* Bloom filter*/

				 bl.handleQuery(q,qR);
				 if((i+1) %100 == 0 && i>10)
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


		  fprintf(results,"%d \t%f \t%f \t%f \t%f \t%f \t%f \t%f  \n",
		  	    			RANGEFILTERSIZE,
		  	    			quad.stats.getFpr(),
		  	    			multi.stats.getFpr(),
		  	    			multi_alt.stats.getFpr(),
		  	    			sync.stats.getFpr(),
		  	    			synz.stats.getFpr(),
		  	    			synh.stats.getFpr(),
		  	    			bl.stats.getFpr());
		  fflush(results);




}

fclose(results);










 return 0;
}





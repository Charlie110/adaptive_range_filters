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
#include "Synopsis/FastSynopsis.h"
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


string plotScript(char*);
string getFilename();


bool ZIPFDATA;
bool ZIPFQUERIES = 0;
int JUSTPOINTQUERIES;
int RANGEFILTERSIZE;

int EXPQUERIES = 300000;//20000;
int TRAININGQUERIES = 510000;//12000;// 5100;
double MEANT = 3000;
double STDDEVT = 1000;
double MEAN = 30;
double STDDEV = 10;
double ZIPFQUERIESFACTOR = 1.2;
double ZIPFDATAFACTOR = 1.2;
int P = 20;
char * FOLDER;
int BITS = 4; //2?
int UPDATES=1000;
int UPDATEBATCH =5000;



int main(int argc, char* argv[]) {

	P = atoi(argv[1]);

	uint DBDOMAIN = (1<<P) -1;

	cout<<"DBDOMAIN:"<<DBDOMAIN<<endl;

	Database db(DBDOMAIN+1);

	int ZIPFD = atoi(argv[3]);
	int DBSIZE = atoi(argv[2]);
	RANGEFILTERSIZE = atoi(argv[4])* DBSIZE;
	MEAN = 30;

    Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
    Uniform unif(DBDOMAIN+1);
	UPDATEBATCH = DBSIZE/20;


  

   if(!ZIPFD)
  {

	  db.populate(DBSIZE, &unif);
  } else
 {
	db.populate(DBSIZE, &zipf);
}

db.plot();
FastSynopsis fsyn(0, DBDOMAIN, &db);
	fsyn.perfect(&db);

        FastSynopsis fsyn1(1, DBDOMAIN, &db);
        fsyn1.perfect(&db);

      FastSynopsis fsyn0(0, DBDOMAIN, &db);
        fsyn0.perfect(&db);




   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);


   Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
 		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
 		  ZIPFQUERIES,false);
   for (int i=0; i<TRAININGQUERIES/3; i++) {
     Query::Query_t q = qTrain1.nextQuery();
     if(JUSTPOINTQUERIES)
        	  q.left=q.right;
     bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);
		  fsyn1.handle_query(q.left, q.right, qR, true);
fsyn0.handle_query(q.left, q.right, qR, true);
    



   }

   cout<<"2nd training.."<<endl;

   Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
 		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
 		  ZIPFQUERIES,false);
   for (int i=0; i<2*TRAININGQUERIES/3; i++) {
     Query::Query_t q = qTrain2.nextQuery();
     bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);
		  fsyn1.handle_query(q.left, q.right, qR, true);
fsyn0.handle_query(q.left, q.right, qR, true);
     

   }

  cout<<"--------------END OF TRAINING----------------------"<<endl;

		fsyn.reset_training_phase();
	fsyn.truncate(RANGEFILTERSIZE);
	fsyn.end_training_phase();
                     fsyn0.reset_training_phase();
        fsyn0.truncate(RANGEFILTERSIZE);
        fsyn0.end_training_phase();
                fsyn1.reset_training_phase();
        fsyn1.truncate(RANGEFILTERSIZE);
        fsyn1.end_training_phase();

  /* Real experiment - Tries are frozen (no adaptation) */
   /*Query qGen(DBDOMAIN, 0,MEAN,STDDEV,
		   ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES);*/

  int fpBloom = 0,tnBloom = 0;
  int tp = 0;
  int tn = 0;


  int updates=0;
  for(int upd =0;upd<=DBSIZE;upd+=UPDATEBATCH)
  {
      // --- RESET STATS --- //
  fsyn.stats.reset();
  fsyn0.stats.reset();
  fsyn1.stats.reset();
  bl.stats.reset();

      Query qGen(DBDOMAIN, 0,MEAN,STDDEV,
      		   ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,false);

    for (int i=0; i<EXPQUERIES; i++)
   {

		Query::Query_t q = qGen.nextQuery();
	bool dR = db.rangeQuery(q);
			bool qR = bl.handleQuery(q, dR);
			 fsyn.handle_query(q.left, q.right, dR, false);
			fsyn1.handle_query(q.left, q.right, dR, true);
			bool res1 = fsyn0.handle_query(q.left, q.right, dR, true);
			fsyn0.truncate(RANGEFILTERSIZE);
			fsyn1.truncate(RANGEFILTERSIZE);
	if(i % 1000 ==0)
		cout<<i<<" out of "<<EXPQUERIES<<endl;

    }
   /* end of one update round */

//    cout<<"Writing to "<<OUTPUTFILE<<endl;



    /*	cout<<"Syn0 score:"<<syn0.getScore(0,DBDOMAIN,&db)<<endl;
    	cout<<"Syn1 score:"<<syn1.getScore(0,DBDOMAIN,&db)<<endl;
    	cout<<"Syn score:"<<syn.getScore(0,DBDOMAIN,&db)<<endl;*/
     	  fprintf(stderr,"%d\t%f\t%f\t%f\t%f\n",
		    			upd,fsyn.stats.getFpr(),
		    			fsyn0.stats.getFpr(),
		    			fsyn1.stats.getFpr(),
		    			bl.stats.getFpr());

     	

       int prev_size = db.size();
       /* new data */

       uint64* newKeys = NULL;

       if(ZIPFD)
     	 newKeys = db.addKeys(UPDATEBATCH,&zipf);
       else
     	  newKeys = db.addKeys(UPDATEBATCH,&unif);

       fsyn.recordNewKeys(newKeys, UPDATEBATCH,STRAT0);
       fsyn0.recordNewKeys(newKeys, UPDATEBATCH,STRAT0);
       fsyn1.recordNewKeys(newKeys, UPDATEBATCH,STRAT0);
fsyn0.truncate(RANGEFILTERSIZE);
			fsyn1.truncate(RANGEFILTERSIZE);


       bl.recordNewKeys(newKeys, UPDATEBATCH);
       db.deleteKeys(UPDATEBATCH);
       free(newKeys);

  } //End of all the updates

  return 0;
}  // main

string plotScript(char* OUTPUTFILE)
{

	stringstream ss,title,fn;


	if(JUSTPOINTQUERIES)
	{
		title<<"Point Queries ";
		fn<<"point";
	}
	else
	{
		title<<"Range Queries (m ="<<MEAN<<") ";
		fn<<"range"<<MEAN;
	}
	if(ZIPFDATA)
	{
		title<<"Zipf Data /";
		fn<<"z";
	}
	else
	{
		title<<"Uniform Data /";
		fn<<"u";
	}
	if(ZIPFQUERIES)
	{
		title<<"Zipf Queries";
		fn<<"z";
	}
	else
	{
		title<<"Uniform Queries";
		fn<<"u";
	}

	fn<<".png";

    ss<<" set yrange[0:129] \n";
    //ss<<" set xrange[1:300] \n";
    //ss<<" set xtics(1,50,100,150,200,250,300)\n";
    ss<<" set ytics (0,20,40,60,80,100)\n";
    ss<<"set title \""<<title.str()<<"\" \n";
	ss<<"set xlabel \"#Updates\"\n";
	ss<<"set ylabel \"False Positive Rate % \"\n";
	ss<<"set term png\n";
	ss<<"set output \""<<getFilename()<<"\"\n";
	ss<<"plot \""<<OUTPUTFILE<<"\"  using 1:2 title"
			" \"ARF non-adaptive\""
			" with linespoints pt 6 linecolor rgb \"blue\" linewidth 1,\\";
	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:3 title \"ARF adaptive 0-bit\""
				" with linespoints pt 8 linecolor rgb \"#7D26CD\" linewidth 1,\\";
	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:4 title \"ARF adaptive 1-bit\""
					" with linespoints pt 9 linecolor rgb \"purple\" linewidth 1,\\";


	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:5 title \"Bloom\""
			" with linespoints pt 4 linecolor rgb \"#610000\" linewidth 1";




	return ss.str();

}


string getFilename()
{
	stringstream fn;

	fn<<"exp9-";


	if(JUSTPOINTQUERIES)
		{

			fn<<"point";
		}
		else
		{

			fn<<"range"<<MEAN;
		}
		if(ZIPFDATA)
		{

			fn<<"z";
		}
		else
		{

			fn<<"u";
		}
		if(ZIPFQUERIES)
		{

			fn<<"z";
		}
		else
		{

			fn<<"u";
		}

		fn<<".png";

	return fn.str();
}

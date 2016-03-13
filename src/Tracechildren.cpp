/*
 * Tracechildren.cpp
 *
 *  Created on: Jan 13, 2013
 *      Author: carolinux
 */
#include "Synopsis/QuadSynopsis.h"
#include <stdio.h>
#include "Database.h"
#include <vector>
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
int MINRANGE = 3;

//  Database Parameters
uint DBDOMAIN;  // domain of the keys of the database
uint DBSIZE = 1000;    // number of keys in the cold store of the database
bool ZIPFDATA = 0 ;
double ZIPFDATAFACTOR = 1.2;  // zipf factor for database

int MAX_FSIZE = 10000;

int EXPQUERIES = 5000;


int TRAININGQUERIES = 5000;    // number of range queries used for training

double MEANT = 300;
double STDDEVT = 100;

int JUSTPOINTQUERIES = 0;
double MEAN=30;
double STDDEV=10;
bool USEOLDZIPFINTERVAL = false;
bool USEOLDZIPFINTERVALT = false;

bool ZIPFQUERIES = 0;
double ZIPFQUERIESFACTOR = 1.2;     // zipf factor for distribution of length

//  Access Filter parameters

int RANGEFILTERSIZE = 1000;  // size of synopsis in nodes
//int BLOOMFILTERSIZE;//  2 * RANGEFILTERSIZE  // size of poor man's Bloom filter in bits


// Book-keeping
string OUTPUTFILE;
string FOLDER;
using namespace std;

int main(int argc, char * argv[])
{
	JUSTPOINTQUERIES = atoi(argv[1]);
	ZIPFDATA = atoi(argv[2]);
	ZIPFQUERIES = atoi(argv[3]);
	FOLDER = argv[4];
	int f = 256;

	//DBDOMAIN = (1048576 * f )-1;
	DBDOMAIN = 131171;

	int CHILDREN;

	stringstream ss,ss2,ss3,ss1;
	ss1<<"rm -rf "<<FOLDER;
	system(ss1.str().c_str());
	ss<<"mkdir "<<FOLDER;
	system(ss.str().c_str());


	ss2<<FOLDER<<"/results.txt";
	OUTPUTFILE = ss2.str();

	ss3<<"touch "<<OUTPUTFILE;
	cout<<"command: "<<ss3.str();
	system(ss3.str().c_str());



	FILE * results = fopen(OUTPUTFILE.c_str(),"w");
		fclose(results);
		results = fopen(OUTPUTFILE.c_str(),"a");
		fprintf(results,"CHI\t ARF\t\t BLOOM\t UNUSED-BITS\t\n");

	RANGEFILTERSIZE = 8000;
	Database db(DBDOMAIN+1);
	if(ZIPFDATA)
	  {
		  Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
		 // if(!ZIPFDATA && ZIPFQUERIES && JUSTPOINTQUERIES)
			//  zipf = Zipf(DBDOMAIN+1,ZIPFDATAFACTOR,true);

		  db.populate(DBSIZE, &zipf);
	  }

	  else
	  {
		  Uniform unif(DBDOMAIN+1);
		  db.populate(DBSIZE, &unif);
	  }
	vector<int> lowp(1),highp(1);
	lowp[0] = 0;
	highp[0] = DBDOMAIN;
	Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
		 		  ZIPFQUERIESFACTOR, TRAININGQUERIES/3,
		 		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
	 Query qTrain2(DBDOMAIN, 0,MEANT,STDDEVT,
		 		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES/3,
		 		  ZIPFQUERIES,USEOLDZIPFINTERVALT);
	 Query qGen(DBDOMAIN, MINRANGE,MEAN,STDDEV,
		  	   		   ZIPFQUERIESFACTOR, EXPQUERIES, ZIPFQUERIES,false);

	//QuadSynopsis child(1,&db,highp,lowp,32,CHILDREN);

	//child.takeSnapshot("foo");
	//return 0;
	for( CHILDREN=2;CHILDREN<50;CHILDREN++)
	{

	 QuadSynopsis child(1,&db,highp,lowp,32,CHILDREN);


	   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);


	   qTrain1.reset();
	   for (int i=0; i<TRAININGQUERIES/3; i++) {
	     Query::Query_t q = qTrain1.nextQuery();
	     if(JUSTPOINTQUERIES)
	        	   q.left=q.right;
	     bool qR = db.rangeQuery(q);

	     child.handleQuery(q,true,qR);



	   }

	   cout<<"2nd training.."<<endl;

	   qTrain2.reset();
	   for (int i=0; i<2*TRAININGQUERIES/3; i++) {
	     Query::Query_t q = qTrain2.nextQuery();

	     bool qR = db.rangeQuery(q);

	     child.handleQuery(q,true,qR);

	   }

	   child.truncate(RANGEFILTERSIZE);
	   child.convert(0);
	   child.stats.reset();


	  	qGen.reset();
	  	 for (int i=0; i<EXPQUERIES; i++)
	  	   {

	  			Query::Query_t q = qGen.nextQuery();

	  			if(JUSTPOINTQUERIES)
	  				q.right=q.left;

	  			//cout<<"Pt:"<<q.right<<endl;

	  			bool qR = db.rangeQuery(q);


	  			child.handleQuery(q,false,qR);

	  			bl.handleQuery(q,qR);


	  	   }

	  	 printf(" \tbase \tchild \tbloom \tunused bits \n");
		  printf("%d \t%f  \t%ft %d \n",
		  	    			CHILDREN,
		  	    			child.stats.getFpr(),
		  	    			bl.stats.getFpr(),child.getUnused()*2);
		  fprintf(results,"%d \t%f  \t%ft %d \n",
		 		  	    			CHILDREN,
		 		  	    			child.stats.getFpr(),
		 		  	    			bl.stats.getFpr(),child.getUnused()*2);
		  fflush(results);



		  child.stats.print();
		  bl.stats.print();


		  cout<<"Child unused :"<<child.getUnused()<<endl;


	}
	fclose(results);



	return 0;
}


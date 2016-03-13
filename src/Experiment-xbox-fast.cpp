/*
 * Experiment-xbox-fast.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: car
 */


#include <iostream>
using namespace std;
#include <string>
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
#include "Bloom.h"
#include "Synopsis/FastSynopsis.h"



using namespace std;

Synopsis convert0(SynopsisIntClock synStart, int RANGEFILTERSIZE) {

	SynopsisIntClock synStart0(synStart);
	   synStart0.set(0);
	   synStart0.truncateClock(RANGEFILTERSIZE);
	   synStart.resetTime();
	   synStart0.resetTime();
	   synStart0.convertToCompact(0);
	   //syn1.sanityCheckUsed();
	   Synopsis syn0(synStart0);
	   syn0.set(0);
	   return syn0;


}

Synopsis convert1(SynopsisIntClock synStart, int RANGEFILTERSIZE) {

	   synStart.truncateClock(RANGEFILTERSIZE);
	   synStart.resetTime();
	   synStart.convertToCompact(1);
	   Synopsis syn1(synStart);
	   syn1.set(1);
	   return syn1;

}
int main(int argc, char ** argv) {
	//ftiakse mia vasi dedomenwn
	int BITS = atoi(argv[1]);
	int DATA_IDX = 2;
	int QUERIES_IDX = 3;
	int RANGEFILTERSIZE = 8 * 8000;
	printf("#arguments: %d \n",argc);
	//getchar();



	unsigned int DBDOMAIN = (1 << BITS) - 1;
	cout<<"initializing database"<<endl;
	Database db(argv[DATA_IDX], DBDOMAIN);
	db.plot();
	cout<<" initalizing bloom filer.."<<endl;
	RANGEFILTERSIZE = 8 * db.num_keys;
	if(RANGEFILTERSIZE == 0) {
		RANGEFILTERSIZE = 10;
	}
	Bloom    bl(DBDOMAIN, db.num_keys, &db, RANGEFILTERSIZE);
	Bloom    bls(DBDOMAIN, db.num_keys, &db, RANGEFILTERSIZE/2);
	cout<<"initializing queries...";
	Query queries(argv[QUERIES_IDX]);
	cout<<"Initializing synopsis"<<endl;
	SynopsisIntClock synStart(DBDOMAIN, RANGEFILTERSIZE, false, &db);
	FastSynopsis fsyn(0, DBDOMAIN, &db);
	//fsyn.perfect(&db);
	fsyn.print_size();
	//getchar();
	FastSynopsis fsmall(0, DBDOMAIN, &db);
	synStart.init();
	synStart.setDatabase(&db);
	/*synStart.perfect(&db);*/
	 synStart.printStats("syn");

	 cout<<"LIMIT:"<<RANGEFILTERSIZE<<endl;
	// getchar();

	cout<<"ready to train!"<<endl;

	   for (int i=0; i<queries.getSize(); i++) {
	     Query::Query_t q = queries.nextQuery();

	     if (i % 100000 == 0) {
	    	 cout<<"training: Query "<<i<<" out of "<<queries.getSize()<<" ("<<i*100.0/queries.getSize() <<"%)"<<endl;
	     }
	     bool qR = db.rangeQuery(q);
	    // synStart.handleQuery(q,&db,true,qR);
	     fsyn.handle_query(q.left, q.right, qR, true);
	     fsmall.handle_query(q.left, q.right, qR, true);


	  /*   cout<<"After query "<<i<<":"<<q.left<<", "<<q.right<<endl;
		   fsyn.print_size();
		   synStart.printStats("syn");
		   synStart.takeSnapshot();

		   getchar();*/
	   }

	   cout<<"ended training"<<endl;



	   Synopsis syn0 = convert0(synStart, RANGEFILTERSIZE);
	   Synopsis synsmall = convert0(synStart, RANGEFILTERSIZE/2);
	   Synopsis syn1 = convert1(synStart, RANGEFILTERSIZE);
	   fsyn.truncate(RANGEFILTERSIZE);
	   fsyn.end_training_phase();


	   fsmall.truncate(RANGEFILTERSIZE/2);
	   fsmall.end_training_phase();
	   queries.reset();

	   for (int i=0; i<queries.getSize(); i++) {
	     Query::Query_t q = queries.nextQuery();
	     bool qR = db.rangeQuery(q);
	     if (i % 100000 == 0) {
	    	    	 cout<<"real run: Query "<<i<<" out of "<<queries.getSize()<<" ("<<i*100.0/queries.getSize() <<"%)"<<endl;
	    }
	     	    // syn0.handleQuery(q,&db,false,qR);
	     	     //synsmall.handleQuery(q,&db,false,qR);
	     	     bl.handleQuery(q,qR);
	     	     bls.handleQuery(q,qR);

	     	    fsyn.handle_query(q.left, q.right, qR, false);
	     	    fsmall.handle_query(q.left, q.right, qR, false);


	   }



	// meta: kane train me ola ta queries
	// ksanatreksta kai des ti egine :D

	   printf("-----------------\n");
	   synsmall.printStats("small synopsis");
	   syn0.printStats("synopsis");
	   bls.printStats("bloom filter");
	   bl.printStats("bloom filter");
	   printf("Filter sizes: %d\n", RANGEFILTERSIZE);
	   printf("Unique values: %d\n", db.num_keys);
	   printf("Queries: %d\n", queries.getSize());

	   printf("--------\n");
	   fsyn.print_size();
	   fsyn.stats.print();
	   printf("--------\n");
	   printf("--------\n");
	   fsmall.print_size();
	   fsmall.stats.print();
	   printf("--------\n");

GREEN_COLOR
printf("%f\t %f\t %f\t %f\t %f\n", fsmall.stats.getFpr(), fsyn.stats.getFpr(), bls.stats.getFpr(), bl.stats.getFpr(), fsyn.stats.getColdStore());
WHITE_COLOR
printf("\n");
printf("%f\t %f\t %f\t %f\t %f\n", fsmall.stats.getFpr(), fsyn.stats.getFpr(), bls.stats.getFpr(), bl.stats.getFpr(), fsyn.stats.getColdStore());



	return 1;
}





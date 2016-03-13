
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
	SynopsisIntClock synStart(DBDOMAIN, RANGEFILTERSIZE, true, &db);
	synStart.init();
	//synStart.perfect(&db);

	cout<<"ready to train!"<<endl;

	   for (int i=0; i<queries.getSize(); i++) {
	     Query::Query_t q = queries.nextQuery();

	     if (i % 100000 == 0) {
	    	 cout<<"training: Query "<<i<<" out of "<<queries.getSize()<<" ("<<i*100.0/queries.getSize() <<"%)"<<endl;
	     }
	     bool qR = db.rangeQuery(q);
	     synStart.handleQuery(q,&db,true,qR);
	   }

	   cout<<"ended training"<<endl;

//TODO: make a proper function that turns IntClock to whatever it is supposed to turn it :-)
	   // no moar stupid

	   Synopsis syn0 = convert0(synStart, RANGEFILTERSIZE);
	   Synopsis synsmall = convert0(synStart, RANGEFILTERSIZE/2);
	   Synopsis syn1 = convert1(synStart, RANGEFILTERSIZE);

	   queries.reset();

	   for (int i=0; i<queries.getSize(); i++) {
	     Query::Query_t q = queries.nextQuery();
	     bool qR = db.rangeQuery(q);
	     if (i % 100000 == 0) {
	    	    	 cout<<"real run: Query "<<i<<" out of "<<queries.getSize()<<" ("<<i*100.0/queries.getSize() <<"%)"<<endl;
	    }
	     	     syn0.handleQuery(q,&db,false,qR);
	     	     synsmall.handleQuery(q,&db,false,qR);
	     	     bl.handleQuery(q,qR);
	     	     bls.handleQuery(q,qR);


	   }



	// meta: kane train me ola ta queries
	// ksanatreksta kai des ti egine :D

	   synsmall.printStats("small synopsis");
	   syn0.printStats("synopsis");
	   bls.printStats("bloom filter");
	   bl.printStats("bloom filter");
	   printf("Filter sizes: %d\n", RANGEFILTERSIZE);
	   printf("Unique values: %d\n", db.num_keys);
	   printf("Queries: %d\n", queries.getSize());

GREEN_COLOR
printf("%f\t %f\t %f\t %f\t %f\n", synsmall.getFpr(), syn0.getFpr(), bls.stats.getFpr(), bl.stats.getFpr(), syn0.getColdStore());
WHITE_COLOR
printf("\n");
printf("%f\t %f\t %f\t %f\t %f\n", synsmall.getFpr(), syn0.getFpr(), bls.stats.getFpr(), bl.stats.getFpr(), syn0.getColdStore());



	return 1;
}

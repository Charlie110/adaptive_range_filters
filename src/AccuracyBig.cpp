/*
 * AccuracyBig.cpp
 *
 *  Created on: Jun 17, 2013
 *      Author: car
 */




/*
 * RangedBloomTest.cpp
 *
 *  Created on: Jun 5, 2013
 *      Author: carolinux
 */

#include "Bloom.h"
#include <stdio.h>
#include <assert.h>
#define GREEN_COLOR printf("\033[01;32m");
#define WHITE_COLOR printf("\033[01;37m");
#include <vector>
#include "Synopsis/FastSynopsis.h"
#include "Synopsis.h"
#include <stdlib.h>
#include "Util.h"
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


void testTrivial(int RANGEFILTERSIZE, uint64 DBDOMAIN, int DBSIZE, int seed, bool ZIPFD, int ZIPFQ, int MEAN) {
	//int MEAN = 1;
	uint64 step = RANGEFILTERSIZE;
	double STDDEV = 0;

	if(MEAN ==1) 
		STDDEV = 0;
	else

		STDDEV = MEAN /3.0;


	//uint64 DBDOMAIN = (1<<23)-1;

	FastDB db(DBDOMAIN+1);
	cout<<"DB space allocated."<<endl;

	  Uniform unif(DBDOMAIN+1);
	  Zipf zipf(DBDOMAIN+1,1.2,seed);
	 if(ZIPFD == true)
	  	db.populate(DBSIZE, &zipf);
	else
	   	db.populate(DBSIZE, &unif);
	  db.plot();

	  cout<<"plotted"<<endl;

	  printf("Db keys: %u\n", db.values.size());
	  uint64 contains = 0;
	  for(uint64 i =0; i<DBDOMAIN+1;i++) {
		  if(db.contains(i))
			  contains++;



	  }

	  cout<<"contains:"<<contains<<"out of "<<DBDOMAIN<<endl;


	//TODO maek a synopsis hiar
	FastSynopsis fsyn(0, DBDOMAIN, &db);
	fsyn.perfect(&db);

        FastSynopsis fsyn1(1, DBDOMAIN, &db);
        fsyn1.perfect(&db);

      FastSynopsis fsyn0(0, DBDOMAIN, &db);
        fsyn0.perfect(&db);

uint64 EXPQUERIES = 20*db.num_keys;
	if(db.num_keys>=100000)
		EXPQUERIES = 3 * db.num_keys;



Query * qGen;
if(ZIPFQ == 1)
	   qGen = new Query(DBDOMAIN,1,MEAN,STDDEV,
	  		   1.2, EXPQUERIES, ZIPFQ,false,seed);
else
   qGen = new Query(DBDOMAIN,1,MEAN,STDDEV,
	  		   1.2, EXPQUERIES, ZIPFQ,false);


	int TRAININGQUERIES = 510000;
Query * qTrain1;
if(ZIPFQ ==1)
	  qTrain1 = new Query(DBDOMAIN, 0,MEAN,STDDEV,
	 		  1.2, TRAININGQUERIES/3,
	 		  ZIPFQ,false, seed);
else
     qTrain1 =  new Query(DBDOMAIN, 0,MEAN,STDDEV,
	 		  1.2, TRAININGQUERIES/3,
	 		  ZIPFQ,false);


	   for (int i=0; i<1*TRAININGQUERIES/3; i++) {
	     Query::Query_t q = qTrain1->nextQuery();
	     //if(JUSTPOINTQUERIES)
	      //       	   q.left=q.right; //FIXME here
	    //if(i% 10000
	     bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);
		  fsyn1.handle_query(q.left, q.right, qR, true);
fsyn0.handle_query(q.left, q.right, qR, true);


	   }
Query * qTrain2;
if(ZIPFQ ==1)
	  qTrain2 = new Query(DBDOMAIN, 0,MEAN*100,STDDEV*100,
	 		  1.2, 2*TRAININGQUERIES/3,
	 		  ZIPFQ,false, seed);
else
     qTrain2 = new Query(DBDOMAIN, 0,MEAN*100,STDDEV*100,
	 		  1.2, 2*TRAININGQUERIES/3,
	 		  ZIPFQ,false);
	    for (int i=0; i<2*TRAININGQUERIES/3; i++) {
	      Query::Query_t q = qTrain2->nextQuery();
	      //if(JUSTPOINTQUERIES)
	       //       	   q.left=q.right; //FIXME here
	      bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);
  fsyn1.handle_query(q.left, q.right, qR, true);
fsyn0.handle_query(q.left, q.right, qR, true);


	    }



	for(RANGEFILTERSIZE = 10*step;RANGEFILTERSIZE>=step; RANGEFILTERSIZE-=step) {
//for(int i=1;i<=1;i++) {
		fsyn.reset_training_phase();
	fsyn.truncate(RANGEFILTERSIZE);
	fsyn.end_training_phase();
                     fsyn0.reset_training_phase();
        fsyn0.truncate(RANGEFILTERSIZE);
        fsyn0.end_training_phase();
                fsyn1.reset_training_phase();
        fsyn1.truncate(RANGEFILTERSIZE);
        fsyn1.end_training_phase();

  
	Bloom    bl(DBDOMAIN, db.num_keys, &db, RANGEFILTERSIZE);




	


	   int wrongs =0;

	    for (uint64 j=0; j<EXPQUERIES; j++)
	   {

			Query::Query_t q = qGen->nextQuery();


			//printf("Query: %u - %u \n",q.left,q.right);
			if(j%10000 ==0)
			printf("query %d out of %d \n", j, EXPQUERIES);


			bool dR = db.rangeQuery(q);
			bool qR = bl.handleQuery(q, dR);
			 fsyn.handle_query(q.left, q.right, dR, false);
			fsyn1.handle_query(q.left, q.right, dR, true);
			fsyn0.handle_query(q.left, q.right, dR, true);

			if(j%5 ==0) 
{
			fsyn0.truncate(RANGEFILTERSIZE);
			fsyn1.truncate(RANGEFILTERSIZE);}

	   }


	printf("Synopsis:\n");
	fsyn.stats.print();


	cout<<"plain bloom:"<<endl;
	bl.stats.print();
	 cout<<"expected error rate from math:"<<bl.fpRate(MEAN)<<endl;


	 printf("wrongs:%d\n",wrongs);
	 fprintf(stderr,"%d\t %f\t%f\t%f\t%f\n",RANGEFILTERSIZE, fsyn.stats.getFpr(),fsyn0.stats.getFpr(),fsyn1.stats.getFpr(), bl.stats.getFpr());


}

}



int main(int argc, char* argv[]) {

if(argc ==1) {


	printf("usage: domainpower,keys, mean, zipfd, zipfq, (seed)\n");
return 1;
}


    int seed = 0;
    int ARGS = 7;
    if(argc == ARGS)
	seed = atoi(argv[ARGS-1]);
    int bits_per_key = 10;

    int keys = atoi(argv[2]);
    bool ZIPFD = atoi(argv[4]);
    int ZIPFQ = atoi(argv[5]);
    int RANGEFILTERSIZE = bits_per_key * keys;

    uint64 DBDOMAIN = (2<<atoi(argv[1]))-1;

   printf("Keys %d, domain %llu, ratio %f",keys, DBDOMAIN, (keys+0.0)/DBDOMAIN);
     int MEAN= (atoi(argv[3]));


// TEST 1//
 
// graphs 1-8
//FIXME ZIPF 1?
   
    for(int i=1; i<2; i++) {

	RANGEFILTERSIZE = keys * i;
	testTrivial(RANGEFILTERSIZE,DBDOMAIN,keys, seed, ZIPFD, ZIPFQ, MEAN);

     }
    return 1;

//test 2//
//graph 9*/

    for(int i=1; i<=300; i++) {

	RANGEFILTERSIZE = keys * 8;
	testTrivial(RANGEFILTERSIZE,DBDOMAIN,keys, seed, false, false, i);

     }


 GREEN_COLOR
 printf("ALL TESTS PASS :-) \n");
 WHITE_COLOR


}

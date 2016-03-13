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

void testTrivial(int RANGEFILTERSIZE, int seed) {
	int MEAN = 10;
	int STDDEV = 0;
	bool ZIPFQ = true;
	bool ZIPFD = false;


	int DBDOMAIN = (2<<24)-1;
	int DBSIZE = 100000;
	RANGEFILTERSIZE = DBSIZE * 8;

	Database db(DBDOMAIN+1);

	  Uniform unif(DBDOMAIN+1);
	  Zipf zipf(DBDOMAIN+1,1.2,seed);
	 if(ZIPFD == true)
	  	db.populate(DBSIZE, &zipf);
	else
	   	db.populate(DBSIZE, &unif);
	  db.plot();



	//TODO maek a synopsis hiar
	FastSynopsis fsyn(0, DBDOMAIN, &db);
	fsyn.perfect(&db);

	
	int TRAININGQUERIES = 5000;

	  Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
	 		  1.2, 2*TRAININGQUERIES/3,
	 		  ZIPFQ,false, seed);
	   for (int i=0; i<1*TRAININGQUERIES/3; i++) {
	     Query::Query_t q = qTrain1.nextQuery();
	     //if(JUSTPOINTQUERIES)
	      //       	   q.left=q.right; //FIXME here
	     bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);

	   }

	   Query qTrain2(DBDOMAIN, 0,MEAN*100,STDDEV*5,
	  		  1.2, 2*TRAININGQUERIES/3,
	  		ZIPFQ, false, seed);
	    for (int i=0; i<2*TRAININGQUERIES/3; i++) {
	      Query::Query_t q = qTrain2.nextQuery();
	      //if(JUSTPOINTQUERIES)
	       //       	   q.left=q.right; //FIXME here
	      bool qR = db.rangeQuery(q);
	     fsyn.handle_query(q.left, q.right, qR, true);

	    }



	//for(RANGEFILTERSIZE = 10000;RANGEFILTERSIZE>=1000; RANGEFILTERSIZE-=1000) {

		fsyn.reset_training_phase();
	fsyn.truncate(RANGEFILTERSIZE);
	fsyn.end_training_phase();

	Bloom    bl(DBDOMAIN, db.num_keys, &db, RANGEFILTERSIZE);


	vector<Bloom*> blooms = vector<Bloom*>();

	for (int i=0;i<=300;i++) {
		//Bloom b(DBDOMAIN,db.num_keys, &db, RANGEFILTERSIZE, i+1);
		if(i==0) {
			blooms.push_back(NULL);
			continue;
		}
		blooms.push_back(new Bloom(DBDOMAIN,db.num_keys, &db, RANGEFILTERSIZE, i));
	}



	int EXPQUERIES = 200000;

	   Query qGen(DBDOMAIN,1,MEAN,STDDEV,
	  		   1.2, EXPQUERIES, ZIPFQ,false,seed);




	    for (int j=0; j<EXPQUERIES; j++)
	   {

			Query::Query_t q = qGen.nextQuery();


			//printf("Query: %d - %d \n",q.left,q.right);

			

			bool dR = db.rangeQuery(q);
			bool qR = bl.handleQuery(q, dR);
			bool qRr = blooms[1]->handleQueryRanged(q.left, q.right, dR, &db);
			assert(qR == qRr);

			for(int i=1;i<blooms.size();i++) {
				Query::Query_t q2;
				q2.left = (q.left) - (q.left % blooms[i]->getCover());
				assert(q2.left % blooms[i]->getCover() == 0);
				q2.right = q2.left + MEAN -1;
				bool res = db.rangeQuery(q);
				bool res2 = blooms[i]->handleQueryRanged(q.left, q.right, res, &db);
				//if(i == 31 && res2 && !res)
				//	assert(1==0);
			}
			 fsyn.handle_query(q.left, q.right, dR, false);

	   }

	    double min = 100;
	    int min_cover = -1;
	for (int i = 1; i < blooms.size(); i++) {

		printf("-------------\n");
		printf("Ranged bloom with r = %d \n", i+1);
		blooms[i]->stats.print();
		cout<<"expected error rate from math:"<<blooms[i]->fpRate()<<endl;
		

		if(blooms[i]->stats.getFpr()<min) {
			min = blooms[i]->stats.getFpr();
			min_cover = blooms[i]->getCover();
		}
	}

	printf("Synopsis:\n");
	fsyn.stats.printFp();
	printf("Smaller bloom: %f  (%d th) with mean = %d \n", min, min_cover, MEAN);

	cout<<"plain bloom:"<<endl;
	bl.stats.print();
	 cout<<"expected error rate from math:"<<bl.fpRate(MEAN)<<endl;


	 fprintf(stderr,"%f\n", fsyn.stats.getFpr());
	for(int i=1;i<blooms.size();i++) {

		fprintf(stderr,"%d\t%f\n",i,blooms[i]->stats.getFpr());
	}

	for(int i=1;i<blooms.size() ; i++) {
		free(blooms[i]);
	}




}



int main(int argc, char* argv[]) {

    int seed = 0;
    if(argc == 3)
	seed = atoi(argv[2]);
    testTrivial(atoi(argv[1]), seed);
    return 1;

	Database db(128);
	Uniform unif(128);
    db.addValue(2);
    db.addValue(3);
    db.addValue(15);
    db.addValue(15);
    db.addValue(115);
    db.plot();

    Bloom bl(127, db.num_keys, &db, 1000, 10);

    assert(bl.pointQuery(0) == true);
    assert(bl.pointQuery(1) == true);
    assert(bl.pointQuery(11) == true);
    assert(bl.pointQuery(12) == false);
    assert(bl.pointQuery(2) == false);
    assert(bl.pointQuery(5) == false);

   assert(bl.handleQueryRanged(2,2,false, &db) == true);
   assert(bl.handleQueryRanged(6,6,false, &db) == true); //even though that's wrong
   assert(bl.handleQueryRanged(8,14,false, &db) == true);
   assert(bl.handleQueryRanged(15,18,false, &db) == true);
   assert(bl.handleQueryRanged(20,21,false, &db) == false);
   assert(bl.handleQueryRanged(19,19,false, &db) == true);
   assert(bl.handleQueryRanged(20,109,false, &db) == false);
   assert(bl.handleQueryRanged(95,109,false, &db) == false);
   assert(bl.handleQueryRanged(115,130,false, &db) == true);
   assert(bl.handleQueryRanged(125,130,false, &db) == false);



 GREEN_COLOR
 printf("ALL TESTS PASS :-) \n");
 WHITE_COLOR


}

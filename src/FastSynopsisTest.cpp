/*
 /* FastSynopsisTest.cpp
 *
 *  Created on: Jun 6, 2013
 *      Author: car
 */

#include "Bloom.h"
#include "Synopsis.h"
#include "Synopsis/FastSynopsis.h"
#include <stdio.h>
#include "Query.h"
#include <vector>
#include <assert.h>
#define GREEN_COLOR printf("\033[01;32m");
#define WHITE_COLOR printf("\033[01;37m");
using namespace std;

void testMerge() {

	int DBDOMAIN = 127;
		Database db(DBDOMAIN+1);

		vector<int> db_values = vector<int>();


	    db_values.push_back(2);
	    db_values.push_back(3);
	    db_values.push_back(15);
	    db_values.push_back(15);


	    for(int i=0;i<db_values.size();i++) {
	    	db.addValue(db_values[i]);
	    }
	    db.plot();


	    FastSynopsis fsyn(0, DBDOMAIN, &db);

	    assert(fsyn.size == 2);
	    fsyn.sanity_check();
	    fsyn.learn_from_fp(64, 127);
	    fsyn.sanity_check();
	    assert(fsyn.size == 2 + 2);
	    fsyn.sanity_check();
	    fsyn.merge(fsyn.root->left_child, fsyn.root->right_child);
	    fsyn.sanity_check();
	    assert(fsyn.size == 2);
	    fsyn.sanity_check();
	    fsyn.learn_from_fp(0, 31);
	    fsyn.sanity_check();

	    assert(fsyn.size == 2+1+2+2);
	    fsyn.sanity_check();
	    fsyn.truncate(5);
	    fsyn.sanity_check();
	    assert(fsyn.size == 4);
	    assert(fsyn.handle_query(0,31,false,false) == true);
	    printf("[ %d - %d ] \n", fsyn.last_evicted_left, fsyn.last_evicted_right);
	    assert(fsyn.start_over == false);

}


int main() {


	testMerge();



	int DBDOMAIN = 127;
	Database db(DBDOMAIN+1);

	vector<int> db_values = vector<int>();


    db_values.push_back(2);
    db_values.push_back(3);
    db_values.push_back(15);
    db_values.push_back(15);
    db_values.push_back(115);

    for(int i=0;i<db_values.size();i++) {
    	db.addValue(db_values[i]);
    }
    db.plot();


    FastSynopsis fsyn(0, DBDOMAIN, &db);

    /* check that the navigate thing works */
    for(int i=0;i<=DBDOMAIN;i++) {
    	assert(fsyn.handle_query(i,i,true) == true);
    }

    printf("Size of synopsis (default): %d\n",fsyn.size);

    //learning from false positives //

    for(int i=0;i<=DBDOMAIN;i++) {
    	if(!db.contains(i))
    		assert(fsyn.handle_query(i,i,false) == true);
    }

    printf("Size of synopsis now: %d\n",fsyn.size);

   for(int i=0;i<db_values.size();i++) {
        	int val = db_values[i];
        	assert(fsyn.handle_query(val,val,true) == true);
        	if(val-2>=0)
        		assert(fsyn.handle_query(val-2,val,true) == true);
        }

   assert(fsyn.handle_query(0,1,false) == false);

   assert(fsyn.getLeaf(0)->used_counter == 1);
   assert(fsyn.getLeaf(1)->used_counter == 1);
   assert(fsyn.handle_query(4,14,false) == false);
   assert(fsyn.getLeaf(12)->used_counter == 1);
   assert(fsyn.getLeaf(13)->used_counter == 1);
   assert(fsyn.handle_query(11,13,false) == false);

   assert(fsyn.getLeaf(12)->used_counter == 2);
   assert(fsyn.getLeaf(13)->used_counter == 2);
   assert(fsyn.getLeaf(11)->used_counter == 2);
   assert(fsyn.getLeaf(14)->used_counter == 1);
   assert(fsyn.getLeaf(4)->used_counter == 1);
   assert(fsyn.getLeaf(8)->used_counter == 1);
   assert(fsyn.handle_query(16,20,false) == false);
   assert(fsyn.handle_query(10,20,false) == true);


   // now moar tests //
   int DBDOMAIN2 = (1<<23) -1;
   Database db2(DBDOMAIN2+1);
   Uniform  unif(DBDOMAIN2 + 1);
   db2.populate(30000, &unif);
   db2.plot();

   FastSynopsis fsyn2(0, DBDOMAIN2, &db2);
   FastSynopsis fsyn3(0, DBDOMAIN2, &db2);
   /*Synopsis syn(DBDOMAIN, 1000, true, &db2,CLOCK);
   syn.init();
   syn.setDatabase(&db2);
   syn.perfect(&db2);*/
   fsyn3.perfect(&db2);

   int QUERIES = 100000;
 //  int QUERIES = 200;


   Query qTrain1(DBDOMAIN2, 0, 8, 13,
    		  1.1, QUERIES,
    		  false,false);




      for (int i=0; i<QUERIES; i++) {
        Query::Query_t q = qTrain1.nextQuery();

        if(i % 100 == 0)
        	printf("%d out of %d\n", i, QUERIES);

        bool qR = db2.rangeQuery(q);
        fsyn2.handle_query(q.left, q.right, qR);
      // syn.handleQuery(q,&db,true,qR);
        bool sR3 = fsyn3.handle_query(q.left, q.right, qR);
        assert(qR == sR3);
        assert(fsyn3.stats.fp == 0);

      }
      assert(fsyn3.stats.tp == fsyn2.stats.tp);

      //then assert that for this workload all the queries are graet :D
      	  qTrain1.reset();
      	  fsyn2.stats.reset();

      for (int i=0; i<QUERIES; i++) {
        Query::Query_t q = qTrain1.nextQuery();

        bool qR = db2.rangeQuery(q);
        bool  sR = fsyn2.handle_query(q.left, q.right, qR, false);
        assert(qR == sR);
        assert(fsyn2.stats.fp == 0);
      }

      printf("size: %d\n", fsyn2.size);

      int prev = fsyn2.size;

      fsyn2.truncate(fsyn2.size/2);
      printf("size: %d\n", fsyn2.size);
      assert(fsyn2.size<=prev/2);
      fsyn2.sanity_check();



 GREEN_COLOR
 printf("ALL TESTS PASS :-) \n");
 WHITE_COLOR


}

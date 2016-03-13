/*
 * test.cpp
 *
 *  Created on: Oct 8, 2012
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
#include <sstream>
#include <unistd.h>


int main()
{
	  int DBDOMAIN = 127;
	  bool ZIPFDATA = 0;
	  Database db(DBDOMAIN+1);
	  Zipf zipf(DBDOMAIN+1,1.1);
	  Uniform unif(DBDOMAIN+1);
	  if(ZIPFDATA)
	  	db.populate(10, &zipf);
	  else
		db.populate(10, &unif);
	 // print_config();
	  db.plot();
	  Query::Query_t q;
	  q.left = 0;
	  q.right =DBDOMAIN;
	  Synopsis0bit syn0(DBDOMAIN, 100, true,NULL);
	  syn0.setDatabase(&db);
	  syn0.init();
	  syn0.exportDatabase();
	  int numQ = 15;

	  Query qTrain1(DBDOMAIN, 0,3,10,
	 		  1.1, numQ,
	 		  true,false);
	   for (int i=0; i<numQ; i++) {
	     Query::Query_t q = qTrain1.nextQuery();
	     bool qR = db.rangeQuery(q);
	     syn0.handleQuery(q,&db,true,qR);

	   }
	   syn0.truncateClock(30);
	   syn0.truncateClock(20);



	return 0;
}

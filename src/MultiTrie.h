/* ---------------------------------------------------------------------------
 *                               CLASS INTERFACE
 *
 *   CLASS NAME:        Synopsis
 *
 *   CLASS KEY :        Synopsis
 *
 *   DESCRIPTION:       Simulates a synopsis that determines whether a range
 *                      query possibly has a result. As a synopsis, we use a trie
 *                      that partitions the domain in half and keeps a bit per
 *                      partition indicating whether tuples are in that domain.
 *
 * ---------------------------------------------------------------------------
 */

#include <vector>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <sys/types.h>
#include "Util.h"
#include "Database.h"
#include "Query.h"
#include <string>
#include "Synopsis.h"
#include <algorithm>
/* Make sure that this file is included only once */
#ifndef MULTITRIE_H
#define MULTITRIE_H
#define MULTIDEBUG 1

/* PARTITION MODES */
#define AGNOSTIC 0
#define DATAAWARE 1
#define QUERYAWARE 2


using namespace std;

class MultiTrie {



 protected:
    struct interval
  {
   int low;
   int high;

  };
 int partitions;
 int domain;
 int bits;
 int snapshots;




 vector<int> sizes;
 Database * db;
 int fp;
 int tp;
 int tn;
 int cold_records;
 static const int MINSIZE = 18; //20

 /* replacement policy */
 int victim_trie;


 void partition_query();
 void partition_data();
 void partition_data2();
 void create_uniform_bounds();
 int getNumInRange(int startidx,int low,int high,vector<uint> & container);
 void partition_agnostic();
 long double getTotalTime();


 public:
  MultiTrie(int partitions,int partition_mode,int DBSIZE,int domain,int bits,Database *db,int repl_policy,int clock_bits =1);
  ~MultiTrie();
  bool handleQuery(Query::Query_t q,Database*,bool);
  bool handleQueryOld(Query::Query_t q,Database*,bool);
  void resetTime();
  void getStats();
  void truncateClockPartial(int);
  void truncateClock(int);
  void print(int idx);
  int getFp();
  void printFps();
   vector<Synopsis *> tries;
   uint64_t getQueryTime(int mode);
   int multiple;
   int lastMerged;
   int size();
   vector<interval> bounds;
   int clock_bits;
   int unused;
   list<uint64_t> lookupt;
   list<uint64_t> truncatet;
   list<uint64_t> adaptt;
   inline void set(int cl)
   {
	   for(int i=0;i<tries.size();i++)
		   tries[i]->set(cl);
   }
   inline void sanityCheckUsed()
   {
	   for(int i=0;i<tries.size();i++)
	  	   {
	  		   tries[i]->sanityCheckUsed();
	  	   }
   }
   inline void convert(int cl)
   {
	   for(int i=0;i<tries.size();i++)
	   {
		   tries[i]->convertToCompact(cl);
	   }
   }
   inline int leaves()
   {
	int sum = 0;
	for(int i=0;i<tries.size();i++)
	{
		sum+=tries[i]->Numleaves();
	}
	return sum;
   }

   inline int nodes()
      {
   	int sum = 0;
   	for(int i=0;i<tries.size();i++)
   	{
   		sum+=tries[i]->nodes();
   	}
   	return sum;
      }

  uint64_t getSum(int mode);
  uint64_t getStddev(int mode,int q);
  uint64_t getLookupMedian();



};


#endif

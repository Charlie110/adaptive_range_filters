/* ---------------------------------------------------------------------------
 *                               CLASS INTERFACE
 *
 *   CLASS NAME:        Bloom
 *
 *   CLASS KEY :        Bloom
 *
 *   DESCRIPTION:       Quick and dirty hack of a Bloom filter.
 *
 * ---------------------------------------------------------------------------
 */

#include <math.h>
#include "Util.h"
#include "Query.h"
#include "Database.h"
#include "Synopsis/Statistics.h"
#include <vector>
#include <list>
#include "curves/Curve.h"
#include <assert.h>


/* Make sure that this file is included only once */
#ifndef Bloom_H
#define Bloom_H

// Thetas for the hash functions (according to Knuth)
#define THETA0  sqrt(33) / 8
#define THETA1  (sqrt(5)-1) / 2
#define THETA2  sqrt(2) / 2
#define THETA3  sqrt(3) - 1
#define THETA4  (sqrt(7)-1) / 2
#define THETA5  (sqrt(8)-1) / 2
#define THETA6  sqrt(11) / 4
#define THETA7  sqrt(13) / 4
#define MAXHASHES 8




class Bloom {
 protected:

  // member variables
 protected:
  long size;        // size of the Bloom filter
  //bool* b;          // Synopsis  (array of Boolean)
  vector<bool> b;
  double*  theta;   // seeds for hash functions
  int  k;           // number of hash functions
  int noKeys; //number of keys - initially at least
  Curve * c;
  int range_covered;
  

  // public methods
 public:

  /* multi dimensional */
  Statistics stats;

  int getCover() {
	return this->range_covered;
}
  bool pointQuery(vector<int> pt);
  bool rangeQuery(Query::QueryMD_t q);

  void printStats(string desc) {

 	 cout<<desc<<endl;
 	 stats.print();
 	 int tn = stats.tn;
 	 int tp = stats.tp;
 	 int fp = stats.fp;
	GREEN_COLOR
	cout<<"Fp rate:"<<(fp*100.0)/(fp+tn)<<endl;
	 cout<<"Queries requiring access to cold store %:"<<(tp)*100.0/(fp+tn+tp)<<endl;
	WHITE_COLOR
  }

  void sanityCheck();

  bool handleQueryRanged(uint left, uint right,bool qR, Database *db);

 /* statistics */
  list<uint64_t> lookupt;
  inline void resetTime()
  {
	  lookupt.clear();
  }

  Bloom(uint domain, long noKeys, Database* db, long inSize);
  Bloom(uint domain, long noKeys, Database* db, long inSize,
		  Curve * curve);

  Bloom(uint domain, long noKeys, Database* db, long inSize,
		  int range);

  bool handleQuery(Query::QueryMD_t q,bool qR);
  bool handleQuery(Query::Query_t q,bool qR);
  ~Bloom();
  double fpRate();
  double fpRate(int);
  int getK();

  // simulates execution of range query on Synopsis
  // (may result false positive; must not result in false negative)
  // q.left == q.right must hold!!!  (q.right is ignored!)
  bool rangeQuery(Query::Query_t q);
  bool pointQuery(int key);

  // record a set of new keys
  void recordNewKeys(uint64* newKeys, long n) {
    for (long i=0; i<n; i++)
      recordNewKey(newKeys[i]);
  }  // recordNewKeys

 private:
  // record a single new key
  void recordNewKey(uint newK);

  // compute the h'th hash function on key
  long hash(uint key, int h) {
    double r = theta[h] * key - floor(theta[h] * key);
    long result = (long) floor(r * size);
    return result;
  }  // hash
};

#endif // Bloom_H

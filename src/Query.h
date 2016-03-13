/* ---------------------------------------------------------------------------
 *                               CLASS INTERFACE
 *
 *   CLASS NAME:        Query
 *
 *   CLASS KEY :        Query
 *
 *   DESCRIPTION:       Generates a random range query for a given domain.
 *                      Width of range is generated according to a normal distribution
 *                      with standard deviation 1 and user defined mean.
 *                      Position of range is generated using a uniform distribution.
 *
 *                      Logs all queries.  So, can be used to train with a set of
 *                      queries and then replay after training with the same set of
 *                      queries.
 *
 *                      After "noQueriesInWorkload" queries, rolls over to Query 0.
 *
 * ---------------------------------------------------------------------------
 */

#include "Util.h"
#include "Distribution.h"

#include "curves/DatagenMD.h"

/* Make sure that this file is included only once */
#ifndef Query_H
#define Query_H


class Query {
 public:
  struct Query_t {
    uint left;
    uint right;
  };  // Query_t


  struct QueryMD_t
  {
	  vector<int> low;
	  vector<int> high;
	  bool point;
  };
//typedef struct Query_t interval;

 protected:
  uint domain;      // size of the domain; all keys are in [0 ... domain[
  Query_t* log;     // query log for replay
  long n;           // number of queries in workload
  long logPos;      // current position in query log
  vector<QueryMD_t> logMD;


 public:
  // skew controls whether there is skew in the queries: i.e., more queries on the end of the domain
  // if the database is skewed towards the beginning of the domain, then skew=true will
  // result in queries in empty regions
  int getSize() {
	  return n;
  }
  Query(uint,int);
  Query(char *); // point queries from file
  Query()
  {
	  n = 0;
  }
  inline int numQueries()
  {
	  return n;
  }

 QueryMD_t convertToMD(Query::Query_t q);

  /* plain 1-dim queries */
  Query(uint inDomain, uint inMin,
		  double mean,double stddev,
		  double inZipf, long noQueriesInWorkload,
		  bool skew,bool old=false);
  double getAverageLength();
  /* multi dim queries (can have different domains/distributions per dimension) */
  Query(int dim,vector <Distribution *> distr,
  		int noQueriesInWorkload,
  		double mean,double stddev,bool skew);

  /* multi dim queries (same domains in each dimension) */
  Query(int dims, int domainPerDim,int noQueriesInWorkload,
		  double mean,double stddev,
		  double inZipf, bool skew);

  Query(vector<std::pair<int,int> > attr,int noQueriesInWorkload,
  		double mean,double stddev,double inZipf, bool skew,char correlation);
  /* special 2D corr/anti-corr dataset generation */

  double getAverageArea();
  Query(uint inDomain, uint inMin,double mean,double stddev,
  		double inZipf, long noQueriesInWorkload, bool skew,bool old,
  		int seed);

  Query(vector<int > domains,
		  int noQueriesInWorkload,double mean,double stddev,char correlation);
  ~Query();
  Query_t nextQuery();  // generate next query
  QueryMD_t nextQueryMD();
  void reset();         // reset replay at Query 0 - automatic after noQueriesInWorkload queries
  void prettyPrint(Query_t q);   // print the query
  void generateHotZipf(double zipf,double mean,double stddev);
  void generateUniform(double mean, double stddev);
};

#endif // Query_H

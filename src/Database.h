/* ---------------------------------------------------------------------------
 *                               CLASS INTERFACE
 *
 *   CLASS NAME:        Database
 *
 *   CLASS KEY :        Database
 *
 *   DESCRIPTION:       Simulates a database of n keys
 *                      a.) specifies a domain [0 ... domainSize[
 *                      b.) populates the domain with n random keys using
 *                          different distributions
 *                      c.) simulates the execution of a range query
 *                          true: if there exists a key in the range
 *
 * ---------------------------------------------------------------------------
 */

#include "Util.h"
#include "Query.h"
#include <vector>
#include <set>
#include "curves/Curve.h"
#include "curves/Zcurve.h"
#include "curves/Hcurve.h"
#include "curves/Ccurve.h"
#include "Database/PointGen.h"

/* Make sure that this file is included only once */
#ifndef Database_H
#define Database_H

class Database {
 protected:
  uint64 domain;    // size of the domain; all keys are in [0 ... domain[
  bool* db;       // db[k] == true if k is in the database

  int dims;



 public:
  uint64 getDomain() {
	  return this->domain;
  }
  Database(Database *d,int dimension);
  vector<Curve::attribute> attr;
  uint64 num_keys;
  Curve * curve;
  void addValue(uint64 val) {
	  set(val,true);
	  num_keys++;
  }

  bool exists(int val, int dim);
  bool exists(int low,int high, int dim);

  vector< vector<int> > getPoints();
  Database(uint64 inDomain,int dims=1);
  inline Curve * getCurve()
  {
	  return curve;
  }
  Database(vector<Curve::attribute> attr, char curve_type);

  Database(vector<Curve::attribute>,char curve_type,string filename,int size); /* from file */

  void populateMD(uint64 n,vector<Distribution *> distr); /* from distributions */
  void populateMD(uint64 n,vector<int> domains,char correlation); /*from corr/anti corr generator */

  Database(char * filename,uint64 inDomain);
  ~Database();

  void addPoint(vector<int> pt);


  void removeKeys(uint64);

  Database(Database * d, char curve_type);
  Database(vector<int> columns,string file, char curveType,
  		bool sameDimForAll,int multiplier);

  virtual bool contains(uint64);
  virtual void set(uint64,bool);
  void clear();
  void populate(uint64 n, Distribution * dist);
  void populateMD(uint64 n,Distribution * dist);
  void sanity_check();
  vector<Query::Query_t> determineEmptyRanges(Query::Query_t r);
  vector<Query::Query_t> determineEmptyRanges(Query::Query_t r,int low, int domain);
  uint64 * addKeys(uint64 n, Distribution * dist); //returns the added keys
  vector<uint64> getKeys(); //returns a (sorted) vector of all the keys present in the database

  // warning: if less than n items exist, then will delete only as many items as exist
  // deletes items according to a uniform distribution
  void deleteKeys(uint64 n);
  void deleteKeys(uint64 n, Distribution * dist);
  void plot();
  int size();
  bool rangeQuery(uint64 left, uint64 right);
  bool rangeQuery(vector<int> left, vector<int> right,int cdim = -1);
  bool rangeQuery(Query::Query_t q) { return rangeQuery(q.left, q.right); }
  uint64 getFirstResult(Query::Query_t q);
  uint64 getNextResult(Query::Query_t q, uint64 p);
  void prettyPrint();
  uint64 noValue() {return domain; }
};


class FastDB : public Database
{
protected:


public:
	 FastDB(uint64 inDomain);
	 bool contains(uint64);
	 void set(uint64,bool);
	 std::set<uint64> values;

};

#endif // Database_H

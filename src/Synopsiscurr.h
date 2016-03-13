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
#include <sstream>
#include <math.h>
#include <sys/types.h>
#include "Query.h"
#include "Database.h"
#include <string>
#include <list>

//#include "MultiTrie.h"

#ifndef Synopsis_H
#define Synopsis_H


//#define DOTRACE 1 //this creates a picture execution of the program
#define TRACEQUERY 0
#define TRACESPLIT 1
#define TRACEMARKEMPTY 2
#define TRACETRUEPOS 3
#define TRACEMERGE 4
#define TRACETRUNCATE 5
#define TRACEMARKUSED 6
#define TRACEEVAL 7
// don't use with large inputs -> major slow down and space used

/* Make sure that this file is included only once */

#define SYNDEBUG 1

#define RANDOM 0
#define CLOCK 1
#define LOOKUP 0
#define ADAPT 1
#define TRUNCATE 2

#define TICKTOCK 1
//#define MERGE 1
#define LEARNFROMTP
#define ONLYEMPTY 1
#define MARKUNUSED 1
#define DOTRUNCATE 1

#define STRAT0 0
#define STRAT1 1
#define STRAT2 2

using namespace std;

class Synopsis {

	 friend class MultiTrie;

 protected:
    struct state //history state
  {
    int lvl; //this can be optimized away but idc right now
    int idx;
    int actual_idx;
    bool wentLeft;

  };

  struct prefix
  {
    int idx;
    int prefix;
    int previdx;
    int prevfix;

  };

  struct node
  {
    bool exists;
    int low;
    int high;
    int lvl;
    int idx;
    int leaf_idx;
    int parent_idx;

   };

  struct gnode
  {
	  int lvl;
	  int idx;
	  string color;
  };



  int domain;      // size of the domain; all keys are in [low ... low+domain[
  int limit;       // maximum size of the synopsis
  int lowerb;
  int actual_size; //sometimes this is <domain
  vector<vector<bool> > shape;
  vector<vector<bool> > leaves;
  vector<vector<bool> > used;
  int num_bits;
  struct node prev_victim;
  bool use_tp;
  Database * db;
  Database *database;
  int clock_bits;

  int repl_policy;



  // public methods
 public:

 /* statistics */

 uint tp;
 uint tn;
 uint fp;
 uint snapshots;
 string outfolder;

 Query::Query_t current_query;
 vector<double> queryTimes;
 vector<double> truncateTimes;
 vector<double> adaptTimes;
 list<uint64_t> truncatet;
 list<uint64_t> lookupt;
 list<uint64_t> merget;
 list<uint64_t> falsepos;



  Synopsis(int domain,int limit,bool tp,Database * db,int repl_policy, int low = 0);
  ~Synopsis();
  Synopsis(Synopsis * s, vector< vector<int> > weights,int clock_bits=1);

  int getScore(int low,int high,Database *); /*DEBUG - a metric to say how relevant the data is*/
  int getReplPolicy();
  int id;
  void perfect(Database *);
  inline void set(int clock_bits)
  {
	  this->clock_bits = clock_bits;
	  if(clock_bits ==0)
	  {
		  for(int i=0;i<used.size();i++)
		  {
			  used[i].clear();
		  }
		  used.clear();
	  }
	  this->num_bits = this->size();
  }

  bool handleQuery(Query::Query_t q,Database * db,bool doAdapt,bool dbResult); //DO EVERYTHING + maybe return time elapsed for trie-related stuff

  //bool rangeQuery(Query::Query_t q);
  bool pointQuery(int key);
  bool rangeQuery(Query::Query_t q);
  bool rangeQueryOpt(Query::Query_t q);
  bool navigateOpt(int key,int lvl,int idx,int actual_idx,vector<Synopsis::prefix> * levels,int*,int*);
  //node navigateOpt(int key,int lvl,int idx,int actual_idx,vector<Synopsis::prefix> * levels);
  vector<prefix> createPrefixHelper();

  node navigateOpt(int key,int lvl,int idx,int actual_idx,vector<Synopsis::prefix> * levels);




  long double getQueryTime(int mode);
  void setUpperBound(int b);
  void recordNewKeys(uint * keys,int num_keys, int strategy=STRAT0);
  int nodes();
  void resetTime();
  int getMaxVal();
  int size();
  void setDatabase(Database * db);
  void resetUsed();

  /* visualization */
  void prettyPrint();
  void exportGraphViz(string filename,vector<pair<int,int> > special_nodes = vector <pair<int,int> >(0) );
  string exportQuery(Query::Query_t q,Database * db,bool sR,bool qR, string file);
  void takeSnapshot(vector<pair<int,int> > special_nodes,int mode=0);
  string exportText(string text);
  void takeTextSnapshot(string text);
  void writeGraph(string);
  void exportDatabase();

  //for refinement
  void merge(); //merges sibling leaves with the same value
  void learn_from_tp(vector<Query::Query_t> * empty_ranges);
  void learn_from_tn(Query::Query_t r);
  void learn_from_fp(Query::Query_t q); //expand!
  void expand(int key,bool isLeft, bool expandAnyway = false);
  //for Clock
  void truncateClock(int n);
  void truncateRandom(int n);
  virtual void init();
  void setId(int);
  void resetFp();
  int Numleaves();
  virtual int getUsed(int lvl,int idx);
  inline int height()
  {
	  return shape.size();
  }
 protected:

  struct node navigate(int key,int lvl,int idx,int actual_idx,vector<struct state> * history,bool keepHist=true);
  bool navigate(int key,int lvl,int idx,int actual_idx); //optimized navigate
  void merge(int lvl,int idx);
  //helper
  bool isLastLevel(int lvl);
  bool isLogicalLeaf(int lvl,int idx,int& left_child,bool assignLeft);
  bool isRedundant(int lvl,int idx, int &left_child,bool assign);
  int getLeftChild(int parent_lvl,int parent_idx);
  int getLeftChild(int parent_lvl,int start_idx,int parent_idx);
  int getRightChild(int left_child_idx, int parent_lvl,int parent_idx);
  int lowMiddle(int low, int high);
  void getRange(int lvl,int idx, int * low, int * high);
  void getSingleRange(int lvl,int idx, int * low, int * high);
  int getLeafOffset(int lvl,int idx);
  state getRestartPoint(int bound,vector<state> &hist);

  bool isLeaf(int lvl,int idx);
  bool getLeaf(int lvl, int node_idx,int& offset, bool assignOffset);
  bool getLeaf(int lvl, int node_idx,int offset);
  bool getLeafValue(int lvl, int leaf_idx);
  //helper MODIFICATIONS
  void split(int lvl,int idx,bool l,bool r,int *lidx, int *ridx);

  void setValue(int lvl,int idx,bool val);
  void setLeafValue(int lvl,int idx,bool val);
  void insertValue(int lvl,int idx,bool val);
  virtual void insertLeafValue(int lvl,int idx,bool val);
  void removeValue(int lvl,int idx);
  virtual void removeLeafValue(int lvl,int idx);
  virtual void grow();
  int getParentIdx(int child_lvl,int left_idx);
  void mergeOpt(int lvl,int idx, vector<prefix> * levels);
  void merge(int lvl, int left_idx, int parent_idx);
  //marks leaf as either empty or used (depending on the value of markEmpty)
uint mark_leaf(Query::Query_t r,int lvl,int idx,int actual_idx,vector<Synopsis::state> * history,bool markEmpty,vector<Synopsis::node> & marked, bool falsePos = false);
  void merge_marked(vector<node> marked);
  //helper REPLACEMENT POLICY
  virtual void incrementUsed(int lvl, int leaf_idx);
  virtual void decrementUsed(int lvl, int leaf_idx);

  virtual void setUsed(int lvl,int leaf_idx,int val);
  void evict_victims(int,bool *);
  bool evict_victims(int lvl, int idx,int plvl, int pidx,bool * visited_prev,int target_size, vector<prefix> * levels);
  void print_previous_victim();
  bool evict(int lvl,int idx,int left_child,int right_child,bool left,bool right,int target_size,vector<prefix> * levels);
  void killRandom(int);
  void resetClock();
  bool evict_top(int plvl,int pidx,bool *visited,int target_size);




 //debug | visualize
  void print_vector(vector<bool> v,bool newline);
  void prettyPrint(int lvl, vector<int> positions);

  void buildGraph(int lvl,int idx,int actual_idx,string & graph, vector<pair<int,int> > special_nodes, int mode=0 );
  void verify_tn(Query::Query_t q); //verify that the used bits have been set properly
  int getEmptyLeaves();
  void countEmpty(int lvl,int idx,int * count);
  void sanityCheckUsed();



};

class Synopsis2bit : public Synopsis
{
  public:
  Synopsis2bit(int domain,int limit,bool tp,Database * db,int low=0);
  ~Synopsis2bit();
  void init();
  int getUsed(int lvl,int idx);
  protected:
  void incrementUsed(int lvl, int leaf_idx);
  void decrementUsed(int lvl, int leaf_idx);

  void setUsed(int lvl,int leaf_idx,int val);
  void insertLeafValue(int lvl,int idx,bool val);
  void removeLeafValue(int lvl,int idx);


};

class Synopsis0bit : public Synopsis
{
  public:
  Synopsis0bit(int domain,int limit,bool tp,Database * db,int low =0);
  ~Synopsis0bit();
  Synopsis0bit(Synopsis * s, vector< vector<int> > weights);
  void init();
  int getUsed(int lvl,int idx);
  protected:
  void incrementUsed(int lvl, int leaf_idx);
  void decrementUsed(int lvl, int leaf_idx);

  void setUsed(int lvl,int leaf_idx,int val);
  void insertLeafValue(int lvl,int idx,bool val);
  void removeLeafValue(int lvl,int idx);
  void grow();


};

class SynopsisIntClock : public Synopsis
{
protected:

  public:

	vector< vector<int> > weights;
  SynopsisIntClock(int domain,int limit,bool tp,Database * db,int low =0);
  ~SynopsisIntClock();
  int getUsed(int lvl,int idx);
  void init();
  void convertToCompact(int clockbits);
  protected:
  void incrementUsed(int lvl, int leaf_idx);
  void decrementUsed(int lvl, int leaf_idx);
 // int getUsed(int lvl,int idx);
  void setUsed(int lvl,int leaf_idx,int val);
  void insertLeafValue(int lvl,int idx,bool val);
  void removeLeafValue(int lvl,int idx);
  void grow();



};

#endif // Synopsis_H

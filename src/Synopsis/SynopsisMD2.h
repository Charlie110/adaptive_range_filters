/*
 * SynopsisMD2.h
 *
 *  Created on: Dec 10, 2012
 *      Author: carolinux
 */
#include "../Synopsis.h"
#include "../Query.h"
#include "Statistics.h"
#include <algorithm>
#ifndef SYNOPSISMD2_H_
#define SYNOPSISMD2_H_
#define LESS -1
#define EQUAL 0
#define GREATER 1
#define REACHED_END -1

using namespace std;
class SynopsisMD2 {

	int snapshots;
	string outfolder;
	uint domain;
	int dim;
	int lowerb;
	vector<vector<bool> > shape;
	vector<vector<bool> > leaves;
	vector<vector<bool> > used;
	vector<vector<int> > weights;
	vector<int> lowp;
	vector<int> highp;
	int clock_bits;
	Database *db;
	bool mergeEqual;

	int num_bits;
	vector<int> domains;
	vector<int> bits;




	bool alternatingDims;
	bool onlyEmptyUsed;



public:
	int dim_bits;
	Statistics stats;
	bool currentMark;

	struct depth
	{
		int d; //dimension idx;
		int max;
	};

	struct history
	{
		 vector<int> low;
		 vector<int> high;
		 bool wentLeft;
		 int split_dim;
		 int middle;
	};


	struct nodeMD
	  {
	    bool exists;
	    vector<int> low;
	    vector<int> high;
	    int lvl;
	    int idx;
	    int leaf_idx;
	    int parent_idx;
	    int actual_idx;
	   /* int dim;
	    int dimp;
	    bool is_left_sibling;*/

	   };

	struct nodeMD prev_victim;
	struct nodeMD curr_leaf;
	Query::QueryMD_t cq;
	vector<int> maxdepths;

	int size();
	int ranges_seen;



	inline void set(int u)
	{
		used_bit_size = u;
		num_bits = size();
	}

	int used_bit_size;
	int id;

	void convert(int bits)
	{
		clock_bits = bits;
	}

	void perfect(Database *database);
	bool dcompare(struct depth a, struct depth b);
	int getDimensionAlt(int lvl);
	SynopsisMD2(int dim,Database * db,vector<int> highp,vector<int> lowp,int clock_bits);
	bool pointQuery(vector<int> point);
	int getMaxDepth(int dimension);
	void  printDims();
	int getNextDimension(int d);
	int  getDimension(int lvl,int idx);
	int getMiddle(vector<int> low,vector<int> high,int idx);
	bool lessorEqual(vector<int> pt,int middle,int dim_idx);
	bool lessin1dim(vector<int> pt,vector<int> high);
	bool getLeaf(int lvl, int node_idx);
	void getMidpoints(vector<int> low,vector<int> high,int middle,int d,
			vector<int> * midpoint,vector<int> * midpoint2);
	bool isEqual(vector<int> a,vector<int> b);
	void mark_empty(int lvl, int idx);
	bool handleQuery(Query::QueryMD_t q, bool doAdapt,bool qR);
	int qcompare(vector<int> a,vector<int> b);
	//void expand(vector<int> key,bool isLeft);
	void carve(struct nodeMD res,vector<int> clow,vector<int> chigh,
			vector<int> low,vector<int> high,int d);
	int getNextDimension(int lvl,vector<int> x,vector<int> y,
			vector<int> low,vector<int> high,int cdim);
	void carve_alt(struct nodeMD res,vector<int> low,vector<int> high);
	bool expandQueryBox(vector<int> low,vector<int> high);
	bool inBox(vector<int> pt,vector<int> low,vector<int> high);
	bool inBox(vector<int> clow,vector<int> chigh,vector<int> low,vector<int> high);
	void  printLeaf(vector<int> low,vector<int> high);
	bool overlap(vector<int> x,vector<int> y,vector<int> low,vector<int> high);
	bool dimInBox(vector<int> x,vector<int> y,vector<int> low,vector<int> high,int dim);
	int getLeafValue(int lvl,int leaf_idx);
	void learn_from_fp(vector<int> low, vector<int> high);
	vector<int> getNextLeaf(vector<history> hist, vector<int> last,
										 vector<int> high);
	bool rangeQuery(vector<int> low,vector<int> high);
	int getLevels();
	bool getLeaf(int lvl, int node_idx,int & leaf_offset,bool uncomputed);
	void mark_empty(vector<int>& low,vector<int>& high);
	void print_point(vector<int> pt);
	struct nodeMD navigate(vector<int> point, vector<history> & hist);
	void insertDimension(int lvl,int idx, int dimension);
	void split(int lvl, int idx,bool left,
			bool right,int *lidx,int *ridx,int existing_leaf_idx,int split_dim);
	void setValue(int lvl,int idx,bool val);
	void setLeafValue(int lvl,int leaf_idx,bool val);
	 void insertValue(int lvl,int idx,bool val);
	 void removeValue(int lvl,int idx);
	void removeLeafValue(int lvl,int idx);
	bool navigate(vector<int> point);
	void insertNode(int lvl, int idx, vector<bool> internal, vector<bool> leafs);
	void insertLeafValue(int lvl,int idx, bool val);
	int getLeftChild(int parent_lvl,int parent_idx);
	bool isLeaf(int lvl,int idx);
	int getLeafOffset(int lvl,int idx);
	void grow();
	virtual ~SynopsisMD2();
	void setClockBits(int);
	inline void setAlternate()
	{
		alternatingDims = true;
		dim_bits = 0;
	}

	bool getBit(int lvl, int idx);

	bool leafBit(int lvl,int idx);

	bool lessorEqual(vector<int> pt, vector<int> thres);
	bool less(vector<int> pt, vector<int> thres);
	bool greaterorEqual(vector<int> pt, vector<int> thres);
	bool greater(vector<int> pt, vector<int> thres);
	bool dimensionBit(int idx);
	/* deescalation-related functions */

	void learn_from_tn(vector<int> low,vector<int> high);
	void mark_used(vector<int> low,vector<int> high);
	void incrementUsed(int lvl,int leaf_idx);
	void setUsed(int lvl,int leaf_idx,int val);
	void decrementUsed(int lvl,int leaf_idx);
	int getUsed(int lvl,int leaf_idx);

	/* deesc - truncate */
	bool evict_victims(int lvl, int idx,int plvl, int pidx,
			bool * visited_prev,int target_size);
	bool evict_lleaf(int lvl,int idx,int left_child,
			int leaf_idx,bool left,bool right,int target_size);
	void merge(int parent_lvl, int left_idx, int parent_idx);
	void truncate(int n);
	void truncate(int n,bool * visited_prev);
	bool evict_top(int plvl,int pidx,bool *visited,int target_size);
	bool isLogicalLeaf(int lvl,int idx,int & left_child,bool assign);
	void print_clock();
	void merge_equal();





	/*in file SynopsisVizMD (visualization functions); */
	void sanityCheck1();
	void takeTextSnapshot(string text);
	void takeSnapshot(string text);
	string firstNode();
	void buildGraph(int lvl,int idx,vector<int> low, vector<int> high,string & graph);
	string getVector(vector<int> pt);
	void writeGraph(string text);
	void exportGraphViz(string file,string text);
	string getQueryText();
	string exportText(string text);





};

#endif /* SYNOPSISMD2_H_ */

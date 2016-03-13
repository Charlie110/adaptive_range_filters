/*
 * QuadSynopsis.h
 *
 *  Created on: Dec 14, 2012
 *      Author: carolinux
 */


#include "../Database.h"
#include "../Query.h"
#include "SynopsisMD2.h"
#include "Statistics.h"
#ifndef QUADSYNOPSIS_H_
#define QUADSYNOPSIS_H_

#define QGREATER 1
#define QLESS 	 -1
#define QEQUAL 0
#define QNO_COMP 2


class QuadSynopsis {

	vector<vector<bool> > shape;
	vector<vector<bool> > leaves;
	vector<vector<bool> > used;
	vector<vector<int> > weights;

	struct SynopsisMD2::nodeMD prev_victim;
	struct  SynopsisMD2::nodeMD curr_leaf;

	Query::QueryMD_t cq;
	int ranges_seen;


public:
	int dim;
	int cdim;
	int num_bits;
	bool onlyEmptyUsed;
	Database *db;
	vector<int> lowp,highp;
	int clock_bits;
	int id,snapshots,used_bit_size;
	string outfolder;
	vector<int> domains;
	Statistics stats;
	bool onedimspecial;
	int numc;
	bool mergeEqual;

	inline void setSpecial(int children)
	{
		onedimspecial = true;
		numc = children;

	}
	bool currentMark;

	int size();

	int getUnused();
	int unused;


	inline void set(int u)
	{
		used_bit_size = u;
		num_bits = size();
	}

	void convert(int bits)
	{
		clock_bits = bits;
	}


	/* functions in escalate.cpp */

	void perfect(Database *);

	bool handleQuery(Query::Query_t q,
			bool doAdapt,bool qR);
	bool sanityCheck2();
	bool sanityCheck();
	bool sanityCheck(int lvl,int idx, vector<int> clow, vector<int> chigh);

	void split(int lvl, int idx,
			vector<bool> leaf_vals,int *fidx,int existing_leaf_idx);
	void setLeafValue(int lvl,int leaf_idx,bool val);
	void mark_empty(int lvl,int idx);
	void expandQueryBox(vector<int> low, vector<int> high);
	void expandQueryBox(int lvl, int idx,
			vector<int> lowpt, vector<int> highpt,
			vector<int> low, vector<int> high);
	void learn_from_fp(vector<int> low, vector<int> high);
	bool handleQuery(Query::QueryMD_t q, bool doAdapt,bool qR);
	void carve(int lvl,int idx,vector<int> clow,vector<int> chigh,
			vector<int> low,vector<int> high);
	bool exceedsAllDims(vector<int> key, vector<int> threshold);
	bool lessorEqual(vector<int> key, vector<int> threshold);



	/* misc functios */

	void setValue(int lvl,int idx,bool val);
	void print_point(vector<int> pt);

	bool rangeQuery(vector<int> low,vector<int> high);
	bool overlap(vector<int> lowpt,vector<int> highpt,
			vector<int> low,vector<int> high);
	bool rangeQuery(int lvl,int first_idx,
			vector<int> lowpt, vector<int> highpt,
			vector<int> low,vector<int> high);

	void removeLeafValue(int lvl,int idx);
	void insertValue(int lvl,int idx,bool val);
	void insertLeafValue(int lvl,int idx, bool val);
	void removeValue(int lvl,int idx);
	int getLeafOffset(int lvl,int idx);
	void grow();
	bool isLeaf(int lvl,int idx);
	bool getLeaf(int lvl, int node_idx);
	bool isContained(vector<int> lowpt, vector<int> highpt,
			vector<int> lowbox, vector<int> highbox);
	bool getLeafValue(int lvl,int idx);
	int getFirstChild(int parent_lvl,int parent_idx);
	int compareAllDim(vector<int> key, vector<int> threshold);
	int compare1Dim(vector<int> pt, vector<int> threshold,int d);
	vector<int> getMiddles(vector<int> low, vector<int> high);
	bool getBit(int nr,int idx);
	vector<vector<int> > getChildrenUpperBounds(vector<int> low,vector<int> high);
	vector<vector<int> > getChildrenLowerBounds(vector<int> low,vector<int> high);





	inline int numChildren()
	{
		if(!onedimspecial)
			return pow2(dim);
		else
			return numc;
	}

	inline bool pointQuery(vector<int> key)
	{
		return navigate(key);
	}

	bool navigate(vector<int> key);

	inline int pow2(int x)
	{
		int res=1;
		while(x>0)
		{
			res*=2;
			x--;
		}
		return res;
	}
	QuadSynopsis(int dim,Database * db,
			vector<int> highp,vector<int> lowp,int clock_bits,
			int numc = 0);
	virtual ~QuadSynopsis();

	/* in file QuadSynopsisdeescalate.cpp */
	void incrementUsed(int lvl,int leaf_idx);
	void setUsed(int lvl,int leaf_idx,int val);
	void decrementUsed(int lvl,int leaf_idx);
	int getUsed(int lvl,int leaf_idx);
	void learn_from_tn(vector<int> low,vector<int> high);
	void mark_used(int lvl, int first_idx,
			vector<int> lowpt, vector<int> highpt,
			vector<int> clow, vector<int> chigh);

	/* related to truncate in deesc.cpp */
	bool evict_victims(int lvl, int idx,int plvl, int pidx,
				bool * visited_prev,int target_size);
		bool evict_lleaf(int lvl,int idx,int left_child,
				int target_size);
		bool mergeable(int lvl, int first_idx);
		void merge(int parent_lvl, int first_idx, int parent_idx,int first_leaf_idx);
		void truncate(int n);
		void truncate(int n,bool * visited_prev);
		bool evict_top(int plvl,int pidx,bool *visited,int target_size);
		bool isLogicalLeaf(int lvl,int idx,int & left_child,bool assign);
		void merge_equal();
		bool all_equal(int lvl, int first_leaf_idx);

	/* in file QuadViz.cpp */

	void takeTextSnapshot(string text);
	void takeSnapshot(string text);
	string firstNode();
	void buildGraph(int lvl,int idx,vector<int> low, vector<int> high,string & graph);
	string getVector(vector<int> pt);
	void writeGraph(string text);
	//void exportGraphViz(string file,string text);
	string getQueryText();
	string exportText(string text);


};

#endif /* QUADSYNOPSIS_H_ */

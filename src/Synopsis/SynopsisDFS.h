/*
 * SynopsisDFS.h
 *
 *  Created on: Nov 19, 2012
 *      Author: carolinux
 */

#include "../Util.h"
#include <vector>
#include <list>
#include "../Synopsis.h"
#include "../Query.h"
#include <sched.h>

#ifndef SYNOPSISDFS_H_
#define SYNOPSISDFS_H_

class SynopsisDFS {

private:

	vector<bool> shape;
	vector<bool> leaves;


	uint domain;
	uint lowerb;

	void serialize(Synopsis&,int,int);
	void buildGraph(string & dotty);
	bool isLeaf(int);
	bool leafValue(int);
	int skipSubtree(int,int*);
	bool pointQuery(uint k,int*);

public:
	list<uint64_t> lookupt;
	int fp;

	SynopsisDFS(Synopsis & syn);
	virtual ~SynopsisDFS();
	bool pointQuery(uint k);
	bool rangeQuery(uint low, uint high);
	void exportGraphViz(string file);
	bool handleQuery(Query::Query_t q);
};

#endif /* SYNOPSISDFS_H_ */

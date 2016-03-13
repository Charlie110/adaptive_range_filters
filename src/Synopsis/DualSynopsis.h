/*
 * DualSynopsis.h
 *
 *  Created on: Jan 22, 2013
 *      Author: carolinux
 */
#include <vector>
#include "QuadSynopsis.h"
#include "Statistics.h"

#ifndef DUALSYNOPSIS_H_
#define DUALSYNOPSIS_H_

class DualSynopsis {
public:
	int dim;
	DualSynopsis(int dim,Database * db,
			vector<int> highp,vector<int> lowp,int clock_bits);
	virtual ~DualSynopsis();
	vector<QuadSynopsis * > syn;
	vector<Database *> dbs;
	Database *db;
	Statistics  stats;

	bool handleQuery(Query::QueryMD_t q, bool doAdapt);
	void truncate(int size);
	void convert(int bits);
	void reset();
	void perfect();
};

#endif /* DUALSYNOPSIS_H_ */

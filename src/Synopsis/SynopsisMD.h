/*
 * SynopsisMD.h
 *
 *  Created on: Nov 23, 2012
 *      Author: carolinux
 */

#include "../Synopsis.h"
#include "../Query.h"
#include "../curves/Curve.h"
#include "../curves/Zcurve.h"
#include "../curves/Hcurve.h"
#include "../curves/Ccurve.h"
#include "Statistics.h"
#include <iostream>
#include <fstream>

#ifndef SYNOPSISMD_H_
#define SYNOPSISMD_H_

class SynopsisMD {
public:
	Synopsis * syn;
	Curve * curve;
	Database * db;
	char curve_t;
	int fp;
	int tn;
	int tp;
	int dim;
	Statistics stats;
	SynopsisMD(vector<Curve::attribute> attr,char curveType,int filterSize,Database *db);
	virtual ~SynopsisMD();
	bool pointQuery(vector<int>);
	bool isPointQuery(vector<int> low, vector<int> high);
	bool handleQuery(vector<int> low, vector<int> high,bool doAdapt);
	bool handleQuery(Query::QueryMD_t q,bool doAdapt);

	void takeSnapshot()
	{
		syn->takeSnapshot(vector<std::pair<int,int> >());
	}
	inline void truncate(int b)
	{
		return truncateClock(b);
	}
	void set(int bits);
	void convert(int bits);
	bool rangeQuery(Query::Query_t zq);
	bool rangeQuery(vector<int> low, vector<int> high);
	void listValues();
	string exportCurve(bool useDB = false);
	void exportCurveFile(string,bool useDB = false);
	void truncateClock(int);
	void reset()
	{
		fp = 0;
		tn = 0;
		tp =0;

	};

	bool pointQuery(vector<int> low, vector<int> high);
	inline void perfect()
	{
		if(db==NULL)
		{
			cout<<"Warning: can't make perfect"<<endl;
		}
		else
		{

		Query::Query_t q;
		q.left = 0;
		q.right = syn->actual_size;
		vector<Query::Query_t> empty = db->determineEmptyRanges(q,syn->lowerb,syn->actual_size);
		for(int i = 0;i<empty.size();i++)
		{

			syn->learn_from_fp(empty[i]);
		}

		}
	}
};

#endif /* SYNOPSISMD_H_ */

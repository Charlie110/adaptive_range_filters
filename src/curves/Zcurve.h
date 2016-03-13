/*
 * Zcurve.h
 *
 *  Created on: Nov 21, 2012
 *      Author: carolinux
 */
//#include "../Synopsis.h"
//#include "../Util.h"
#include "Curve.h"
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
using namespace std;
#ifndef ZCURVE_H_
#define ZCURVE_H_


//point query(data) -> pointquery(z(data))
//range query(data1,data2) -> range queries in a number of ranges (this
//is gonna be super interesting !! )


class Zcurve : public Curve {



private:
	//int dim;
	//int domain;
	//Synopsis * syn;
	int * mask;
	//int bits;
	inline int maxpos(int i)
	{
		return bits - dim + i;
	}
public:

	Zcurve(vector<attribute> attr);
	inline int getDomain(){ return domain;};
	inline int getDimensions(){ return dim;};
	int Z(vector<int>);
	vector<int> Zinv(int x);
	virtual ~Zcurve();

	vector<Query::Query_t> linearizeRangeQuery(vector<int> low,vector<int> high);
	int linearizePointQuery(vector<int>);
	void validateNJI(int cur,int zmin,int zmax,int njo);
	void validateNJO(int cur,int zmin,int zmax,int njo);
	void validateRangeQuery(vector<int> low,vector<int> high,vector<Query::Query_t> qs);

	int nextJumpIn(int,int,int);
	int nextJumpOut(int,int,int); //Broken, unfortunately
	int nextJumpOutDumb(int,int,int);
	vector<int> getViolations(int cur,int zmin, int zmax);
	int getBit(int val,int pos);
	void printBinary(int);
	int getU(int,int,int);
	void printDims(vector<int>);
	int flipBit(int val,int pos);
	void setBit(int & val,int pos,int bit);
	vector<vector<int> > generateAllTheQueries(vector<int>);
	vector<int>delinearizeCurveValue(int Zval);
	void printCoordinates(int);
};

#endif /* ZCURVE_H_ */

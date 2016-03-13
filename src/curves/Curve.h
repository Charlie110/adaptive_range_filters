/*
 * Curve.h
 *
 *  Created on: Nov 22, 2012
 *      Author: carolinux
 */
#include "../Query.h"

#include <vector>

#ifndef CURVE_H_
#define CURVE_H_



class Curve {

protected:
	int bits;
	int domain;
	int dim;
	vector<int> domains;

public:

	bool sameDomain;
	char type;
	struct attribute
		{
			int lowerb;
			int higherb;

		};
	Curve();
	virtual vector<Query::Query_t> linearizeRangeQuery(vector<int>,vector<int>) = 0;
	virtual int linearizePointQuery(vector<int>) = 0;
	virtual ~Curve();
	virtual int getDomain() = 0;
	virtual vector<int> delinearizeCurveValue(int) = 0;
	virtual int getDimensions() = 0;
	virtual void printCoordinates(int) = 0;
	int closestLargerPower2(int);
	int getBits(int);
	bool isContained(vector<int> point,vector<int> low, vector<int> high);
	int getDomainOfDim();
	int getDomainOfDim(int d)
	{
		return domains[d];
	}
	vector<int> getDomains()
	{
		return domains;
	}
	void printp(vector<int> pt);

};

#endif /* CURVE_H_ */

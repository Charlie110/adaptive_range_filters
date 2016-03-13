/*
 * Hcurve.h
 *
 *  Created on: Nov 26, 2012
 *      Author: carolinux
 */
#include "Curve.h"
#ifndef HCURVE_H_
#define HCURVE_H_


//implementation requires things to have the same domain :(

class Hcurve :public Curve {
	//int dim;
	//int domain;
	int bitsperdim;
public:
	Hcurve(vector<Curve::attribute> attr);
		virtual ~Hcurve();
		 vector<Query::Query_t> linearizeRangeQuery(vector<int>,vector<int>);
		 vector<Query::Query_t> linearizeRangeQueryStupid(int lowb,int highb,
		 		vector<int> low,vector<int> high,int& hits);
		int linearizePointQuery(vector<int>);

		 int getDomain(){return domain;};
		 vector<int> delinearizeCurveValue(int);
		 int getDimensions(){return dim;};
		 void printCoordinates(int);
		 int getBoxCardinality(vector<int> low,vector<int> high);
		 bool continuous(Query::Query_t a, Query::Query_t b);
};

#endif /* HCURVE_H_ */

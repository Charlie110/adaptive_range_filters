/*
 * Ccurve.h
 *
 *  Created on: Nov 26, 2012
 *      Author: carolinux
 */

#include "Curve.h"
#include <vector>
#ifndef CCURVE_H_
#define CCURVE_H_

class Ccurve : public Curve{
	//int dim;
	//int domain;
	vector<int> widths;
public:
	Ccurve(vector<Curve::attribute> attr);
	virtual ~Ccurve();
	 vector<Query::Query_t> linearizeRangeQuery(vector<int>,vector<int>);
	int linearizePointQuery(vector<int>);

	 int getDomain(){return domain;};
	 vector<int> delinearizeCurveValue(int);
	 int getDimensions(){return dim;};
	 void printCoordinates(int);
};

#endif /* CCURVE_H_ */

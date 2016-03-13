/*
 * PointGen.h
 *
 *  Created on: Nov 28, 2012
 *      Author: carolinux
 *
 *      A point generator that returns tuples
 *      of dimension d ie [a1,a2,...,a(d-1),ad]
 *      of independent variables that follow a given
 *      distribution(s)
 */

#include "../Distribution.h"
#include "../curves/Curve.h"
#include <vector>

using namespace std;
#ifndef POINTGEN_H_
#define POINTGEN_H_

class PointGen {

	Curve * c;
	vector<Distribution *> distr;
	bool useSame;
public:
	PointGen(int,Distribution *);
	PointGen(int,vector<Distribution *>);
	virtual ~PointGen();
	int dim;
	vector<int> generate();
};

#endif /* POINTGEN_H_ */

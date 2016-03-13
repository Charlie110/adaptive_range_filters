/*
 * Statistics.h
 *
 *  Created on: Dec 10, 2012
 *      Author: carolinux
 */
#include <vector>
#include "assert.h"
using namespace std;

#ifndef STATISTICS_H_
#define STATISTICS_H_

class Statistics {


public:

	int tp ;
		int tn ;
		int fp;
		int total_ranges;
		int q;

	vector<int> fps;


	Statistics();
	Statistics(int domain);
	virtual ~Statistics();
	void update(bool,bool);
	void incrementFP(int low,int high);
	void reset();
	double getFpr();
	void printFp();
	void print();
	double getColdStore();

	void updateRange(int r);
};

#endif /* STATISTICS_H_ */

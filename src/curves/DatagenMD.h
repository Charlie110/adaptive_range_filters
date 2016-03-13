/*
 * DatagenMD.h
 *
 *  Created on: Nov 23, 2012
 *      Author: carolinux
 */


// ***************************************************************************
// ** Datengenerierung **
// ***************************************************************************

	#include <stdio.h>
	#include <stdlib.h>
	#include <limits.h>
	#include <math.h>
	#include <vector>
#include <iostream>
	// #ifdef CYGWIN
	#define MAXINT 2147483647
	// #else
	// #define MAXINT 32767
	// #endif

	#define sqr(a) ((a)*(a))
#ifndef DATAGENMD_H_
#define DATAGENMD_H_
using namespace std;
class DatagenMD {
	int Statistics_Count;
		double* Statistics_SumX;
		double* Statistics_SumXsquared;
		double* Statistics_SumProduct;
public:
	DatagenMD();
	virtual ~DatagenMD();
	void GenerateData(int Dimensions,char Distribution,int Count,char* FileName) ;
	vector< vector<double> > GenerateDataAnticorrelated(FILE* f,int Count,int Dimensions) ;
	vector< vector<double> > GenerateDataCorrelated(FILE* f,int Count,int Dimensions) ;
	vector< vector<double> > GenerateDataEqually(FILE* f,int Count,int Dimensions) ;
	double RandomNormal(double med,double var);
	double RandomPeak(double min,double max,int dim);
	double RandomEqual(double min,double max) ;
	void OutputStatistics(int Dimensions) ;
	void EnterStatistics(int Dimensions,double* x) ;
	void InitStatistics(int Dimensions) ;
};

#endif /* DATAGENMD_H_ */

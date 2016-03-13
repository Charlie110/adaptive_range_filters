/*
 * ctest.cpp
 *
 *  Created on: Jan 11, 2013
 *      Author: carolinux
 */

#include <stdio.h>
#include <stdlib.h>
#include "Curve.h"
#include "Ccurve.h"
#include "Zcurve.h"
#include "Hcurve.h"
#include <vector>

using namespace std;
int main(int argc,char * argv[])
{
	char type = argv[1][0];
	int dims = atoi(argv[2]);
	int domain = 1;
	int width =  atoi(argv[3]);
	vector<Curve::attribute> attr(dims);
    vector<int> q(dims);
	for(int i=0;i<dims;i++)
	{
		q[i] = atoi(argv[3+1+i]);
	}


	for(int i=0;i<dims;i++)
	{
		domain*=width;
		attr[i].lowerb = 0;
		attr[i].higherb = width -1;
	}

	cout<<"Range: [0 -"<<domain-1<<"]"<<endl;


	Curve * c = NULL;
	if(type=='c')
	{
	 c = new Ccurve(attr);
	}
	if(type=='h')
	{
	 c = new Hcurve(attr);
	}
	if(type=='z')
	{
	 c = new Zcurve(attr);
	}



	for(int i=0;i<domain;i++)
	{

		vector<int> a = c->delinearizeCurveValue(i);

		int idx = c->linearizePointQuery(a);
		assert(idx == i);


	}

	c->printp(q);
	int val = c->linearizePointQuery(q);


	vector<int> pt = c->delinearizeCurveValue(val);
	c->printp(pt);

	for(int i=0;i<dims;i++)
	{
		assert(pt[i] == q[i]);
	}


	return 0;
}


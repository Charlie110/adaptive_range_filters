/*
 * Curve.cpp
 *
 *  Created on: Nov 22, 2012
 *      Author: carolinux
 */

#include "Curve.h"

Curve::Curve() {
	// TODO Auto-generated constructor stub

}

Curve::~Curve() {
	// TODO Auto-generated destructor stub
}


int Curve::closestLargerPower2(int n)
{
	if ((n & (n-1)) == 0)
		return n;
	int i = 1;
	while(n>>=1)
		i++;
	return 1<<i;

}

 int Curve::getBits(int domain)
{
		bits = 0;

		while (domain > 0) {
		    bits++;
		    domain = domain >> 1;
		}
		return bits;
}
int Curve::getDomainOfDim()
{
	//requires: all dimensions use the same number of bits, ie
	//have the same domain //
	int d = domain + 1;
	for(int i=1;i<dim;i++)
	{
		d = sqrt(d);
	}
	return d-1;
}

void Curve::printp(vector<int> pt)
{
	for(int i=0;i<pt.size();i++)
	{
		cout<<pt[i]<<",";
	}
	cout<<endl;
}
 bool Curve::isContained(vector<int> point,vector<int> low, vector<int> high)
 {
 	for(int i=0;i<point.size();i++)
 	{
 		if(point[i]<low[i] || point[i]>high[i])
 		{
 			//cout<<"not contained!"<<endl;
 			//cout<<low[i]<<" "<<point[i]<<" "<<high[i]<<endl;
 			return false;
 		}
 	}

 	return true;
 }

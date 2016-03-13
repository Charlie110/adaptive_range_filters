/*
 * PointGen.cpp
 *
 *  Created on: Nov 28, 2012
 *      Author: carolinux
 */

#include "PointGen.h"

PointGen::PointGen(int dim,Distribution *d)
{


	this->distr = vector<Distribution*>();
	distr.push_back(d);
	this->dim = dim;
	this->useSame = true;



}


PointGen::PointGen(int dim,vector<Distribution *>d)
{


	this->distr = d;
	this->dim = dim;
	this->useSame = false;



}


vector<int> PointGen::generate()
{
	vector<int> res(dim);

	for(int i=0;i<dim;i++)
	{
		if(useSame)
			res[i] = distr[0]->probe();
		else
			res[i] = distr[i]->probe();
	}


	return res;

}

PointGen::~PointGen() {
	// TODO Auto-generated destructor stub
}

/*
 * DualSynopsis.cpp
 *
 *  Created on: Jan 22, 2013
 *      Author: carolinux
 */

#include "DualSynopsis.h"


void DualSynopsis::reset()
{
	stats.reset();
	for(int i=0;i<dim;i++)
		syn[i]->stats.reset();
}

void DualSynopsis::convert(int bits)
{
	for(int i=0;i<dim;i++)
		syn[i]->convert(bits);
}



DualSynopsis::DualSynopsis(int dim,Database * db,
		vector<int> highp,vector<int> lowp,int clock_bits)
{
	stats = Statistics();

	this->db = db;
	this->dim = dim;
	syn.resize(dim);
	for(int i=0;i<dim;i++)
	{
		vector<int> high(1),low(1);
		high[0] = highp[i];
		low[0] = lowp[i];
		syn[i] = new QuadSynopsis(1,db, high,low,32);
		syn[i]->cdim = i;
	}


}

void DualSynopsis::truncate(int size)
{
	assert(dim ==2);
	for(int i=0;i<dim;i++)
	{
		syn[i]->truncate(size/dim);
	}
}


DualSynopsis::~DualSynopsis() {
	// TODO Auto-generated destructor stub
	for(int i=0;i<dim;i++)
		free(syn[i]);
}

void DualSynopsis::perfect()
{
	for(int i=0;i<dim;i++)
	{
		Database d(db,i);
		syn[i]->perfect(&d);

	}
	return;
}



bool DualSynopsis::handleQuery(Query::QueryMD_t q, bool doAdapt)
{
	bool res = true;
	for(int i=0;i<dim;i++)
	{
		bool dbres = db->exists(q.low[i],q.high[i],i);
		vector<int> low(1),high(1);
		low[0] = q.low[i];
		high[0] = q.high[i];
		Query::QueryMD_t q2;
		q2.low = low;
		q2.high = high;
		res = res & syn[i]->handleQuery(q2,doAdapt,dbres);

	}

	bool dbr = db->rangeQuery(q.low,q.high);

	stats.update(res,dbr);

	return res;
}

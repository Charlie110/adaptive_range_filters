/*
 * SynopsisMD.cpp
 *
 *  Created on: Nov 23, 2012
 *      Author: carolinux
 */

#include "SynopsisMD.h"
#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>

void SynopsisMD::listValues()
{
	for(int i=0;i<=curve->getDomain();i++)
	{
		if(db->contains(i))
		{
			curve->printCoordinates(i);
		}
	}
}

SynopsisMD::SynopsisMD(vector<Curve::attribute> attr,char curveType,int filterSize,Database *indb) {

	this->db = indb;
	dim = attr.size();
	curve_t = curveType;
	if(curveType=='z')
	{
		curve = new Zcurve(attr);
	}
	if(curveType=='c')
	{
		curve = new Ccurve(attr);
	}
	if(curveType=='h')
	{
		curve = new Hcurve(attr);
	}


	cout<<"Domain of (flattened) multi dimensional trie is:"<<curve->getDomain()<<endl;


	syn = new SynopsisIntClock(curve->getDomain(),filterSize,true,db);
	syn->setDatabase(db);
	syn->init();
	tp = 0;
	fp = 0;
	tn = 0;

	stats = Statistics();


	//maybe make db nao?

}

bool SynopsisMD::isPointQuery(vector<int> low, vector<int> high)
{
	assert(low.size() == high.size());
	for(int i=0;i<low.size();i++)
	{
		if(low[i]!=high[i])
			return false;
	}

	return true;
}

bool SynopsisMD::handleQuery(Query::QueryMD_t q, bool doAdapt)
{
	return handleQuery(q.low,q.high,doAdapt);
}

bool SynopsisMD::pointQuery(vector<int> low, vector<int> high)
{
	for(int i=0;i<low.size();i++)
	{
		if(low[i]!=high[i])
			return false;
	}
	return true;
}

bool SynopsisMD::handleQuery(vector<int> low, vector<int> high,bool doAdapt)
{
	vector<Query::Query_t> queries = curve->linearizeRangeQuery(low,high);
	bool result = false;
	bool dbresult = false;
	assert(this->curve->type == db->curve->type);
	/* otherwise the mapping makes no sense */

	/*
	if(curve_t=='c')
	{
		cout<<"c-intervls:"<<queries.size()<<endl;
	}
	if(curve_t=='z')
	{
		cout<<"z-intervls:"<<queries.size()<<endl;
	}*/
	if(!pointQuery(low,high))
		stats.updateRange(queries.size());

	for(int i=0;i<queries.size();i++)
	{

		//cout<<"handling: "<<queries[i].left<<" "<<queries[i].right<<endl;
		bool dbRes = db->rangeQuery(queries[i]);
		bool synRes = syn->handleQuery(queries[i],db,doAdapt,dbRes);
		assert(!(!synRes && dbRes)); //very important

		/*if(curve_t=='c')
		{
			cout<<"db res:"<<dbRes<<endl;
			cout<<"my res:"<<synRes<<endl;
		}*/

		if(doAdapt && (!dbRes && synRes))
		{
			/*if(curve_t=='c')
			{
				cout<<"adapting c curve"<<endl;
			}*/
			/*if(curve_t =='c' || curve_t=='z')
				assert(!rangeQuery(queries[i]));*/

			assert(!syn->handleQuery(queries[i],db,false,dbRes));
		}

		result = result | synRes;
		dbresult = dbresult | dbRes;

		if(!doAdapt && synRes)
		{
			result = true;
			break;
		}



	}

	stats.update(result,dbresult);

	return result;

}


bool SynopsisMD::rangeQuery(Query::Query_t zq)
{
	//FIXME: This is invalid for the Hilbert curve
	return rangeQuery(curve->delinearizeCurveValue(zq.left),curve->delinearizeCurveValue(zq.right));
}

bool SynopsisMD::rangeQuery(vector<int> low, vector<int> high)
{
	vector<Query::Query_t> queries = curve->linearizeRangeQuery(low,high);
	bool res = false;

	for(int i=0;i<queries.size();i++)
	{
		//cout<<"querying: "<<queries[i].left<<" "<<queries[i].right<<endl;
		res = syn->rangeQueryOpt(queries[i]);
		if(res)
			return true;
	}
	assert(res == false); //if we've reached that point :)
	return res;
}


void SynopsisMD::exportCurveFile(string file, bool useDB)
{
	string curve = exportCurve(useDB);
	ofstream myfile;
	 myfile.open (file);
	 myfile << curve;
	 myfile.close();
}

void SynopsisMD::set(int clock_bits)
{
	syn->set(0);
}

void SynopsisMD::convert(int bits)
{
	syn->convertToCompact(bits);
}
void SynopsisMD::truncateClock(int s)
{
	cout<<curve_t<<"-curve Start size:"<<syn->size()<<endl;
	syn->truncateClock(s);
	cout<<"End size:"<<syn->size()<<endl;
}
string SynopsisMD::exportCurve(bool useDB)
{
	stringstream ss;
	if(curve->getDimensions()!=2)
	{
		ss<<curve->getDimensions()<<" "<<'\n';
	}
	else
	{
		ss<<curve->getDimensions()<<" "<<sqrt(curve->getDomain()+1)<<" "<<sqrt(curve->getDomain()+1)<<'\n';
	}



	for(int i=0;i<=curve->getDomain();i++)
	{
		int key = i;
		bool exists = syn->pointQuery(key);
		if(useDB)
			exists = db->contains(key);
		vector<int> data = curve->delinearizeCurveValue(i);
		for(int j=0;j<data.size();j++)
			ss<<data[j]<<" ";

		ss<<exists<<"\n";
	}
	return ss.str();
}

bool SynopsisMD::pointQuery(vector<int> data)
{

	int key = curve->linearizePointQuery(data);

	return syn->pointQuery(key);

}

SynopsisMD::~SynopsisMD() {
	// TODO Auto-generated destructor stub
}

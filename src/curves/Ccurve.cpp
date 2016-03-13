/*
 * Ccurve.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: carolinux
 */

#include "Ccurve.h"

Ccurve::Ccurve(vector<Curve::attribute> attr):Curve() {

	dim = attr.size();
	domain = 1;
	widths = vector<int>(dim);
	type='c';
	for(int i=0;i<dim;i++)
	{
		int width = attr[i].higherb - attr[i].lowerb +1 ;

		cout<<"width: "<<width<<endl;
		domain*= closestLargerPower2(width);
		widths[i] = getBits(closestLargerPower2(width)-1);
		domains.push_back(width-1);

		cout<<"bits of dim"<<i<<":"<<widths[i]<<endl;
	}
	domain--;



}

/* unoptimized */


vector<Query::Query_t> Ccurve::linearizeRangeQuery(vector<int> low,vector<int> high)
{


	vector<Query::Query_t> qs;
		int curr_idx =linearizePointQuery(low);
		int limit = linearizePointQuery(high);

		//cout<<curr_idx<<"-"<<limit<<endl;

		while(curr_idx<=limit)
		{

			//cout<<"curr idx:"<<curr_idx<<endl;
			//cout<<"limit: "<<limit<<endl;


				Query::Query_t q;
				while(!isContained(delinearizeCurveValue(curr_idx),low,high))
				{
					//printp(delinearizeCurveValue(curr_idx));
					//getchar();
					curr_idx++;
					if(curr_idx>limit)
					{
						return qs;
					}
				}
				q.left = curr_idx;

				while(isContained(delinearizeCurveValue(curr_idx),low,high))
				{
					curr_idx++;
					if(curr_idx>limit)
					{
						curr_idx = limit+1;
						break;
					}
				}

				q.right = curr_idx-1;

				qs.push_back(q);
				if(curr_idx == limit)
					break;

		}

/*
		cout<<"Ccurve interval count:"<<qs.size()<<endl;
		cout<<"range q:"<<endl;
			cout<<low[0]<<","<<low[1]<<","<<low[2]<<endl;
		      	cout<<high[0]<<","<<high[1]<<","<<high[2]<<endl;*/


		return qs;

}



int Ccurve::linearizePointQuery(vector<int> data)
{

	assert(data.size() == dim);
	int res = 0;
	int rshift = 0;
	for(int i =0;i<data.size();i++)
	{
		res = res | (data[i]<<rshift);
		rshift+=widths[i];
	}
	//cout<<"res: "<<res<<endl;
	return res;
}

//TODO: FIX THIS

 vector<int> Ccurve::delinearizeCurveValue(int cval)
 {
	 vector<int> res(dim);
	 int mask = 0;
	 int cbits = widths[0];
	 int lshift = 0;
	 for(int i=0;i<dim;i++)
	 {
		 mask = 0;
		 for(int j=0;j<widths[i];j++)
		 {
			 mask = (mask<<1) | 1;
		 }
		 mask = mask << lshift;
		 res[i] = (cval & mask)>>lshift;
		 //cout<<"dimm: "<<i<<":"<<res[i]<<endl;

		 lshift+=widths[i];
		 cbits+=widths[i];

	 }



	 return res;
 }

void Ccurve::printCoordinates(int cval)
{
	vector<int> coords = delinearizeCurveValue(cval);
	for(int i=0;i<dim;i++)
	{
		cout<<coords[i];
		if(i<dim-1)
			cout<<",";
	}

	cout<<endl;
}

Ccurve::~Ccurve() {
	// TODO Auto-generated destructor stub
}

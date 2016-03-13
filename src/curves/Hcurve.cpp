/*
 * Hcurve.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: carolinux
 */

#include "Hcurve.h"
#include "../hilbert/hilbert.h" /*  magical code from the 90s */



Hcurve::Hcurve(vector<Curve::attribute> attr):Curve() {

	dim = attr.size();
	int maxw = 0;
	bitsperdim = -1;
	type ='h';


	for(int i=0;i<dim;i++)
	{
		int width = attr[i].higherb - attr[i].lowerb +1 ;
		width = closestPower2(width-1);
		domains.push_back(width -1);
		int bpd = getBits(width);
		//assert(bitsperdim == -1 || bitsperdim == bpd);
		bitsperdim = std::max(bitsperdim,bpd);
		maxw= std::max(width,maxw);

	}
	domain = 1;
	for(int i=0;i<dim;i++)
		domain*=maxw;

	domain--;

}

int Hcurve::getBoxCardinality(vector<int> low,vector<int> high)
{


	int card = 1;
	bool equal = true;
	for(int i=0;i<low.size();i++)
	{
		if(low[i]!=high[i])
		{
			equal =false;
		}
	}
	if(equal)
		return 1;

	for(int i=0;i<low.size();i++)
	{

		card = card * (high[i]-low[i]+1);

	}

	return card;

}


vector<Query::Query_t> Hcurve::linearizeRangeQueryStupid(int lowb,int highb,
		vector<int> low,vector<int> high, int& points_found)
{
	vector<Query::Query_t> qs;
	int curr_idx = lowb;
	if(lowb<0 || highb<0 )
		return qs;
	while(curr_idx<=highb)
		{

				Query::Query_t q;
				while(!isContained(delinearizeCurveValue(curr_idx),low,high))
				{
					curr_idx++;
					if(curr_idx>domain)
					{
						return qs;
					}
				}
				q.left = curr_idx;

				while(isContained(delinearizeCurveValue(curr_idx),low,high))
				{

					points_found++;
					curr_idx++;
					if(curr_idx>domain)
					{
						curr_idx = domain;
						break;
					}
				}

				q.right = curr_idx;

				qs.push_back(q);
				if(curr_idx == domain)
					break;
		}
	return qs;

}


/* unoptimized */
/* the most stupid, cpu-wasting thing */

vector<Query::Query_t> Hcurve::linearizeRangeQuery(vector<int> low,vector<int> high)
{


	int curr_idx =0;
	int points_found = 0;
	int points_in_box = getBoxCardinality(low,high);



	int lowb = linearizePointQuery(low);
	int highb = linearizePointQuery(high);


	vector<Query::Query_t> qs = linearizeRangeQueryStupid(lowb,highb,
			low,high,points_found);

	//cout<<"Points found in [hl,hh]:"<<points_found<<endl;




	int offs =0;
	int least =-1;
	int maxim = -1;
	while(points_found <points_in_box)
	{
		//cout<<"found: "<<points_found<<endl;
		//cout<<"needed: "<<points_in_box<<endl;
		//expand downwards and upwards//
		vector<int> prev, next;
		int prev_idx = curr_idx-(offs+1);
		int next_idx = curr_idx+(offs+1);

		if(prev_idx>=0 && isContained(delinearizeCurveValue(prev_idx),low,high))
		{
			least = prev_idx;
			points_found++;
		}

		if(next_idx<=domain && isContained(delinearizeCurveValue(next_idx),low,high))
		{
			points_found++;
			maxim = next_idx;

		}

		offs++;


	}
	//cout<<"Max offset:"<<offs<<endl;
	//getchar();

	//ta vrikame :)
	//twra ftiahnoume ta rangess i guess... //



	assert(points_found == points_in_box);

	vector<Query::Query_t> lows = linearizeRangeQueryStupid(least,lowb-1,low,high,points_found);
	vector<Query::Query_t> highs = linearizeRangeQueryStupid(highb+1,maxim,low,high,points_found);

	if(qs.size()>0 && lows.size()>0)
	{
		if(continuous(lows[lows.size()-1],qs[0]))
		{
			//cout<<"merge low -main"<<endl;
			qs[0].left = lows.back().left;
			lows.pop_back();
		}
	}

	if(qs.size()>0 && highs.size()>0)
	{
		if(continuous(qs.back(),highs[0]))
		{
			//cout<<"merge main-high"<<endl;
			highs[0].left =qs.back().left;
			qs.pop_back();
		}
	}



	for(int i=0;i<qs.size();i++)
	{

		lows.push_back(qs[i]);
	}

	for(int i=0;i<highs.size();i++)
		lows.push_back(highs[i]);

  //TODO: some moar optimization here if some ranges are contiguous//


	/*cout<<"-------------------"<<endl;
	cout<<"Rnages: "<<lows.size()<<endl;
	for(int i=0;i<lows.size();i++)
	{
		cout<<lows[i].left<<"-"<<lows[i].right<<endl;
	}
	cout<<"-------------------"<<endl;


*/
	return lows;

}



bool Hcurve::continuous(Query::Query_t a, Query::Query_t b)
{
	return (a.right == (b.left -1));
}

int Hcurve::linearizePointQuery(vector<int> data)
{
	int pt[dim];
	for(int i=0;i<data.size();i++)
	{
		pt[i] = data[i];
	}


	return hilbert_c2i(dim,bitsperdim,pt);
}

 vector<int> Hcurve::delinearizeCurveValue(int hval)
 {
	 int pt[dim];
	 hilbert_i2c(dim,bitsperdim,hval,pt);
	 vector<int> res(dim);
	 for(int i=0;i<res.size();i++)
	{
		res[i] = pt[i];
	}
	 return res;
 }

void Hcurve::printCoordinates(int hval)
{
	vector<int> coords = delinearizeCurveValue(hval);
	for(int i=0;i<dim;i++)
	{
		cout<<coords[i];
		if(i<dim-1)
			cout<<",";
	}

	cout<<endl;
}

Hcurve::~Hcurve() {
	// TODO Auto-generated destructor stub
}

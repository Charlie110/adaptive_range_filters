/*
 * Zcurve.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: carolinux
 */

#include "Zcurve.h"

void Zcurve::validateRangeQuery(vector<int> low,vector<int> high,vector<Query::Query_t> qs)
{

  // step 1: assert that all the points in the returned intervals are indeed
	//within the original query box
	int zvals = 0;
	for(int i=0;i<qs.size();i++)
	{
		Query::Query_t q = qs[i];
		//cout<<"Interval "<<i<<endl;
		for(int j = q.left;j<=q.right;j++)
		{
			int cur  = j;
			vector<int> pt = Zinv(cur);
			//printDims(pt);
			zvals++;
			assert(isContained(pt,low,high));
		}
	}

	//cout<<"The query matches: "<<zvals<<" z values"<<endl;
	int rvals = 1;
	for(int i=0;i<low.size();i++)
	{
		rvals*=high[i]+1-low[i];
	}
	cout<<"it shoud actually match: "<<rvals<<" z values"<<endl;
	assert(rvals == zvals);
	//step 2: assert that there are no other points not in the query box (count them)

}
void Zcurve::printCoordinates(int zval)
{
	printDims(Zinv(zval));
}

void Zcurve::validateNJI(int cur,int zmin,int zmax,int nji)
{
	vector<int> low = Zinv(zmin);
	vector<int> high = Zinv(zmax);
	//cout<<"Current pos (out of query box)"<<endl;
	printBinary(cur);

	assert(!isContained(Zinv(cur),low,high));
	while(true)
	{
		if(isContained(Zinv(cur+1),low,high))
		{
			cout<<"is:"<<endl;
			printBinary(nji);
			cout<<"should be:"<<endl;
			printBinary(cur+1);
			assert(nji == cur+1);
			return;
		}
		cur++;
	}

}

void Zcurve::validateNJO(int cur,int zmin,int zmax,int njo)
{
	vector<int> low = Zinv(zmin);
	vector<int> high = Zinv(zmax);

	assert(isContained(Zinv(cur),low,high));
	cout<<"in pt:"<<endl;
	int startin = cur;
	printBinary(cur);
	cout<<"min:"<<endl;
	printBinary(zmin);
	cout<<"max:"<<endl;
	printBinary(zmax);
	while(true)
	{
		if(!isContained(Zinv(cur+1),low,high))
		{
			cout<<"is:"<<endl;
			printBinary(njo);
			cout<<"should be:"<<endl;
			printBinary(cur);
			assert(njo == cur);
			return;
		}
		cur++;
	}

}

vector<int> Zcurve::delinearizeCurveValue(int Zval)
{
	return Zinv(Zval);
}

vector<vector<int> > Zcurve::generateAllTheQueries(vector<int> lowerb)
{

	//domain
	int zmax = domain;
	vector<vector<int> > res;
	/*vector<int> curr(dim);
	for(int i=0;i<dim;i++)
		curr[i] = lowerb[i];
	bool  done = false;
	for(int i=0;i<dim;i++)
	{

		for(int j=lowerb;j<=domain;j++)
		{

		}

	}
*/
	return res;
}


vector<Query::Query_t> Zcurve::linearizeRangeQuery(vector<int> low,vector<int> high)
{
	//linearize the query box into an (unknown) number of intervals
	//on the z curve
	vector<Query::Query_t > res;

	bool done = false;
	int zmin = Z(low);
	int zmax = Z(high);
	int cur = zmin;
	//cout<<"zmin:"<<zmin<<endl;
	//cout<<"zmax: "<<zmax<<endl;

	while(!done)
	{
		Query::Query_t q;

		q.left = cur;
		//cout<<"Low end"<<endl;
		//printBinary(cur);
		if(isContained(Zinv(cur+1),low,high))
		{
			  //  cout<<cur+1<<"is in the qb"<<endl;
				q.right = nextJumpOutDumb(q.left,zmin,zmax); //FIXME
		}
		else q.right = q.left;
		//
		//cout<<"cur:"<<Zinv(cur)[0]<<","<<Zinv(cur)[1]<<endl;

		//cout<<"next jump out:"<<Zinv(q.right)[0]<<","<<Zinv(q.right)[1]<<endl;


		//cout<<"QRI:"<<q.right<<endl;
		//ffs

		//if(q.right<=zmax)
		/*{
			//validateNJO(cur,zmin,zmax,q.right);
			assert(isContained(Zinv(q.right),low,high));
		}*/
		if(q.right >= zmax)
		{
			q.right = zmax;
			done  = true;
		}
		else
		{
			//assert(isContained(Zinv(q.right),low,high));

		}
		if(!done)
		{
			cur = nextJumpIn(q.right+1,zmin,zmax);
			//validateNJI(q.right+1,zmin,zmax,cur);
			//cout<<"Next jump in:"<<Zinv(cur)[0]<<","<<Zinv(cur)[1]<<endl;
		}


		res.push_back(q);
	}

	//cout<<"Zcurve size:"<<res.size()<<endl;
	//cout<<"------------------------------"<<endl;

	//validateRangeQuery(low,high,res);
	//cout<<"Range query covers "<<res.size()<<" intervals on the Z curve"<<endl;
	return res;
}

int Zcurve::linearizePointQuery(vector<int> data)
{
	return Z(data);
}

Zcurve::Zcurve(vector<attribute> attr):Curve() {

	type='z';
	dim = attr.size();

	int max = 0;
	for(int i=0;i<dim;i++)
	{
		max = std::max(max, (1+ attr[i].higherb-attr[i].lowerb));

		domains.push_back(attr[i].higherb-attr[i].lowerb);

	}
	int universe = 1;

	for(int i=0;i<dim;i++)
		universe*=max;

	domain = closestLargerPower2(universe)-1;
	//assert(domain >= universe);
	cout<<"Zcurve Domain is [0,"<<domain<<"]"<<endl;

	bits = getBits(domain);

	assert(bits<=31); //otherwise the int won't work..
	assert(bits % dim ==0);
	cout<<"Bits per z value: "<<bits<<endl;

	mask = (int* ) malloc(sizeof(int)*bits);

	for(int i=0;i<bits;i++)
	{
		mask[i] = (0<<31) | (1<<i);
		cout<<"Mask "<<i<<" :"<<mask[i]<<endl;
	}

	// :)
	//syn = new Synopsis(domain); //maek the synopsis

}

void Zcurve::setBit(int & val,int pos,int bit)
{

	if(getBit(val,pos) == bit)
		return;
	else
		val = flipBit(val,pos);
}


int Zcurve::nextJumpOutDumb(int cur, int zmin,int zmax)
{
	while(true)
	{
		if(cur == zmax)
			return zmax;
		if(!isContained(Zinv(cur+1),Zinv(zmin),Zinv(zmax)))
			return cur;
		cur++;

	}
}
//STUPID ASS FUNCTION THAT DOESNT WORK :( //
int Zcurve::nextJumpOut(int cur,int zmin,int zmax)
{
	//precond: cur is inside the query box :)
	int MAXPOS = bits -1;

	//cout<<"start pt"<<endl;
	//printBinary(cur);
	vector<int> viols = getViolations(cur,zmin,zmax);
	vector<int> maxs = Zinv(zmax);
	vector<int> dims = Zinv(cur);
	int njo = 0;
	bool vbetween = false;
	if(viols.empty())
	{
		//cout<<"zmax"<<endl;
		return zmax;
	}

	while(!viols.empty())
	{
		cout<<"viol size:"<<viols.size()<<endl;
		int c;
		int v = viols.front();
		viols.erase(viols.begin());
		cout<<"viol size:"<<viols.size()<<endl;
		//cout<<"viol at position: "<<v<<endl;

		int d = v % dim;
		if(getBit(cur,v) == 0)
		{
			cout<<"MAX VIOL!!"<<endl;
			int temp = maxs[d] + 1;
			dims[d] = temp;
			temp = Z(dims);

			for(int i=MAXPOS;i>=0;i--) //all the bits
			{
			//	cout<<"bit: "<<i<<endl;
				////printBinary(temp);
				if(i>v && i%dim !=d )
					setBit(temp,i,getBit(cur,i));
				if(i<v && i % dim !=d)
					setBit(temp,i,0);
			}
			cout<<"max viol at pos "<<v<<endl;
			return temp-1;
		}
		else //min violation
		{
		    cout<<"Min viol at pos"<<v<<endl;
			if(!viols.empty())
			{
				//cout<<"get next"<<endl;
				c = viols.front();
				viols.erase(viols.begin());
				cout<<"viol size:"<<viols.size()<<endl;

			}
			else
			{
				//cout<<"emptyyy.."<<endl;
				c = v;
			}
			//cout<<"c = "<<c<<endl;
			int cp = v;
			njo = 0;
			int ival;
			for(int i=v+1;i<=MAXPOS;i++)
			{
				//cout<<"maxpos:"<<maxpos(d)<<endl;
				ival = i;
				//cout<<"c vs i?"<<c<<"=="<<i<<"?"<<endl;

				if(getBit(cur,i)==0)
				{
					cout<<"inbetweeb?"<<vbetween<<endl;
					cout<<"found zero bit at pos "<<i<<endl;
					if(i % dim !=d || vbetween)
					{
						int dcurr = i % dim;
						njo = cur;
						//cout<<"current viol:"<<v<<endl;
						cout<<"found next  zero bit of another dimension at pos: "<<i<<endl;
						for(int j= MAXPOS;j>=0;j--)
						{
							//cout<<"bit:"<<j<<endl;
							//printBinary(njo);
							cout<<"j>=i?"<<j<<">="<<i<<"?"<<endl;
							if(j>=i)
							{
								continue;
								setBit(njo,j,getBit(cur,j));
								cout<<"bit: "<<getBit(cur,j)<<endl;
							}
							else if(j == i)
								setBit(njo,j,1);
							else
							{
								cout<<"1??"<<endl;

								setBit(njo,j,0);
							}
						}
						njo--;
						cout<<"[min] result:"<<endl;
						//printBinary(njo);
						//cout<<"min viol"<<endl;
						return njo;
					}
					else if(c == i)
					{
						cout<<"break??"<<endl;
						viols.insert(viols.begin(),i);
						cout<<"viol size after break:"<<viols.size()<<endl;
						//ival = bits; //FIXME no idea
						break;
					}
					/*else //FIXME : Not sure if this should be here
					{
					cout<<"zero bit discarded.."<<endl;
					break; //??
					}*/
				}
				else if(i == c && i % dim !=d)
				{
					vbetween = true;
					if(!viols.empty())
					{
						c = viols.front();
						viols.erase(viols.begin());
						cout<<"viol size:"<<viols.size()<<endl;
						//i = v +1;
					}



				}
			}//end of for
				if(ival>MAXPOS)
				{
					cout<<"returning max"<<endl;
					return zmax;
				}
			} //end of min viol


		   cout<<"out of else..."<<endl;
		   cout<<"v size:"<<viols.size()<<endl;

			//min violation yadda yadda
		}

	//cout<<"error prone"<<endl;
	//return zmax;


}

void Zcurve::printDims(vector<int> d)
{
	for(int i=0;i<d.size();i++)
	{
		cout<<d[i];
		//cout<<"-";
		//printBinary(d[i]);
		if(i<d.size()-1)
			cout<<",";
		cout<<" ";
	}
	cout<<endl;
}

vector<int> Zcurve::getViolations(int cur,int zmin, int zmax)
{
	//precondition: cur must be IN the query box
	vector<int> viol;
	int min_cand,max_cand;
	int cur_bit,min_bit,max_bit;
	bool min_done;
	bool max_done;
	for(int i=0;i<dim;i++)
	{
		//cout<<"-------------"<<endl;
		//cout<<"Dimension "<<i+1<<endl;
		min_done = false;
		max_done = false;
		min_cand = -1;
		max_cand = -1;
		for(int j=maxpos(i);j>=0;j-=dim) //go through all the bits of the dimension
		{


			int pos = j;
			cur_bit = getBit(cur,pos);
			max_bit = getBit(zmax,pos);
			min_bit = getBit(zmin,pos);

			if(!min_done)
			{
				if(cur_bit ==min_bit && min_bit ==1 && min_cand ==-1)
				{
					//cout<<"min violation at pos:"<<pos<<endl;
					viol.push_back(pos);
				}
				else if(cur_bit ==1 && min_bit ==0 && min_cand ==-1)
				{
					min_cand = pos;
				}
				else if(min_bit ==1 && min_cand>=0)
				{
					viol.push_back(min_cand);
					//cout<<"min violation at pos:"<<min_cand<<endl;
					min_done = true;
				}

			}

			if(!max_done)
			{
				if(cur_bit ==max_bit && max_bit ==0 && max_cand ==-1)
				{
					//cout<<"max violation at pos:"<<pos<<endl;
					viol.push_back(pos);
				}
				else if(cur_bit ==0 && max_bit ==1 && max_cand ==-1)
				{
					max_cand = pos;
				}
				else if(max_bit ==0 && max_cand>=0)
				{
					//cout<<"max violation at pos:"<<max_cand<<endl;
					viol.push_back(max_cand);
					max_done = true;
				}

			}

		}
	}
	std::sort(viol.begin(),viol.end());
	for(int i=0;i<viol.size();i++)
	{
		//cout<<"viol:"<<viol[i];
		/*if(getBit(cur,viol[i]) ==0 )
			cout<<" (MAX)"<<endl;
		else
			cout<<" (MIN)"<<endl;*/
	}

	//cout<<"-------------"<<endl;

	return viol;
}


int Zcurve::flipBit(int val,int pos)
{

	int b = getBit(val,pos);
	//cout<<"bit to flip: "<<b<<endl;
	//cout<<"val:"<<val<<endl;

	if(b) //from 1 to 0
	{
		return val & (~(1<<pos));
	}
	else //from 0 to 1
	{

		return val | (1<<(pos));

	}
}
int Zcurve::getU(int cur,int zmin,int zmax)
{

	int maxV = 0;

	vector<int> mins = Zinv(zmin);
	vector<int> maxs = Zinv(zmax);
	vector<int> curs = Zinv(cur);


	for(int i=0;i<dim;i++)
	{
		if(curs[i]<=maxs[i] && curs[i]>=mins[i])
			continue; //no violation in this dimension

		//cout<<"DImension "<<i+1<<endl;
		/*printBinary(curs[i]);
		printBinary(mins[i]);
		printBinary(maxs[i]);*/
		//cout<<"Violation in dimension "<<i+1<<" !"<<endl;

		int cur1 = 0;
		int min1 = 0;
		int max1 = 0;
		for(int j=(bits/dim)-1;j>=0;j--)
		{
			int cur_bit = getBit(curs[i],j);
			int min_bit = getBit(mins[i],j);
			int max_bit = getBit(maxs[i],j);

			cur1 = (cur1<<1) | cur_bit;
			min1 = (min1<<1) | min_bit;
			max1 = (max1<<1) | max_bit;


			//cout<<"J is "<<j<<endl;

			if(cur1< min1  || cur1>max1 )
			{
				int pos = j* dim +i;
				if(pos>maxV)
					maxV = pos;
				break;
			}
		}

	}

	return maxV;

}


int Zcurve::nextJumpIn(int cur,int zmin,int zmax) // precond: cur is not in the query box
{

	vector<int> mins = Zinv(zmin);
	int u,z;
	z = -1;
	u = -1;
	vector<int> gtMin(dim);
	vector<int> ltMax(dim);


    //find u

	//u = getViolations(cur,zmin,zmax).back();
	u = getU(cur,zmin,zmax);

	//cout<<"u: "<<u<<endl;
	/*if(cur == 12)
		u= 1;
	else
		u= 0;*/

	// find min/max
	for(int i=0;i<dim;i++)
	{
		gtMin[i] = -1;
		ltMax[i] = -1;
		int mini = 0;
		int curi = 0;
		int maxi = 0;
		for(int j=0;j<bits/dim;j++)
		{
			int pos = j*dim + i;
			//cout<<"pos: "<<pos<<endl;
			//cout<<"min:"<<zmin<<endl;
			int b = cur & mask[pos];
			int bmin = zmin & mask[pos];
			int bmax = zmax & mask[pos];

			if(b!=0)
				b=1;
			if(bmin!=0)
				bmin = 1;
			if(bmax!=0)
				bmax = 1;

			//cout<<"b:" <<b<<endl;
			//cout<<"bmin:" <<bmin<<endl;
			curi = curi | b<<j;
			mini = mini | bmin<<j;//min & (1<<i);
			maxi = maxi | bmax<<j;
			//cout<<"curi vs mini: "<<curi<<" vs "<<mini<<endl;
			//cout<<"curi vs maxi: "<<curi<<" vs "<<maxi<<endl;

			if(curi>mini && gtMin[i]==-1)
			{
				gtMin[i] = pos;
				//break;
			}
			if(curi<maxi && ltMax[i] == -1)
			{
				ltMax[i] = pos;
			}
		}
		//cout<<"Min"<<i+1<<": "<<gtMin[i]<<endl;
		//cout<<"Max"<<i+1<<": "<<ltMax[i]<<endl;
		//printBinary(Zinv(zmin)[i]);
		//printBinary(Zinv(cur)[i]);

	}

	//cout<<"u = "<<u<<endl;
	//u=0;

	// find z //
	for(int i=u;i<bits;i++)
	{
		//the ith bt must be zero

		int b = cur & mask[i];
		if(b)
		{
			b=1;
		}

		//cout<<i<<" B: "<<b<<endl;

		if(b==1)
			continue;

		//cout<<"i:"<<i<<endl;

		int curr_dim = i % dim;
		//cout<<"curr dim: "<<curr_dim<<endl;
		//cout<<ltMax[curr_dim]<<" > "<<i<<"?"<<endl;
		//cout<<"Byte of zmax in same pos:"<<getBit(zmax,i)<<endl;
		//if(ltMax[curr_dim]<=i && ltMax[curr_dim]!=-1) //FIXME ineq?
		//if(ltMax[curr_dim]<=i && ltMax[curr_dim]!=-1) //FIXME ineq?
		//cout<<ltMax[curr_dim]<<">"<<i<<"?"<<endl;
		int maxd = Zinv(zmax)[curr_dim];
		int maxt = Zinv(cur)[curr_dim];
		setBit(maxt,z/dim,1);
		if(ltMax[curr_dim]>i || maxt<=maxd) //FIXME ineq?
		{
			z = i;
			break;
		}
	}
	//assert(z!=-1); //ffs

	assert(z!=-1 || cur>=zmax);
	//cout<<"z = "<<z<<endl;

	//ok, and now we get the thing!

	vector<int> dims(dim);
	std::fill(dims.begin(),dims.end(),0);

	int dimz = z % dim;
	int mint = Zinv(cur)[dimz];
	int mind =  Zinv(zmin)[dimz];
	setBit(mint,(z/dim),1);


	//manipulate the thing;

	int result = cur;
	setBit(result,z,1);
	//printDims(Zinv(result));

	for(int i=0;i<dim;i++)
	{

		//cout<<"dim "<<i+1<<endl;
		for(int j=(bits/dim)-1;j>=0;j--)
		{
			//cout<<j<<"iter"<<endl;
			int pos = j*dim + i;

			//edw, an mporw na to valw 0 kai na einai to dimProjection>=min, tote nai
			int copy = result;
			int proj = Zinv(copy)[i];
			//setBit(proj,pos,0);
			proj = proj >> (j);
			int minproj = mins[i] >> (j);

			//cout<<"prefix for cur:"<<endl;
			//printBinary(proj);
			//cout<<"prefix for min:"<<endl;
			//printBinary(minproj);

			if(pos>=z)
							continue;


			if(proj>minproj)
			{
				setBit(result,pos,0);
			}
			else
			{
				setBit(result,pos,getBit(zmin,pos));
			}

			//cout<<"after pos "<<pos<<endl;
			//printBinary(result);


		}

	}


  ////printBinary(dims[0]);
 ////printBinary(dims[1]);

	//int res = Z(dims);
	/*printBinary(res);
	printBinary(Zinv(res)[0]);
	printBinary(Zinv(res)[1]);

   cout<<"Next jump in: "<<res<<endl;
   printBinary(cur);
   printBinary(zmin);
   printBinary(zmax);*/
	//printBinary(result);
   return result;

}

int Zcurve::getBit(int val,int pos)
{

	val = val & mask[pos];
	return (val!=0);
}

int Zcurve::Z(vector<int> data)
{
	assert(data.size() == dim);
	int res = 0 ;

	//first attribute in data vector is least significant (interleaved first)
	int curr_bit =0;
	while(curr_bit <bits)
	{
		int curr = 0;
		for(int i=0;i<dim;i++)
		{
			int b = data[i] & mask[curr_bit/dim];
			if(b!=0)
				b = 1;


			curr = curr | (b<<i);
		}
		res = res | (curr<<curr_bit);

		curr_bit+=dim;
	}



	return res;
	//interleave the things

}
void Zcurve::printBinary(int x)
{

	for(int i=bits-1;i>=0;i--)
	{
		cout<<getBit(x,i);
	}
	cout<<endl;
}
vector<int> Zcurve::Zinv(int x)
{
	vector<int> res(dim);
	std::fill(res.begin(),res.end(),0);
	for(int i=0;i<bits/dim;i++)
	{
		for(int j=0;j<dim;j++)
		{
			int b = x & mask[i*dim +j];
			if(b!=0)
				b=1;
			res[j] = res[j] | (b<<i);
		}
	}

	return res;
}

Zcurve::~Zcurve() {
	free(mask);
}


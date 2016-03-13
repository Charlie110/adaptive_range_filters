/*
 * SynopsisMD2.cpp
 *
 *  Created on: Dec 10, 2012
 *      Author: carolinux
 */

#include "SynopsisMD2.h"
#include <limits.h>

#include "math.h"


bool SynopsisMD2::isEqual(vector<int> a,vector<int> b)
{
	assert(a.size()==b.size());
	for(int i=0;i<a.size();i++)
	{
		if(a[i]!=b[i])
			return false;
	}
	return true;
}

bool SynopsisMD2::handleQuery(Query::QueryMD_t q, bool doAdapt,bool qR)
{

	currentMark = false;
	cq = q;
	bool sR;
	bool pointq = isEqual(q.low,q.high);
	if(pointq)
	{
		sR = pointQuery(q.low);
	}
	else
	{
		ranges_seen = 0;
		sR = rangeQuery(q.low,q.high);
		stats.updateRange(ranges_seen);
		ranges_seen = 0;
	}
	assert(!(!sR && qR));
    stats.update(sR,qR);
    if(!doAdapt)
    	return sR;

    if(sR && !qR) //false positive
    {
    	/*cout<<"FP for query:"<<endl;
    	print_point(q.low);
    	print_point(q.high);*/


    	learn_from_fp(q.low,q.high);
    	//takeSnapshot("after learning from fp");

    	assert(!pointQuery(q.low));
    	assert(!pointQuery(q.high));
    	assert(!rangeQuery(q.low,q.high));



    }
    if(sR && qR) //true positive
    {

    	//we don't need to
    	//do something, after using the db for splitting, i think..

    }
    if(!sR && !qR)
    {
		learn_from_tn(q.low,q.high);
    }


    return sR;

}

int SynopsisMD2::qcompare(vector<int> a,vector<int> b)
{
	//QUICK compare (box required)
	for(int i=0;i<a.size();i++ )
	{
		if(a[i]<b[i])
			return LESS;
		if(a[i]== b[i])
			continue;
		if(a[i]>b[i])
			return GREATER;
	}

	return EQUAL;
}

void SynopsisMD2::getMidpoints(vector<int> low,vector<int> high,int middle,int d,
		vector<int> * midpoint,vector<int> * midpoint2)
{
	midpoint->resize(this->dim);
	midpoint2->resize(this->dim);

	for(int i=0;i<low.size();i++)
	{
		if(i!=d)
		{
			midpoint->at(i) = high[i];
			midpoint2->at(i) = low[i];

		}
		else
		{
			midpoint->at(i) = middle;
			midpoint2->at(i) = middle + 1;
		}
	}
}



/* obsolete */



void SynopsisMD2::learn_from_fp(vector<int> low, vector<int> high)
{

	cq.low = low;
	cq.high = high;
	//expand

	/*expand(low,true);
	takeSnapshot("after left expand");
	expand(high,false);
	takeSnapshot("after right expand");*/

	expandQueryBox(low,high);
	//takeSnapshot("after expand");

	mark_empty(low,high);
	//takeSnapshot("after mark empty");

	//mark empty


}

vector<int> SynopsisMD2::getNextLeaf(vector<history> hist, vector<int> last,
									 vector<int> high)
{
    /* requires : there _is_ a next leaf */

	for(int i=hist.size()-1;i>=0;i--)
	{
		struct history curr = hist[i];
		if(!curr.wentLeft)
			continue;
		else
		{
			int sd = hist[i].split_dim;
			hist[i].low[sd] = hist[i].middle + 1;
			if(qcompare(hist[i].low,high)!=GREATER)
				return hist[i].low;
			else
				continue;
		}

	}

	//cout<<"Warning:Get next leaf called illegally"<<endl;
	vector<int> bogus(dim);
	bogus[0] = REACHED_END;
	return bogus;
	//assert(1==0);
}


void  SynopsisMD2::printLeaf(vector<int> low,vector<int> high)
{
	cout<<"Leaf:"<<endl;
	for(int i=0;i<low.size();i++)
		cout<<low[i]<<",";

	cout<<endl;
	for(int i=0;i<low.size();i++)
			cout<<high[i]<<",";

		cout<<endl;
}

bool SynopsisMD2::lessin1dim(vector<int> pt,vector<int> high)
{
	//returns true if there is at least one dimension
	//where pt.dim < high.dim
	bool res = false;

	for(int i=0;i<pt.size();i++)
	{
		if(pt[i]<high[i])
			return true;
	}
	return false;
}

bool SynopsisMD2::expandQueryBox(vector<int> low,vector<int> high)
{



  vector<history> hist;
  //hist.reserve(getLevels());
  struct nodeMD res = navigate(low,hist);
  curr_leaf.idx =res.idx;
  curr_leaf.lvl = res.lvl;

 // cout<<"exists?"<<res.exists<<endl;
  //cout<<"curr high: "<<res.high[0]<<" "<<res.high[1]<<endl;
  //we want it to be bigger than low (guaranteed)
  //and to not be equal or bigger than the high bound
 // while(overlap(res.low,res.high,low,high) && !inBox(res.low,res.high,low,high))
	  //(!greaterorEqual(res.high,high) || res.low,low))
  while(lessin1dim(res.low,high) || qcompare(res.low,high) == EQUAL)
   {
	// cout<<"carve? -currently at:"<<endl;
	// printLeaf(res.low,res.high);
	 curr_leaf.idx = res.idx;
	 curr_leaf.lvl = res.lvl;

	// takeSnapshot("looking at leafie for expanshun");


	  if(res.exists == !currentMark && overlap(res.low,res.high,low,high) && !inBox(res.low,res.high,low,high)) //if we need to carve this
	  {
		  //i need to cut the query box out of this node

		  	  int d = 0; //FIXME: Not necessarily dearr
		  	  d = getDimension(res.lvl,res.idx);

			  carve(res,res.low,res.high,low,high,d);
	  }




	  vector<int> next = getNextLeaf(hist,res.high,high);
	  if(next[0] == REACHED_END)
		  break;
	  //if none//
	  hist.clear();
	  res = navigate(next,hist);
	  curr_leaf.idx =res.idx;
	   curr_leaf.lvl = res.lvl;

   }

 // takeSnapshot("looking at last leafie for expanshun");
  curr_leaf.idx = -1;
  curr_leaf.lvl = -1;
  return res.exists;

}

bool SynopsisMD2::inBox(vector<int> pt,vector<int> low,vector<int> high)
{
	for(int i=0;i<pt.size();i++)
	{
		if(pt[i]>high[i] || pt[i]<low[i])
			return false;
	}

	return true;

}

bool SynopsisMD2::overlap(vector<int> x,vector<int> y,vector<int> low,vector<int> high)
{
	for(int i=0;i<x.size();i++)
	{
		if(x[i]>high[i] || y[i]<low[i])
			return false;
	}

	return true;

}

void SynopsisMD2::carve_alt(struct nodeMD res,vector<int> low,vector<int> high)
{

}

bool SynopsisMD2::dimInBox(vector<int> x,vector<int> y,vector<int> low,vector<int> high,int cdim)
{

	return (x[cdim]>=low[cdim] && y[cdim]<=high[cdim]);
}

bool SynopsisMD2::inBox(vector<int> clow,vector<int> chigh,vector<int> low,vector<int> high)
{
	for(int i=0;i<clow.size();i++)
	{
		if(clow[i]>=low[i] && clow[i]<=high[i] && chigh[i]>=low[i] && chigh[i]<=high[i])
			continue;
		else
			return false;
	}

	return true;

}


int SynopsisMD2::getNextDimension(int curr_lvl,vector<int> x,vector<int> y,vector<int> low,vector<int> high,int d)
{
	if(dim ==0 )
		return 0;
	if(alternatingDims)
	{

		while(true)
		{
			d = (d+1)%dim; //vreksei xionisei

			if(getMaxDepth(d)<curr_lvl+1)
				continue;
			else
				return d;
		}
	}
	else
	{

		while(true)
		{
			d = (d+1)%dim;
			if(!dimInBox(x,y,low,high,d))
				return d;
		}
	}
}

void  SynopsisMD2::printDims()
{
	if(!alternatingDims)
		return;

	int maxd = 0;
	for(int i=0;i<dim;i++)
		maxd = std::max(maxd,maxdepths[i]);

	for(int i=0;i<=maxd;i++)
	{
		int d = getDimensionAlt(i);
		//cout<<"Level: "<<i<<": dimension: "<<d<<endl;
	}

}

int SynopsisMD2::getDimensionAlt(int lvl)
{

	int d = 0;
	for(int i=0;i<=lvl;i++)
	{
		while(i>maxdepths[d])
		{
			d = getNextDimension(d);
		}

		if(i==lvl)
			return d;

		d = getNextDimension(d);
	}
	/* so as to not generate warning */
	return -1; // unreachable
}


int  SynopsisMD2::getDimension(int lvl,int idx)
{
	if(dim==1)
		return 0;

	if(alternatingDims)
	{
		return getDimensionAlt(lvl);
	}

	/* case where we use a bit sequence for the dimension */


	int node_size = 2 + dim_bits;
	int next_dim_idx = (((idx/node_size)+1) * node_size ) - dim_bits;
	if(dim ==2)
	{
		return shape[lvl][next_dim_idx];
	}
	else
	{
		int d = 0;
		int shft = dim_bits - 1;
		for(int i=next_dim_idx;i<next_dim_idx+dim_bits;i++)
		{
			int bit = shape[lvl][i];
			//cout<<"bit: "<<bit<<endl;
			d= d | (shape[lvl][i]<<shft);

			shft--;
		}

		//cout<<"d?:"<<d<<endl;
		return d;
	}
}




void SynopsisMD2::carve(struct nodeMD res,vector<int> clow,vector<int> chigh,
		vector<int> low,vector<int> high,int d)
{  /* recursive */



	if(dimInBox(clow,chigh,low,high,d))
			d = getNextDimension(res.lvl,clow,chigh,low,high,d); //get next dimension to fix

	if(alternatingDims)
	{
		d = getDimension(res.lvl,res.idx);
		d = getNextDimension(res.lvl,clow,chigh,low,high,d);
	}

	int middle = getMiddle(clow,chigh,d);
	vector<int> midpt1,midpt2;
	getMidpoints(clow,chigh,middle,d,&midpt1,&midpt2);
	int left_idx,right_idx;
	bool left, right;
	left = !currentMark;
	right = !currentMark;
	if(db!=NULL && currentMark!=true)
	{
		//cout<<"left:"<<endl;
		left = db->rangeQuery(clow,midpt1);
		//cout<<"right:"<<endl;
		right =db->rangeQuery(midpt2,chigh);
	}


    //stringstream ss;
    //ss<<"before split in dim:"<<d<<",lvl: "<<res.lvl;
	//takeSnapshot(ss.str());
	split(res.lvl,res.idx,left,right,&left_idx,&right_idx,res.leaf_idx,d);

	/* visualization */
	curr_leaf.idx = res.idx;
	curr_leaf.lvl = res.lvl;
	//takeSnapshot("after split [whilst carving])");
	//curr_leaf.idx = -1;
	//curr_leaf.lvl = -1;

	/* if the new leaves of the split parent overlap with query box
	 *  but are not fully contained, we need to 'carve' them as well */

	if(left==!currentMark && overlap(clow,midpt1,low,high) && !inBox(clow,midpt1,low,high))
	{
		struct nodeMD res2;
		res2.lvl = res.lvl +1;
		res2.idx = left_idx;
		res2.leaf_idx = getLeafOffset(res2.lvl,res2.idx);

		carve(res2,clow,midpt1,low,high,
				getNextDimension(res2.lvl,clow,midpt1,low,high,d) );
	}
	else
	{
		//if(inBox(clow,midpt1,low,high))
						//mark_empty(res.lvl+1, left_idx);
	}
	if(right==!currentMark && overlap(midpt2,chigh,low,high) && !inBox(midpt2,chigh,low,high))
	{
		struct nodeMD res2;
		res2.lvl = res.lvl +1;
		res2.idx = right_idx;
		res2.leaf_idx = getLeafOffset(res2.lvl,res2.idx);
		carve(res2,midpt2,chigh,low,high,
				getNextDimension(res2.lvl,midpt2,chigh,low,high,d) );
	}
	else
	{
		//if(inBox(midpt2,chigh,low,high))
			//			mark_empty(res.lvl+1,right_idx);
	}


}

void SynopsisMD2::mark_empty(int lvl, int idx)
{

	if(isLeaf(lvl,idx))
	{
		setLeafValue(lvl,getLeafOffset(lvl,idx),currentMark);
	}
	else
	{
		int left_child = getLeftChild(lvl,idx);

			mark_empty(lvl+1,left_child);
			mark_empty(lvl+1,left_child+1);


	}


}


bool SynopsisMD2::rangeQuery(vector<int> low,vector<int> high)
{



  vector<history> hist;
  //hist.reserve(getLevels());
  struct nodeMD res = navigate(low,hist);
  curr_leaf.idx =res.idx;
  curr_leaf.lvl = res.lvl;

 // cout<<"exists?"<<res.exists<<endl;
  //cout<<"curr high: "<<res.high[0]<<" "<<res.high[1]<<endl;
  //we want it to be bigger than low (guaranteed)
  //and to not be equal or bigger than the high bound
  while((!overlap(res.low,res.high,low,high) || !res.exists) && !greaterorEqual(res.high,high))
   {
	  //ie, there is a next

	  /*if(overlap(res.low,res.high,low,high))
		  takeSnapshot("looking at leafie for rq");
	  else
		  takeSnapshot("skipping non overlapping leafie for rq");

	  */
	  if(overlap(res.low,res.high,low,high))
		  ranges_seen++;

	  vector<int> next = getNextLeaf(hist,res.high,high);
	  //if none//
	  hist.clear();
	  res = navigate(next,hist);
	  curr_leaf.idx =res.idx;
	   curr_leaf.lvl = res.lvl;

   }

 // takeSnapshot("looking at last leafie for rq");
  curr_leaf.idx = -1;
  curr_leaf.lvl = -1;
  if(overlap(res.low,res.high,low,high))
	  return res.exists;
  else
	  return false;

}

int SynopsisMD2::getLevels()
{
	return shape.size();
}

void SynopsisMD2::mark_empty(vector<int>& low,vector<int>& high)
{

		vector<history> hist;
		struct nodeMD res = navigate(low,hist);

		while( !greater(res.high,high))
		{

		 /*cout<<"leaf conatining lower bound:";
				   cout<<res.low[0]<<","<<res.low[1]<<endl;
				   cout<<res.high[0]<<","<<res.high[1]<<endl;*/
		   if(inBox(res.low,res.high,low,high))
		   {
			   //cout<<"in box"<<endl;
		   	setLeafValue(res.lvl,res.leaf_idx,currentMark);
		   }



		   if(qcompare(res.high,high) ==EQUAL) //we don't need to mark any more leaves, we did it :D
			return;

		   vector<int> next = getNextLeaf(hist,res.high,high);
		   if(next[0] == REACHED_END)
		   		  break;
		   hist.clear();
		   res = navigate(next,hist);
		   //blargh

		   //otherwise//
		   //go to all the dimensions//

		   //naive #2: approach go in all directions, keep a visited set, so as not to
		   // have exponential (?) complexity

		   /*vector<vector<int> > neighbors(dim);
		   int i,j;
		   for(i=0;i<dim;i++)
		   {
			   neighbors[i].push_back(vector<int>(dim));
			   for(j=0;j<dim;j++)
			   {
				   if(i!=j)
					   neighbors[i][j] = start[j];
				   else
					   neighbors[i][j] = res.high[j]+1;
			   }
		   }
		*/


		} //end of {while still have leaves in range to mark as empty}


}

int SynopsisMD2::getMaxDepth(int dimension)
// TODO: This can be precomputed off course
{
	vector<int> bitsleft(dim);

	for(int i=0;i<dim;i++)
		bitsleft[i] = bits[i];

	int d = 0;
	int lvl = 0;
	while(true)
	{
		while(bitsleft[d]<=0)
			d = getNextDimension(d);

		bitsleft[d]--;


		if(dimension == d && bitsleft[d] == 0)
		{
			return lvl;
		}
		else
		{
			lvl++;
			d = getNextDimension(d);
		}
	}



}
int SynopsisMD2::getNextDimension(int d)
{
	return (d+1)% dim;
}

bool SynopsisMD2::dcompare(struct depth a, struct depth b)
{
	assert(a.d!=b.d); //should not happen
	if(a.max!=b.max)
		return a.max<b.max;
	else
		return a.d<b.d;
}

SynopsisMD2::SynopsisMD2(int dim,Database * db,vector<int> highp,vector<int> lowp,int clock_bits)
{

	this->onlyEmptyUsed = false; //false-> for all the leaves...
	this->alternatingDims = false;
	this->db = db;
	this->currentMark = false;
	this->mergeEqual = false;

	this->lowp = lowp;
	this->highp = highp;
	this->dim = dim;
	this->ranges_seen = 0;

	domains = vector<int>(dim);
	bits = vector<int>(dim);
	maxdepths = vector<int>(dim);
	for(int i=0;i<dim;i++)
	{
		domains[i] = highp[i]-lowp[i];
		bits[i] = (log2(domains[i]+1));
		//cout<<"bits: "<<bits[i]<<endl;
	}
	for(int i=0;i<dim;i++)
	{

		maxdepths[i] = getMaxDepth(i);
	}

	this->clock_bits = clock_bits;
	this->stats = Statistics();
	if(clock_bits == 1 || clock_bits == 32)
		used_bit_size = 1; //even for 32-bit clock
	else
		used_bit_size = 0;
	id = 0;
	snapshots = 0;
	outfolder="walkthrough";



	dim_bits = ceil(log2(dim));
	if(alternatingDims)
		dim_bits = 0;
	if(dim == 2 && !alternatingDims)
		assert(dim_bits == 1);
	prev_victim.idx = 0;
	prev_victim.lvl = 0;

	grow();
	vector<bool> internal(2+dim_bits);
	std::fill(internal.begin(),internal.begin()+1,0);
	if(dim_bits>0)
		std::fill(internal.begin()+2,internal.begin()+2+dim_bits,0);

	vector<bool> leaf_values(2);
	std::fill(leaf_values.begin(),leaf_values.end(),1);
	insertNode(0,0,internal,leaf_values);
	if(clock_bits==1 || clock_bits == 32)
	num_bits = 4 + 2 +dim_bits;
	else
		num_bits = 4 + dim_bits;

}


bool SynopsisMD2::pointQuery(vector<int> point)
{
	assert(dim == point.size());

	bool res = navigate(point);
	stringstream ss;
	ss<<"pt nav "<<point[0]<<","<<point[1];

	//takeSnapshot(ss.str());
	return res;
}




int SynopsisMD2::getMiddle(vector<int> low,vector<int> high,int idx)
{
	//idx = split dimension //
	return low[idx] + ((high[idx]-low[idx])>>1);
}


bool SynopsisMD2::lessorEqual(vector<int> pt,int middle,int dim_idx)
{
	int idx = dim_idx;
	return (pt[idx]<=middle);
}

bool SynopsisMD2::lessorEqual(vector<int> pt, vector<int> thres)
{
	for(int i=0;i<pt.size();i++)
	{
		if(pt[i]>thres[i])
			return false;
	}
	return true;

}

bool SynopsisMD2::less(vector<int> pt, vector<int> thres)
{
	for(int i=0;i<pt.size();i++)
	{
		if(pt[i]>=thres[i])
			return false;
	}
	return true;

}

bool SynopsisMD2::greaterorEqual(vector<int> pt, vector<int> thres)
{
	for(int i=0;i<pt.size();i++)
	{
		if(pt[i]<thres[i])
			return false;
	}
	return true;

}

bool SynopsisMD2::greater(vector<int> pt, vector<int> thres)
{
	for(int i=0;i<pt.size();i++)
	{
		if(pt[i]<=thres[i])
			return false;
	}
	return true;

}


bool SynopsisMD2::getLeaf(int lvl, int node_idx)
{

	int leaf_offset = getLeafOffset(lvl,node_idx);
	return leaves[lvl][leaf_offset];
}
void SynopsisMD2::print_point(vector<int> pt)
{
	for(int i=0;i<pt.size();i++)
	{
		cout<<pt[i]<<", ";
	}
	cout<<endl;
}

SynopsisMD2::nodeMD SynopsisMD2::navigate(vector<int> point, vector<SynopsisMD2::history> & hist)
{

	vector<int> low = lowp; //ie [0,0]
	vector<int> high = highp; // ie [127,7] or something
	int lvl = 0;
	int left_idx = 0;
	struct nodeMD result;



	while(true)
	{
		int split_dim = getDimension(lvl,left_idx);
		int right_idx = left_idx + 1;
		int middle = getMiddle(low,high,split_dim);
		struct history h;
		h.low = low;
		h.high = high;
		h.middle = middle;
		h.split_dim = split_dim;

		if(lessorEqual(point,middle,split_dim))
		{
			h.wentLeft = true;
			hist.push_back(h);
			if(isLeaf(lvl,left_idx))
			{
				high[split_dim] = middle;
				result.low = low;
				result.high = high;
				result.lvl = lvl;
				result.idx = left_idx;
				result.leaf_idx = getLeafOffset(lvl,left_idx);
				result.exists = getLeaf(lvl,left_idx);
				return result;
			}
			else
			{
				left_idx = getLeftChild(lvl,left_idx);
				//assert(right == exists);
				high[split_dim] = middle;
				lvl++;

			}
		}
		else //go right
		{
			h.wentLeft = false;
			hist.push_back(h);
			if(isLeaf(lvl,right_idx))
			{
				low[split_dim] = middle+1;
				result.exists = getLeaf(lvl,right_idx);
				result.low = low;
				result.high = high;
				result.lvl = lvl;
				result.idx = right_idx;
				result.leaf_idx =  getLeafOffset(lvl,right_idx);
				return result;
			}
			else
			{

				left_idx = getLeftChild(lvl,right_idx);
				lvl++;

				low[split_dim] = middle+1;


			}


		}



	}
}


bool SynopsisMD2::getBit(int lvl, int idx)
{
	return shape[lvl][idx];
}
bool SynopsisMD2::leafBit(int lvl,int idx)
{
	return leaves[lvl][idx];
}

void SynopsisMD2::insertDimension(int lvl,int idx, int dimension)
{
	if(dim==1 || alternatingDims )
		return;
	if(dim==2)
	{
		shape[lvl].insert(shape[lvl].begin() + idx,dimension);
		return;
	}
	if(dim>2)
	{
		int dimension0 = dimension;
		//cout<<"split dim: "<<dimension<<endl;
		for(int i=0;i<dim_bits;i++)
		{
			int bit = dimension & 1; //get LSB
			//cout<<"bit:"<<bit<<endl;
			shape[lvl].insert(shape[lvl].begin() + idx,bit);
			dimension>>=1;
		}

		//cout<<"Dim: "<<getDimension(lvl,idx)<<endl;
		assert(dimension0 == getDimension(lvl,idx-2));
		assert(dimension0 == getDimension(lvl,idx-1));
		return;
	}
}

bool SynopsisMD2::getLeaf(int lvl, int node_idx,int & leaf_offset,bool uncomputed)
{
#ifdef SYNDEBUG
	assert(uncomputed);
	assert(isLeaf(lvl,node_idx));
#endif
	if(uncomputed)
		leaf_offset = getLeafOffset(lvl,node_idx);
	return leaves[lvl][leaf_offset];

}

void SynopsisMD2::split(int lvl, int idx,bool left,
		bool right,int *lidx,int *ridx,int existing_leaf_idx,int split_dim)

{
	bool updateClock = false;



	if(prev_victim.lvl == lvl && prev_victim.idx == idx)
		updateClock = true;

	int parent_val = getLeafValue(lvl,existing_leaf_idx);
	removeLeafValue(lvl,existing_leaf_idx); //we remove the leaf from current level
	setValue(lvl,idx,true); // now it's an internal node


	int left_idx = getLeftChild(lvl,idx);
	insertValue(lvl+1,left_idx,false);
	insertValue(lvl+1,left_idx+1,false);
	insertDimension(lvl+1,left_idx+2,split_dim);
	int leaf_idx = getLeafOffset(lvl+1,left_idx);

	insertLeafValue(lvl+1,leaf_idx,left);
	insertLeafValue(lvl+1,leaf_idx+1,right);
	*lidx = left_idx;
	*ridx = left_idx + 1;

	num_bits+=3 + dim_bits +used_bit_size;



	if(updateClock)
	{
		prev_victim.lvl = lvl + 1;
		prev_victim.idx = left_idx;

	}



}

void SynopsisMD2::setValue(int lvl,int idx,bool val)
{

  shape[lvl][idx]=val;
}


void SynopsisMD2::setLeafValue(int lvl,int leaf_idx,bool val)
{
  int idx = leaf_idx;
  //bool prev = getLeafValue(lvl,leaf_idx);

  leaves[lvl][idx]=val;

}

int SynopsisMD2::getLeafValue(int lvl,int leaf_idx)
{


  return leaves[lvl][leaf_idx];

}

 void SynopsisMD2::insertValue(int lvl,int idx,bool val)
{

  if(shape.size() -1 < lvl)
  {

	grow();
  }
  shape[lvl].insert(shape[lvl].begin() + idx,val);

}


 void SynopsisMD2::removeValue(int lvl,int idx)
{

  shape[lvl].erase(shape[lvl].begin() + idx);
}



 void SynopsisMD2::sanityCheck1() /*weights */
 {
	 if(clock_bits ==32)
	 {
		 assert(leaves.size()==weights.size());

	 for(int i=0;i<leaves.size();i++)
	 {
		 assert(leaves[i].size() == weights[i].size());
	 }


	 }
 }



 int SynopsisMD2::size()
 {
	 int n=0;
	 for(int i=0;i<shape.size();i++)
	 {
		 n+=shape[i].size();
	 }

	 for(int i=0;i<leaves.size();i++)
	 {
		 n+=leaves[i].size();
	 }

	 if(clock_bits==32)
	 {
		 for(int i=0;i<weights.size();i++)
		 {
			 n+=weights[i].size() * used_bit_size;
		 }
	 }

	 if(clock_bits==1)
		 {
			 for(int i=0;i<used.size();i++)
			 {
				 n+=used[i].size() * used_bit_size;
			 }
		 }


	 return n;
 }


void SynopsisMD2::removeLeafValue(int lvl,int idx)
{


  leaves[lvl].erase(leaves[lvl].begin() + idx);
  if(clock_bits == 1 && !this->onlyEmptyUsed)
  {
	  used[lvl].erase(used[lvl].begin() + idx);
  }
  if(clock_bits == 32)
  {
	  assert(leaves[lvl].size() + 1 == weights[lvl].size() );

	  weights[lvl].erase(weights[lvl].begin() + idx);

  }
}


bool SynopsisMD2::navigate(vector<int> point)
{

	vector<int> low = lowp; //ie [0,0]
	vector<int> high = highp; // ie [127,7] or something
	int lvl = 0;
	int left_idx = 0;



	while(true)
	{
		int split_dim = getDimension(lvl,left_idx);
		int right_idx = left_idx + 1;
		int middle = getMiddle(low,high,split_dim);


		if(lessorEqual(point,middle,split_dim))
		{
			if(isLeaf(lvl,left_idx))
			{

				//curr_leaf.idx = left_idx;
				//curr_leaf.lvl =lvl;
				return getLeaf(lvl,left_idx);
			}
			else
			{
				left_idx = getLeftChild(lvl,left_idx);
				//assert(right == exists);
				high[split_dim] = middle;
				lvl++;

			}
		}
		else //go right
		{
			if(isLeaf(lvl,right_idx))
			{

				//curr_leaf.idx = right_idx;
				//curr_leaf.lvl =lvl;
				return getLeaf(lvl,right_idx);
			}
			else
			{
				left_idx = getLeftChild(lvl,right_idx);
				lvl++;

				low[split_dim] = middle+1;


			}


		}



	}
}



void SynopsisMD2::insertNode(int lvl, int idx, vector<bool> internal, vector<bool> leafs)
{
	shape[lvl].insert(shape[lvl].begin()+idx,internal.begin(),internal.end());

	int offset = getLeafOffset(lvl,idx);

	for(int i=0;i<leafs.size();i++)
	{
		insertLeafValue(lvl, offset+i,leafs[i]);
	}

}

void SynopsisMD2::insertLeafValue(int lvl,int idx, bool val)
{

	if(leaves.size() -1 < lvl)
	{
		grow();
	}
	assert(leaves[lvl].size()>=idx);
	leaves[lvl].insert(leaves[lvl].begin() + idx,val);

	if(clock_bits ==1 && !onlyEmptyUsed)
	{
	  used[lvl].insert(used[lvl].begin() + idx,0);
	}
	if(clock_bits == 32)
	{

		 assert(leaves[lvl].size() -1 == weights[lvl].size());
	  weights[lvl].insert(weights[lvl].begin() + idx,0);
	  assert(leaves[lvl].size() == weights[lvl].size() );
	}
}


int SynopsisMD2::getLeftChild(int parent_lvl,int parent_idx)
{

	int idx = 0;
	for(int i=0;i<parent_idx;i++)
	{// a sort of prefix
		//if(!isLeaf(parent_lvl,i))
		if(!dimensionBit(i))
			idx+=shape[parent_lvl][i];
	}

	if(dim_bits>0 && !alternatingDims)
		return idx*(2+dim_bits);
	else
		return idx<<1;
}

bool SynopsisMD2::isLeaf(int lvl,int idx)
{
	return (!shape[lvl][idx]);
}

int SynopsisMD2::getLeafOffset(int lvl,int idx)
{
	int offset = 0;
	for(int i=0;i<idx;i++) // a sort of prefix
	{

			if(!dimensionBit(i))
				offset+=isLeaf(lvl,i);
	}

	return offset;
}

bool SynopsisMD2::dimensionBit(int idx)
{
	if(dim ==1 || alternatingDims)
		return false;
	if(dim == 2)
	{
		return (idx+1)%3 ==0;
	}
	else
	{
		int nsize = 2 + dim_bits;
		if(idx % nsize <= 1)
			return false;
		else
			return true;

	}
}

void SynopsisMD2::grow()
{
	shape.push_back(vector<bool> ());
	leaves.push_back(vector<bool> ());
    if(clock_bits == 1)
    {
	  used.push_back(vector<bool> ());
    }
    if(clock_bits == 32)
    {
    	weights.push_back(vector<int>());
    }
}


void SynopsisMD2::setClockBits(int c)
{
	clock_bits = c;
}

SynopsisMD2::~SynopsisMD2() {
	// TODO Auto-generated destructor stub
}

void SynopsisMD2::perfect(Database *database)
{
	if(database==NULL)
	{
		cout<<"Warning: can't make perfect."<<endl;
	}
	else
	{

		currentMark = false;
		learn_from_fp(lowp,highp);
		takeSnapshot("all emptiez");
		currentMark = true;

		vector< vector<int> > pts = database->getPoints();

		for(int i=0;i<pts.size();i++)
		{
			//mark as occupied
			//cout<<"Marking point as occupied"<<endl;
			//database->curve->printp(pts[i]);
			learn_from_fp(pts[i],pts[i]);
			//getchar();
		}


		//TODO: merge all equal thingeys for graet justice
		//takeSnapshot("befoar mertz");

		currentMark = false;
		for(int i=0;i<shape.size();i++)
			merge_equal();

	}
}

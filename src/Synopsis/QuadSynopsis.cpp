/*
 * QuadSynopsis.cpp
 *
 *  Created on: Dec 14, 2012
 *      Author: carolinux
 */

#include "QuadSynopsis.h"


int QuadSynopsis::getUnused()
{
	cout<<"Numc: "<<numc<<endl;
	if(!onedimspecial || numc==2)
		return 0;
	else
	{
		takeSnapshot("unused");
		return unused;
	}
}

bool QuadSynopsis::navigate(vector<int> key)
{
	int lvl = 0;
	int first_idx = 0;
	vector<int> low = lowp;
	vector<int> high = highp;
	bool childFound = false;

	while(true)
	{
		vector<vector<int> > child = getChildrenUpperBounds(low,high);
		vector<vector<int> > lows = getChildrenLowerBounds(low,high);
		childFound = false;


		for(int i=0;i<lows.size();i++)
		{
						//assert(comp_res !=QNO_COMP); // this could happen .. //
			if(lessorEqual(key,child[i]))
				/* if all dimensions are less or equal */
			{
				//cout<<"yes"<<endl;

				int idx = first_idx + i;
				if(isLeaf(lvl,idx))
				{
					int leaf_idx = getLeafOffset(lvl,idx);
					bool res = getLeafValue(lvl,leaf_idx);

					curr_leaf.idx = idx;
					curr_leaf.lvl = lvl;

					//takeSnapshot("after nav");

					return res;
				}
				else
				{
					childFound = true;


					first_idx = getFirstChild(lvl,idx);
					lvl++;
					high = child[i];
					low = lows[i];


				}
			}


			if(childFound)
				break;
		}


	}




}

bool QuadSynopsis::rangeQuery(vector<int> low,vector<int> high)
{
	cq.low = low;
	cq.high = high;
	return rangeQuery(0,0,low,high,lowp,highp);
}

bool QuadSynopsis::overlap(vector<int> lowpt,vector<int> highpt,
		vector<int> low,vector<int> high)
{
	for(int i=0;i<dim;i++)
	{

		if(lowpt[i]>high[i] || highpt[i]<low[i])
			return false;
	}
	return true;
}



bool QuadSynopsis::rangeQuery(int lvl,int first_idx,
		vector<int> lowpt, vector<int> highpt,
		vector<int> low,vector<int> high)
{
	//first_idx -> the first index of the node cluster in this level


	bool res =false;
	vector<vector<int> > highs = getChildrenUpperBounds(low,high);
	vector<vector<int> > lows = getChildrenLowerBounds(low,high);

	for(int i=0;i<lows.size();i++)
	{
		int idx = first_idx +i;



	//	if(compareAllDim(lows[i],highpt) == QGREATER)
		//	return res;


		if(!overlap(lowpt,highpt,lows[i],highs[i]))
		{

			continue;

		}

		ranges_seen++;

		//there is overlap

		if(isLeaf(lvl,idx))
		{


			/*curr_leaf.lvl = lvl;QuadSynopsis::
			curr_leaf.idx = idx;
			takeSnapshot("range querying...");*/
			curr_leaf.idx = idx;
			curr_leaf.lvl = lvl;

			res = res | getLeaf(lvl,idx);
		}
		else
		{


			res = res | rangeQuery(lvl+1,getFirstChild(lvl,idx)
					,lowpt,highpt,lows[i],highs[i]);
		}

		if(res == true)
			return true;

	}



	return res;

}
QuadSynopsis::QuadSynopsis(int dim,Database * db,
		vector<int> highp,vector<int> lowp,int clock_bits, int numc)
{
		this->db = db;
		this->cdim = -1;
		this->ranges_seen =0;
		this->currentMark = false;
		this->onlyEmptyUsed = false;
		this->mergeEqual = false;
		if(numc>0)
		{
			this->onedimspecial = true;
			this->numc = numc;
			cout<<"Numc : "<<numc<<endl;
		}
		else
		{
			this->onedimspecial = false;
		}

		this->lowp = lowp;
		this->highp = highp;
		this->dim = dim;
		this->clock_bits = clock_bits;
		this->stats = Statistics();
		if(clock_bits == 1 || clock_bits == 32)
			used_bit_size = 1; //even for 32-bit clock
		else
			used_bit_size = 0;
		id = 0;
		snapshots = 0;
		outfolder="walkthrough";
		domains = vector<int>(dim);
		for(int i=0;i<dim;i++)
		{
			domains[i] = highp[i]-lowp[i];
		}

		grow();
		int lvl =0;
		int nc = getChildrenLowerBounds(lowp,highp).size();
		for(int i=0;i<nc;i++)
		{
			insertValue(lvl,i,false);
			insertLeafValue(lvl,i,true);
		}

		curr_leaf.idx = -1;
		curr_leaf.lvl = -1;
		prev_victim.idx = 0;
		prev_victim.lvl = 0;
		num_bits = nc * (1 + 1 + used_bit_size);
		unused = 0;
}

QuadSynopsis::~QuadSynopsis() {

}

int QuadSynopsis::size()
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

int QuadSynopsis::compare1Dim(vector<int> pt, vector<int> threshold,int d)
{
	if(pt[d] == threshold[d])
		return QEQUAL;
	if(pt[d] < threshold[d])
			return QLESS;
	if(pt[d] > threshold[d])
			return QGREATER;
}

void QuadSynopsis::print_point(vector<int> pt)
{
	for(int i=0;i<pt.size();i++)
	{
		cout<<pt[i]<<" ,";
	}
	cout<<endl;
}

void QuadSynopsis::setLeafValue(int lvl,int leaf_idx,bool val)
{
  int idx = leaf_idx;
  //bool prev = getLeafValue(lvl,leaf_idx);

  leaves[lvl][idx]=val;

}

bool QuadSynopsis::isContained(vector<int> clow, vector<int> chigh,
		vector<int> low, vector<int> high)
{
	/* not sure if correct */
	/*int low_comp = compareAllDim(lowpt,lowbox);
	int high_comp = compareAllDim(highpt,highbox);

	return ((low_comp == QEQUAL || low_comp == QGREATER)
			&& (high_comp == QEQUAL || high_comp == QLESS));*/


	for(int i=0;i<clow.size();i++)
	{
		if(clow[i]>=low[i] && clow[i]<=high[i] && chigh[i]>=low[i] && chigh[i]<=high[i])
			continue;
		else
			return false;
	}

	return true;


}

void QuadSynopsis::insertValue(int lvl,int idx,bool val)
{

 if(shape.size() -1 < lvl)
 {

	grow();
 }
 shape[lvl].insert(shape[lvl].begin() + idx,val);

}


void QuadSynopsis::insertLeafValue(int lvl,int idx, bool val)
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
	  weights[lvl].insert(weights[lvl].begin() + idx,0);
	}
}

void QuadSynopsis::removeValue(int lvl,int idx)
{

 shape[lvl].erase(shape[lvl].begin() + idx);
}


int QuadSynopsis::getLeafOffset(int lvl,int idx)
{
	int offset = 0;
	for(int i=0;i<idx;i++) // a sort of prefix
	{
				offset+=isLeaf(lvl,i);
	}

	return offset;
}

void QuadSynopsis::grow()
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

bool QuadSynopsis::isLeaf(int lvl,int idx)
{
	assert(shape.size()>lvl && shape[lvl].size()>idx);
	return (!shape[lvl][idx]);
}

void QuadSynopsis::removeLeafValue(int lvl,int idx)
{

  leaves[lvl].erase(leaves[lvl].begin() + idx);
  if(clock_bits == 1 && !this->onlyEmptyUsed)
  {
	  used[lvl].erase(used[lvl].begin() + idx);
  }
  if(clock_bits == 32)
  {
	  weights[lvl].erase(weights[lvl].begin() + idx);
  }
}


bool QuadSynopsis::getLeafValue(int lvl,int idx)
{
	assert(leaves.size()>lvl && leaves[lvl].size()>idx);
	return (leaves[lvl][idx]);
}

int QuadSynopsis::getFirstChild(int parent_lvl,int parent_idx)
{

	int idx = 0;
	for(int i=0;i<parent_idx;i++)
	{// a sort of prefix

			idx+=shape[parent_lvl][i];
	}

	if(!onedimspecial)
		return idx<<(dim);
	else
		return idx * numc;
}

bool QuadSynopsis::lessorEqual(vector<int> key, vector<int> threshold)
{
	for(int i=0;i<key.size();i++)
	{
		if(key[i]>threshold[i])
			return false;
	}
	return true;
}

int QuadSynopsis::compareAllDim(vector<int> key, vector<int> threshold)
{
	assert(key.size() == threshold.size() && key.size()>0);
	int i=0;
	if(key[i]>threshold[i])
	{
		for( i=1;i<key.size();i++)
		{
			if(key[i]<=threshold[i])
				return QNO_COMP;
		}
		return QGREATER;

	}
	if(key[i]==threshold[i])
	{
		for( i=1;i<key.size();i++)
		{
			if(key[i]!=threshold[i])
				return QNO_COMP;
		}
		return QEQUAL;

	}

	if(key[i]<threshold[i])
		{
			for( i=1;i<key.size();i++)
			{
				if(key[i]>=threshold[i])
					return QNO_COMP;
			}
			return QLESS;

		}

}

vector<int> QuadSynopsis::getMiddles(vector<int> low, vector<int> high)
{
	vector<int> res(dim);
	for(int i=0;i<dim;i++)
	{
		res[i] = low[i]+ ((high[i]-low[i])>>1);
	}
	return res;
}

bool QuadSynopsis::getBit(int nr,int idx)
{
	return (nr>>idx) & 1;
}

vector<vector<int> > QuadSynopsis::getChildrenUpperBounds(vector<int> low,vector<int> high)
{

	vector<vector<int> > res = vector< vector<int> >(numChildren());

	if(!onedimspecial)
	{
		vector<int> middles = getMiddles(low,high);

		for(int i=0;i<numChildren();i++)
		{
			res[i] = vector<int>(dim);
			for(int j=0;j<dim;j++)
			{
				//an to jith bit tou noumerou einai 1
				if(low[j]==high[j])
				{
					res[i][j] = low[j];
					continue;
				}

				if(getBit(i,j))
					res[i][j] = high[j];
				else
					res[i][j] = middles[j];
			}
		}
	}
	else
	{
		assert(low.size() == 1 && high.size() == 1);
		assert(dim ==1);
		int step = (high[0]-low[0]+1)/numChildren();
		int rem = (high[0]-low[0]+1)%numChildren();
		//cout<<"REMAINDER:"<<rem<<endl;
		assert(rem<numChildren());

		int n = numChildren();
		for(int i=0;i<numChildren();i++)
		{
			res[i] = vector<int>(dim);
			res[i][0] = -1;
		}
		bool childrenOverflow = false;
		if(step == 0)
		{

			childrenOverflow = true;
			step = 0;
			n = high[0]-low[0]+1;
			//res.resize(n);
		}
		int curr=low[0]+step -1 + (0<rem);

		for(int i=0;i<n;i++)
		{
			res[i] = vector<int>(dim);
			if(i == n-1)
				res[i][0] = high[0];
			else
				if(!childrenOverflow)
				{
					/*if(rem!=0)
						res[i][0] = low[0]+ (i+1)*step -1 +(i%rem ==0) ;
					else
						res[i][0] = low[0]+ (i+1)*step -1 ;*/

					res[i][0] = curr;
					curr+=step;
					if(i+1<rem)
						curr++;
				}
				else
					res[i][0] = low[0]+i;
		}


	}

	return res;

}


bool QuadSynopsis::getLeaf(int lvl, int node_idx)
{

	int leaf_offset = getLeafOffset(lvl,node_idx);
	return leaves[lvl][leaf_offset];
}


vector<vector<int> > QuadSynopsis::getChildrenLowerBounds(vector<int> low,vector<int> high)
{
	vector<vector<int> > res = vector< vector<int> >(numChildren());


	if(!onedimspecial)
	{
		vector<int> middles = getMiddles(low,high);
		for(int i=0;i<numChildren();i++)
		{
			res[i] = vector<int>(dim);
			for(int j=0;j<dim;j++)
			{
				if(low[j]==high[j])
				{
					res[i][j] = low[j];
					continue;
				}
				//an to jith bit tou noumerou einai 1
				if(getBit(i,j))
					res[i][j] = middles[j]+1;
				else
					res[i][j] = low[j];
			}
		}
	}
	else
	{
		assert(low.size() == 1 && high.size() == 1);
		assert(dim ==1);
		int step = (high[0]-low[0]+1)/numChildren();
		int n = numChildren();
		bool childrenOverflow = false;
		int rem = (high[0]-low[0]+1)%numChildren();
		assert(rem<numChildren());
		for(int i=0;i<numChildren();i++)
		{
			res[i] = vector<int>(dim);
			res[i][0] = -1;
		}
		//cout<<"step:"<<step<<endl;
		if(step == 0)
		{
			childrenOverflow = true;
			n = high[0]-low[0]+1;
			//res.resize(n);
		}
		int curr = low[0];
		for(int i=0;i<n;i++)
		{
			res[i] = vector<int>(dim);
			/*if(i == 0)
				res[i][0] = low[0];
			else*/
				if(!childrenOverflow)
				{
					/*if(rem!=0)
						res[i][0] = low[0]+ (i*step) +((i-1)%rem ==0) ;
					else
						res[i][0] = low[0]+ (i*step);*/
					res[i][0] = curr;

					curr+=step;
					if(i<rem)
						curr++;

				}
				else
					res[i][0] = low[0]+i;
		}


	}

	return res;

}

void QuadSynopsis::setValue(int lvl,int idx,bool val)
{

  shape[lvl][idx]=val;
}


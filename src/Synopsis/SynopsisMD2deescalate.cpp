/*
 * SynopsisMD2deescalate.cpp
 *
 *  Created on: Dec 11, 2012
 *      Author: carolinux
 */

#include "SynopsisMD2.h"


bool SynopsisMD2::evict_victims(int lvl, int idx,int plvl, int pidx,bool * visited_prev,int target_size)
{


	bool done;

	if(isLeaf(lvl,idx))
	{

		if(lvl == plvl && idx == pidx)
			*visited_prev = true;
		return false;
	}

	//int left_child =  levels->at(lvl).prefix + getLeftChild(lvl,levels->at(lvl).idx,idx);

	int left_child = getLeftChild(lvl,idx);

	/*
	levels->at(lvl).prefix = left_child;
	levels->at(lvl).idx = idx;*/

	if(*visited_prev && isLogicalLeaf(lvl,idx,left_child,false)) /* if candidate for truncating */
	{
		bool left,right;
		int leaf_idx;
		left = getLeaf(lvl+1,left_child,leaf_idx,true);
		right = getLeafValue(lvl+1, leaf_idx + 1);
				//getLeaf(lvl+1, left_child + 1,leaf_idx+1);

		return evict_lleaf(lvl,idx,left_child,leaf_idx,left,right,target_size);
	}
	 /* else, truncate the descendants (bottom-up) and check again ^^ */
	{

		if(!(*visited_prev) && lvl == plvl && idx == pidx)
		{
			*visited_prev = true;

		}



	   /* look left */

		{
			//cout<<"looking for victims at left child "<<lvl+1<<","<<left_child<<endl;
			done = evict_victims(lvl+1,left_child,plvl,pidx,visited_prev,target_size);
			if(done)
				return done;
		}

       /* look right */

		int right_child = left_child + 1;

		{
			//cout<<"looking for victims at right child "<<lvl+1<<","<<right_child<<endl;
			done = evict_victims(lvl+1,right_child,plvl,pidx,visited_prev,target_size);
			if(done)
				return done;
		}

	}

	return false;
}


bool SynopsisMD2::evict_lleaf(int lvl,int idx,int left_child,
		int leaf_idx,bool left,bool right,int target_size)
{

	prev_victim.idx = idx;
	prev_victim.lvl = lvl;
	bool merged = false;
	string action;
	//print_clock();

	if(!mergeEqual && (getUsed(lvl+1,leaf_idx)>0 || getUsed(lvl+1,leaf_idx+1)>0) && (right != left))
  	{

		//cout<<"decrementing"<<endl;
		decrementUsed(lvl+1,leaf_idx);
		decrementUsed(lvl+1,leaf_idx+1);
		action="decrementing";
  	}

  	else /* they are equal, or mergeable [because none of them is used] */
  	{

  		if(!mergeEqual || (mergeEqual && left==right))
  		{
			merge(lvl,left_child,idx);
			action="merging";
			merged =true;

			//levels->at(lvl).prefix = 0;
			//levels->at(lvl).idx = 0;

			/*int offset = getLeafOffset(lvl,idx);
			if(!getLeaf(lvl,idx,offset)) // if it's a zero leaf
				incrementUsed(lvl,offset); //parents used thingey
			else
				setUsed(lvl,offset,0);*/
  		}
  	}

	stringstream ss;
	//print_clock();
	ss<<"after leaf "<<action<<" : size ="<<num_bits<<" clock:"<<prev_victim.lvl<<","<<prev_victim.idx;
	//takeSnapshot(ss.str());

	if(num_bits>target_size)
		return false;
	else
		return true;

}
void SynopsisMD2::print_clock()
{
	cout<<"prev victim: "<<prev_victim.lvl<<","<<prev_victim.idx<<endl;
}

void SynopsisMD2::merge(int parent_lvl, int left_idx, int parent_idx)
{
  int lvl = parent_lvl;

  bool updateClock = false;

  if(lvl+1 == prev_victim.lvl && (prev_victim.idx == left_idx || prev_victim.idx == left_idx+1))
	updateClock = true;
  int right_idx = left_idx + 1;

  assert(isLeaf(lvl+1,left_idx) && isLeaf(lvl+1,right_idx));

  int leaf_idx;
  bool left = getLeaf(lvl+1,left_idx,leaf_idx,true);
  int right_leaf = leaf_idx + 1;
  bool right = getLeaf(lvl+1,left_idx+1);
  bool merged_val = left | right;
  int used_val = getUsed(lvl+1,leaf_idx) + getUsed(lvl+1,right_leaf);

/*#if defined(DOTRACE) && defined(TRACEMERGE)
	vector<pair<int,int> > special_nodes;
				pair<int,int> p,l,r;
				p.first =parent_lvl;
				p.second = parent_idx;
				r.first = parent_lvl +1;
				r.second = right_idx;
				l.first = parent_lvl +1;
				l.second = left_idx;
				special_nodes.push_back(p);
				special_nodes.push_back(l);
				special_nodes.push_back(r);
				takeSnapshot(special_nodes,TRACEMERGE);

#endif*/


  removeValue(lvl+1,left_idx);
  removeValue(lvl+1,left_idx); //this is not a mistake
  for(int i=0;i<dim_bits;i++)
	  removeValue(lvl+1,left_idx); //removing dimension bits :)

  removeLeafValue(lvl+1,leaf_idx);
  removeLeafValue(lvl+1,leaf_idx);//this is not a mistake
  setValue(lvl,parent_idx,false); //parent is turned into a leaf node
#ifdef SYNDEBUG
  assert(isLeaf(lvl,parent_idx));
#endif
  int leaf_offset = getLeafOffset(lvl,parent_idx);
  insertLeafValue(lvl,leaf_offset ,merged_val); //used will initially be zero or N/A

 // if(!merged_val) //if it is an empty leaf, adjust its used counter
  {
	  setUsed(lvl,leaf_offset,used_val);
  }


	num_bits = num_bits - (3+dim_bits) - (used_bit_size*1);


  //if we merged a prev.victim -> the prev victim is the parent
  if(updateClock)
  {
	prev_victim.lvl = lvl;
	prev_victim.idx = parent_idx;

  }

}



void SynopsisMD2::merge_equal()
{
	mergeEqual =true;
	bool visited_prev;
	visited_prev = true;
	evict_top(0,0,&visited_prev,0);
	mergeEqual = false;
}

void SynopsisMD2::truncate(int n)
{
	// first merge empty - equal? //
	cout<<"kd start size:"<<size()<<endl;
	bool visited_prev = false;
	int loops = 0;
	 while(num_bits>n)
	 {

		  truncate(n,&visited_prev);
		  visited_prev = true;
		  loops++;

	 }
	 cout<<"kd end size:"<<size()<<endl;

}

void SynopsisMD2::truncate(int n,bool * visited_prev) //n = target_size
{
	//determine where to start from by looking at the info about the previous victim ?hrmf?


	if(!(*visited_prev)) //if we need to start from the previous victim
		 evict_top(prev_victim.lvl,prev_victim.idx,visited_prev,n);
	else
		evict_top(0,0,visited_prev,n);





	return;
}

bool SynopsisMD2::evict_top(int plvl,int pidx,bool *visited,int target_size)
{
	//cout<<"truncating.."<<endl;
	//vector<prefix> levels = createPrefixHelper();
	return(evict_victims(0,0,plvl,pidx,visited,target_size) || evict_victims(0,1,plvl,pidx,visited,target_size));
}

bool SynopsisMD2::isLogicalLeaf(int lvl,int idx,int & left_child,bool assign)
{
	if(isLeaf(lvl,idx) /*|| isLastLevel(lvl)*/)
		return false;

	if(assign)
		left_child = getLeftChild(lvl,idx);
	//otherwise use the already calculated value
	int right_child = left_child + 1;
#ifdef SYNDEBUG
	assert(shape.size() > lvl+1); // we are not at the last level
	assert(shape[lvl+1].size() > right_child); // the vector of the children is ok
#endif
	return (isLeaf(lvl+1,left_child) && isLeaf(lvl+1,right_child));

}





void SynopsisMD2::learn_from_tn(vector<int> low,vector<int> high)
{

	//increment the weights of involved leaves
	if(clock_bits==0)
		return;

	mark_used(low,high);



}

void SynopsisMD2::mark_used(vector<int> low,vector<int> high)
{

		vector<history> hist;
		struct nodeMD res = navigate(low,hist);

		while( !greater(res.low,high))
		{

			/* cout<<"marking used leafie:";
				   cout<<res.low[0]<<","<<res.low[1]<<endl;
				   cout<<res.high[0]<<","<<res.high[1]<<endl;
				   */


			if(overlap(res.low,res.high,low,high))
			{
				   /* every leaf involved in answering the query */
				incrementUsed(res.lvl,res.leaf_idx);
			}

		   if(qcompare(res.high,high) == EQUAL) //we don't need to mark any more leaves, we did it :D
			return;

		   vector<int> next = getNextLeaf(hist,res.high,high);
		   hist.clear();
		   res = navigate(next,hist);
		   if(next[0] == REACHED_END)
		   		  break;

		  /* cout<<"next leafie:";
		  				   cout<<res.low[0]<<","<<res.low[1]<<endl;
		  				   cout<<res.high[0]<<","<<res.high[1]<<endl;*/
		   //blargh

		} //end of {while still have leaves in range to mark as used}

}

void SynopsisMD2::incrementUsed(int lvl,int leaf_idx)
{
	if(clock_bits ==0)
		return;
	if(clock_bits == 1)
	{
		used[lvl][leaf_idx]=1;
	}

	if(clock_bits == 32)
	{
		weights[lvl][leaf_idx]++;
	}


}

void SynopsisMD2::setUsed(int lvl,int leaf_idx,int val)
{
	if(clock_bits ==0)
		return;
	if(clock_bits == 1)
	{
		if(val>1)
			val = 1;
		used[lvl][leaf_idx]=val;
	}

	if(clock_bits == 32)
	{
		weights[lvl][leaf_idx]=val;
	}


}


void SynopsisMD2::decrementUsed(int lvl,int leaf_idx)
{
	if(clock_bits ==0)
		return;
	if(clock_bits == 1)
	{
		used[lvl][leaf_idx]=0;
	}

	if(clock_bits == 32 && weights[lvl][leaf_idx]>0)
	{
		weights[lvl][leaf_idx]--;
	}


}

int SynopsisMD2::getUsed(int lvl,int leaf_idx)
{
	if(clock_bits ==0)
		return 0;
	if(clock_bits == 1)
	{
		return used[lvl][leaf_idx];
	}

	if(clock_bits == 32)
	{
		return weights[lvl][leaf_idx];
	}


}

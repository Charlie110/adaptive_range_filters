/*
 * QuadSynospisdeescalate.cpp
 *
 *  Created on: Dec 19, 2012
 *      Author: carolinux
 */



#include "QuadSynopsis.h"

void QuadSynopsis::learn_from_tn(vector<int> low,vector<int> high)
{
	if(clock_bits ==0)
		return;

	mark_used(0,0,low,high,lowp,highp);

}

void QuadSynopsis::mark_used(int lvl, int first_idx,
		vector<int> lowpt, vector<int> highpt,vector<int> clow, vector<int> chigh)
{

	vector<vector<int> > highs = getChildrenUpperBounds(clow,chigh);
	vector<vector<int> > lows = getChildrenLowerBounds(clow,chigh);

	for(int i=0;i<numChildren();i++)
	{
		int idx = first_idx + i;
		if(overlap(lows[i],highs[i],lowpt,highpt))
		{
			/*cout<<"fond overlap HERE!"<<endl;

			cout<<lows[i][0]<<" "<<lows[i][1]<<"-"<<highs[i][0]<<" "<<highs[i][1]<<endl;
			*/

			if(isLeaf(lvl,idx))
			{
				/*cout<<"incrementing used"<<endl;
				curr_leaf.lvl = lvl;
				curr_leaf.idx = idx;
				takeSnapshot("befoar mark used");*/
				incrementUsed(lvl,getLeafOffset(lvl,idx));
				//takeSnapshot("after mark used");
				//cout<<"mark used!"<<endl;
			}
			else
			{
				mark_used(lvl+1,getFirstChild(lvl,idx),
						lowpt,highpt,lows[i],highs[i]);
			}

		}
	}

}



void QuadSynopsis::incrementUsed(int lvl,int leaf_idx)
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

void QuadSynopsis::setUsed(int lvl,int leaf_idx,int val)
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


void QuadSynopsis::decrementUsed(int lvl,int leaf_idx)
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

int QuadSynopsis::getUsed(int lvl,int leaf_idx)
{
	//cout<<"clock bits?"<<clock_bits<<endl;
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


bool QuadSynopsis::evict_victims(int lvl, int idx,int plvl, int pidx,bool * visited_prev,int target_size)
{


	bool done;

	curr_leaf.lvl = lvl;
	curr_leaf.idx = idx;
	//takeSnapshot("evicting in progress");

	if(isLeaf(lvl,idx))
	{

		if(lvl == plvl && idx == pidx)
			*visited_prev = true;
		return false;
	}

	//int left_child =  levels->at(lvl).prefix + getLeftChild(lvl,levels->at(lvl).idx,idx);

	int first_child = getFirstChild(lvl,idx);

	/*
	levels->at(lvl).prefix = left_child;
	levels->at(lvl).idx = idx;*/

	if(*visited_prev && isLogicalLeaf(lvl,idx,first_child,false)) /* if candidate for truncating */
	{

		return evict_lleaf(lvl,idx,first_child,target_size);
	}
	 /* else, truncate the descendants (bottom-up) and check again ^^ */
	{

		if(!(*visited_prev) && lvl == plvl && idx == pidx)
		{
			*visited_prev = true;

		}



	   for(int i=0;i<numChildren();i++)
		{
			//cout<<"looking for victims at left child "<<lvl+1<<","<<left_child<<endl;
			done = evict_victims(lvl+1,first_child+i,plvl,pidx,visited_prev,target_size);
			if(done)
				return done;
		}



	}

	return false;
}

void QuadSynopsis::merge_equal()
{
	//takeSnapshot("befoar merge");
	bool visited_prev;
	visited_prev = true;
	mergeEqual = true;
	evict_top(0,0,&visited_prev,0);
	mergeEqual = false;
}

bool QuadSynopsis::all_equal(int lvl, int first_leaf_idx)
{
	bool allEqual = true;

	for(int i=0;i<numChildren();i++)
	{
		if(i>0 && getLeafValue(lvl,first_leaf_idx+i) != getLeafValue(lvl,first_leaf_idx+i-1))
		{
			allEqual = false;
		}

	}

	return allEqual;
}

bool QuadSynopsis::mergeable(int lvl, int first_leaf_idx)
{
	bool allEqual = true;
	bool allNotUsed = true;
	for(int i=0;i<numChildren();i++)
	{
		if(i>0 && getLeafValue(lvl,first_leaf_idx+i) != getLeafValue(lvl,first_leaf_idx+i-1))
		{
			allEqual = false;
		}
		if(getUsed(lvl,first_leaf_idx+i)>0)
		{
			allNotUsed = false;
		}
	}

	return allEqual || allNotUsed ;
}


bool QuadSynopsis::evict_lleaf(int lvl,int idx,int first_child,
		int target_size)
{

	prev_victim.idx = idx;
	prev_victim.lvl = lvl;
	int leaf_idx = getLeafOffset(lvl+1,first_child);
	bool merged = false;
	string action;
	//print_clock();

	if(!mergeEqual && !mergeable(lvl+1, leaf_idx))
  	{

		//cout<<"decrementing"<<endl;
		for(int i=0;i<numChildren();i++)
			decrementUsed(lvl+1,leaf_idx+i);

		action="decrementing";
  	}

  	else /* they are equal, or mergeable [because none of them is used] */
  	{

  		if(!mergeEqual || (mergeEqual && all_equal(lvl+1,leaf_idx)))
  		{

		merge(lvl,first_child,idx,leaf_idx);
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

void QuadSynopsis::merge(int parent_lvl, int first_idx, int parent_idx,int first_leaf_idx)
{
  int lvl = parent_lvl;

  bool updateClock = false;



  if(lvl+1 == prev_victim.lvl && (prev_victim.idx >= first_idx &&
		  prev_victim.idx < first_idx+numChildren()))
  {
	updateClock = true;
  }


  bool merged_val = true;

  for(int i=0;i<numChildren();i++)
  {
	merged_val = merged_val | getLeafValue(lvl+1,first_leaf_idx + i);
  }

  int used_val = 0;//getUsed(lvl+1,leaf_idx) + getUsed(lvl+1,right_leaf);


  for(int i=0;i<numChildren();i++)
	  removeValue(lvl+1,first_idx);


  for(int i=0;i<numChildren();i++)
	  removeLeafValue(lvl+1,first_leaf_idx);

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


	num_bits = num_bits - (-1 + numChildren()*2) - (used_bit_size*(-1 + numChildren()));


  //if we merged a prev.victim -> the prev victim is the parent
  if(updateClock)
  {
	prev_victim.lvl = lvl;
	prev_victim.idx = parent_idx;

  }

}


bool QuadSynopsis::isLogicalLeaf(int lvl,int idx,int & first_child,bool assign)
{
	if(isLeaf(lvl,idx))
		return false;

	if(assign)
		first_child = getFirstChild(lvl,idx);

	bool res = true;

	for(int i=0;i<numChildren();i++)
	{
		if(!isLeaf(lvl+1,first_child +i))
		return false;
	}

	return res;


}


/* UNCHANGED  high-level Functions */

bool QuadSynopsis::evict_top(int plvl,int pidx,bool *visited,int target_size)
{
	//cout<<"truncating.."<<endl;
	//vector<prefix> levels = createPrefixHelper();


	for(int i=0;i<numChildren();i++)
	{
		bool res = evict_victims(0,i,plvl,pidx,visited,target_size);
		if(res)
			return true;
	}

	return false;
}

void QuadSynopsis::truncate(int n)
{
	// first merge empty - equal? //
	cout<<"Quad start size:"<<size()<<endl;
	bool visited_prev = false;
	int loops = 0;
	 while(num_bits>n)
	 {

		  truncate(n,&visited_prev);
		  visited_prev = true;
		  loops++;

	 }
	 cout<<"Quad end size:"<<size()<<endl;

}

void QuadSynopsis::truncate(int n,bool * visited_prev) //n = target_size
{
	//determine where to start from by looking at the info about the previous victim ?hrmf?


	if(!(*visited_prev)) //if we need to start from the previous victim
		 evict_top(prev_victim.lvl,prev_victim.idx,visited_prev,n);
	else
		evict_top(0,0,visited_prev,n);





	return;
}

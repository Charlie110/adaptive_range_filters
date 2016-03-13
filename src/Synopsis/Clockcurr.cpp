/* ---------------------------------------------------------------------------
 *                               CLASS IMPLEMENTATION
 *
 *   CLASS NAME:        Synopsis
 *
 *   FUNCITONS:         Synopsis::*
 *
 * ---------------------------------------------------------------------------
 */

 /* Synopsis clock: the replacement policy functions */

#include "../Synopsis.h"
#include <stdlib.h> //for malloc
#include <sys/time.h>

//also: clock pointer :)


int Synopsis::getEmptyLeaves()//ugly debug function
{
	return 0;
	int sum=0;
	countEmpty(0,0,&sum);
	countEmpty(0,1,&sum);

	return sum;
}

void Synopsis::countEmpty(int lvl,int idx,int * count)
{
        //cout<<"looking at node: "<<lvl<<","<<idx<<"..."<<endl;
	if(shape[lvl][idx] == 0 || (lvl+1) == shape.size()) //is a leafy or we are at last lvl
		return;

	int left_child = getLeftChild(lvl,idx);
	int offs;
	if(shape[lvl+1][left_child] == 0 && shape[lvl+1][left_child+1] == 0
	&& !getLeaf(lvl+1,left_child,offs,true) && !getLeaf(lvl+1,left_child+1,offs,true))
	{
		/*cout<<"the chidren of "<<lvl<<","<<idx<<" are empty"<<endl;
		cout<<"left:"<<lvl+1<<","<<left_child<<endl;
		cout<<"right:"<<lvl+1<<","<<left_child+1<<endl;*/
		prettyPrint();
		(*count)++;
		return;
	}

	if(shape[lvl+1][left_child] == 1)
		countEmpty(lvl+1,left_child,count);
	if(shape[lvl+1][left_child+1] == 1)
		countEmpty(lvl+1,left_child+1,count);

}

int Synopsis::getReplPolicy()
{
	return repl_policy;
}

void Synopsis::learn_from_tn(Query::Query_t r)
 // precondition: the leafies that cover interval r are empty
 //some of the existence bits were used for this query, so they must be important :)
 // O(N)
{
  //cout<<"marking thingeys as used"<<endl;
  if(clock_bits==0)
	return;
  vector<Synopsis::state> hist;
  vector<Synopsis::node> marked;
  mark_leaf(r,0,0,0,&hist,false,marked);
}


void Synopsis::incrementUsed(int lvl, int leaf_idx)
{
	if(clock_bits ==0)
			return;
   //cout<<"setting the use dof leaf "<<lvl<<","<<leaf_idx<<endl;
   assert(!leaves[lvl][leaf_idx]);
   	used[lvl][leaf_idx] = true; //default is zero
   //when do we call that?

}

void Synopsis::setUsed(int lvl,int leaf_idx,int val)
{
	if(clock_bits ==0)
			return;
#ifdef SYNDEBUG
	assert(!val || !leaves[lvl][leaf_idx]);
#endif
		used[lvl][leaf_idx] = val;
}

void Synopsis::decrementUsed(int lvl, int leaf_idx)
{
	if(clock_bits ==0)
		return;
   used[lvl][leaf_idx] = false; //this may change slightly if we use more bits for the clock

}


int Synopsis::getUsed(int lvl,int leaf_idx)
{
	if(clock_bits ==0)
			return 0;

  return used[lvl][leaf_idx];
}
void Synopsis::print_previous_victim()
{

	//cout<<"Previous victim"<<endl;
	//cout<<"iNFO: "<<prev_victim.lvl<<","<<prev_victim.idx<<endl;


}




void Synopsis::evict_victims(int n,bool * visited_prev) //n = target_size
{
	//determine where to start from by looking at the info about the previous victim ?hrmf?


	if(!(*visited_prev)) //if we need to start from the previous victim
		 evict_top(prev_victim.lvl,prev_victim.idx,visited_prev,n);
	else
		evict_top(0,0,visited_prev,n);





	return;
}

bool Synopsis::evict_top(int plvl,int pidx,bool *visited,int target_size)
{
	//cout<<"truncating.."<<endl;
	vector<prefix> levels = createPrefixHelper();
	return(evict_victims(0,0,plvl,pidx,visited,target_size,&levels) || evict_victims(0,1,plvl,pidx,visited,target_size,&levels));
}

void Synopsis::resetClock()
{


}

void Synopsis::resetUsed()
{
	for(int i=0;i<used.size();i++)
	{
		fill(used[i].begin(),used[i].end(),0);
	}
}

bool Synopsis::evict(int lvl,int idx,int left_child,int leaf_idx,bool left,bool right,int target_size,vector<prefix> *levels)
{




	prev_victim.idx = idx;
	prev_victim.lvl = lvl;
/*#ifdef SYNDEBUG
	assert(!(!left && !right));
#endif*/


	if((getUsed(lvl+1,leaf_idx) || getUsed(lvl+1,leaf_idx+1)) && (right != left))
  	{

		decrementUsed(lvl+1,leaf_idx);
		decrementUsed(lvl+1,leaf_idx+1);
  	}

  	else /* they are equal, or mergeable */
  	{

		merge(lvl,left_child,idx);

		levels->at(lvl).prefix = 0;
		levels->at(lvl).idx = 0;

		int offset = getLeafOffset(lvl,idx);
		if(!getLeaf(lvl,idx,offset)) // if it's a zero leaf
			incrementUsed(lvl,offset); //parents used thingey
		else
			setUsed(lvl,offset,0);
  	}

	if(num_bits>target_size)
		return false;
	else
		return true;

}

//current policy: start removing when you find the first removable logical node
//apparently, if i haven't merged-empty then this removes some things even though it could have removed
//redundant info (found later) instead
//TODO optimize me
bool Synopsis::evict_victims(int lvl, int idx,int plvl, int pidx,bool * visited_prev,int target_size,vector<prefix> * levels )
{


	bool done;

	if(isLeaf(lvl,idx))
	{

		if(lvl == plvl && idx == pidx)
			*visited_prev = true;
		return false;
	}
	int left_child =  levels->at(lvl).prefix + getLeftChild(lvl,levels->at(lvl).idx,idx);

	#ifdef SYNDEBUG
		assert(left_child == getLeftChild(lvl,idx));
	#endif

	levels->at(lvl).prefix = left_child;
	levels->at(lvl).idx = idx;

	if(*visited_prev && isLogicalLeaf(lvl,idx,left_child,false)) /* if candidate for truncating */
	{
		bool left,right;
		int leaf_idx;
		left = getLeaf(lvl+1,left_child,leaf_idx,true);
		right = getLeaf(lvl+1, left_child + 1,leaf_idx+1);
		//cout<<"found!"<<endl;
		return evict(lvl,idx,left_child,leaf_idx,left,right,target_size,levels);
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
			done = evict_victims(lvl+1,left_child,plvl,pidx,visited_prev,target_size,levels);
			if(done)
				return done;
		}

       /* look right */

		int right_child = left_child + 1;

		{
			//cout<<"looking for victims at right child "<<lvl+1<<","<<right_child<<endl;
			done = evict_victims(lvl+1,right_child,plvl,pidx,visited_prev,target_size,levels);
			if(done)
				return done;
		}

	}

	return false;
}

//NOTE: it does not make sense to call this before I've merged empty leaves etc
//because this one deletes information - merging does not!

void Synopsis:: killRandom(int n) //broken -> endless loop
{


	//i must find a random logical leaf to kill
	// i suppose i should do that in the lower levels?
	while(num_bits>n)
	{
		int lvl = shape.size() -2; //the penultimate level
		//what if this level is full of leaves?

		int idx;
		do
		{

			idx = random() % shape[lvl].size();

		}while(isLeaf(lvl,idx));
		assert(!isLeaf(lvl,idx));
		int left_child = getLeftChild(lvl,idx);
		merge(lvl,left_child,idx);

	}


	//int idx

}


void Synopsis::truncateRandom(int n)
{
	 if(num_bits>n)
 		merge(); // get rid of redundant information
	while(num_bits>n)
	{
			killRandom(n); //PUSH THE BUTTON !
	}
}


void Synopsis::truncateClock(int n)
{



#ifdef MERGE
	#ifdef DOTRACE
		#ifdef ONLYEMPTY
			takeTextSnapshot("Merge empty started");
		#else
			takeTextSnapshot("Merge (empty & both 1) started");
		#endif
	#endif

	if(num_bits>n)
	{
		#ifdef TICKTOCK
			 const uint64_t merge0 = rdtscp();

		#endif

		merge();

		#ifdef TICKTOCK
			const uint64_t merge1 = rdtscp();
			 if(merge1>merge0)
			 {
				 merget.push_front(merge1-merge0);
			 }

		#endif
	}

#endif

#ifdef DOTRUNCATE

#ifdef DOTRACE
	takeTextSnapshot("TRUNCATE STARTED");
#endif


 if(num_bits<=n)
	 return;
 bool visited_prev = false;
#ifdef TICKTOCK
 	 const uint64_t counter0 = rdtscp();

#endif
 while(num_bits>n)
 {
	  evict_victims(n,&visited_prev);
	  visited_prev = true;
 }


#endif

#ifdef TICKTOCK
 const uint64_t counter1 = rdtscp();
 if(counter1>counter0)
 {
	 truncatet.push_front(counter1-counter0);
	 /*cout<<"Start: "<<counter0<<endl;
	 cout<<"End: "<<counter1<<endl;*/
	 //cout<<"Cycles?:"<<counter1-counter0<<endl;
 }
 else
 {
	 cout<<"weird..."<<endl;
 }
#endif

}

void Synopsis::sanityCheckUsed()
{ //checks if there are any leaves with value = 1 with used bit set to 1 for some weird reason

	for(int i=0;i<leaves.size();i++)
	{

		for(int j=0;j<leaves[i].size();j++)
		{
			if(leaves[i][j])
				assert(!getUsed(i,j));
			//if(getUsed(i,j)>1)
			//	cout<<"yaaay"<<endl;
		}
	}

}


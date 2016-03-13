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
#include <algorithm>

//also: clock pointer :)

//bool myfunction (int i,int j) { return (i<j); }


bool Synopsis::clockcomp(Synopsis::node i, Synopsis::node j)
{
	return i.clock_val < j.clock_val;
}

vector<Synopsis::node> Synopsis::rankNodes()
{
	vector<node> res = vector<node>();


	rankNodes(0,0,res);
	rankNodes(0,1,res);
	random_shuffle(res.begin(),res.end());
	sort(res.begin(),res.end(),clockcomp);
	return res;
}

void Synopsis::rankNodes(int lvl,int idx,vector<node>& res)
{
	int left_child;
	if(isLeaf(lvl,idx))
		return;
	left_child = getLeftChild(lvl,idx);
	if(isLogicalLeaf(lvl,idx,left_child,false))
	{
		node n;
		n.idx = idx;
		n.lvl = lvl;
		n.clock_val = -lvl;


		if(res.size()==-2)
		{
					cout<<"thing:"<<res.size()<<endl;;
					cout<<"Lvl:"<<n.lvl<<endl;;
					cout<<"idx:"<<n.idx<<endl;
		}

		res.push_back(n);
		return;

	}
	else
	{
		if(!isLeaf(lvl+1,left_child))
			rankNodes(lvl+1,left_child,res);
		if(!isLeaf(lvl+1,left_child+1))
			rankNodes(lvl+1,left_child+1,res);
	}
}


void Synopsis::truncateLoc(int target_size)
{
	while(num_bits>target_size)
	{
		//go through the data structure
		//decrement the used of thingeys
		// create an order of 'using' according to both used bit AND locality
		//merge in that order


		vector<node> nodes = rankNodes();
		for(int i=0;i<nodes.size();i++)
		{
			//vector<node> nodes = rankNodes();
			int lvl = nodes[i].lvl;
			int idx = nodes[i].idx ;
			int left_child = getLeftChild(lvl,idx);
			int leaf_idx = getLeafOffset(lvl+1,left_child);
			bool left = getLeafValue(lvl+1,leaf_idx);
			bool right = getLeafValue(lvl+1,leaf_idx+1);
			vector<prefix> prfx = createPrefixHelper();

			/*cout<<"evicting:"<<i<<" of "<<nodes.size()<<endl;
			cout<<"Lvl:"<<nodes[i].lvl<<endl;;
			cout<<"idx:"<<nodes[i].idx<<endl;*/
			evict(lvl,idx,left_child,leaf_idx,left,right,target_size,&prfx);
			if(num_bits<=target_size)
			{

				break;
			}

		}

		//if we still exceed budget, repeat

	}
}
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

  mark_leaf(r,0,0,0,false);
}

int Synopsis::getUsedOffset(int lvl,int idx)
{
	int s=0;
	for(int i=0;i<idx;i++)
		s+=!(leaves[lvl][i]);

	return s;
}

void Synopsis::incrementUsed(int lvl, int leaf_idx)
{
	if(clock_bits ==0 || getLeafValue(lvl,leaf_idx))
			return;
   //cout<<"setting the use dof leaf "<<lvl<<","<<leaf_idx<<endl;
   assert(!leaves[lvl][leaf_idx]);

   leaf_idx = getUsedOffset(lvl,leaf_idx);
   	used[lvl][leaf_idx] = true; //default is zero
   //when do we call that?

}

void Synopsis::setUsed(int lvl,int leaf_idx,int val)
{
	if(clock_bits ==0 || getLeafValue(lvl,leaf_idx))
			return;
#ifdef SYNDEBUG
	assert(!val || !leaves[lvl][leaf_idx]);
#endif
		leaf_idx = getUsedOffset(lvl,leaf_idx);
		if(val>1)
			val = 1;
		used[lvl][leaf_idx] = val;
}

void Synopsis::decrementUsed(int lvl, int leaf_idx)
{
	if(clock_bits ==0 || getLeafValue(lvl,leaf_idx))
		return;
	leaf_idx = getUsedOffset(lvl,leaf_idx);
   used[lvl][leaf_idx] = false; //this may change slightly if we use more bits for the clock

}

void Synopsis::setUnused()
{
	for(int i=0;i<used.size();i++)
	{
		for(int j=0;j<used[i].size();j++)
			used[i][j] =false;
	}
}

int Synopsis::getUsed(int lvl,int leaf_idx)
{
	if(clock_bits ==0 || getLeafValue(lvl,leaf_idx))
			return 0;

  leaf_idx = getUsedOffset(lvl,leaf_idx);
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
		/*cout<<"will decrement"<<endl;
		cout<<"left:" <<getUsed(lvl+1,leaf_idx)<<endl;
		cout<<"right:" <<getUsed(lvl+1,leaf_idx+1)<<endl;*/
		//cout<<"decrementing"<<endl;
		decrementUsed(lvl+1,leaf_idx);
		decrementUsed(lvl+1,leaf_idx+1);
  	}

  	else /* they are equal, or mergeable */
  	{


  		cout<<"prev:"<<this->num_bits<<endl;
		merge(lvl,left_child,idx);
		cout<<"next:"<<this->num_bits<<endl;

		levels->at(lvl).prefix = 0;
		levels->at(lvl).idx = 0;

		/*int offset = getLeafOffset(lvl,idx);
		if(!getLeaf(lvl,idx,offset)) // if it's a zero leaf
			incrementUsed(lvl,offset); //parents used thingey
		else
			setUsed(lvl,offset,0);*/
		merged++;
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

		//set unused
		decrementUsed(lvl,getLeafOffset(lvl,idx));
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

	//truncateLoc(n);
	//return;

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
	cout<<"---MERGING EMPTYY ----"<<endl;

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
 	 int loops=0;
 	 merged = 0;
 	 cout<<"Initial size:"<<num_bits<<endl;
 while(num_bits>n)
 {
	 //cout<<"TRUNCAETING!! "<<clock_bits<<endl;
	  evict_victims(n,&visited_prev);
	  visited_prev = true;
	  loops++;
	 /* cout<<"limit?:"<<limit<<endl;
	  if(limit==1900)
	  {

		  cout<<"liit!"<<endl;
		  takeSnapshot(vector<std::pair<int,int> >() );
	  }*/
 }

 cout<<"truncate loops for synopsis: "<<loops<<endl;
 printf("Merged for synopsis: %d \n", merged);

#endif

#ifdef TICKTOCK
 const uint64_t counter1 = rdtscp();
 //cout<<"truncate clock"<<endl;
 if(counter1>counter0)
 {
	//cout<<"truncate push"<<endl;
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

int Synopsis::getEmptyLeaves(int lvl)
{
	int s = 0;
	for(int i=0;i<leaves[lvl].size();i++)
	{
		s+=!(leaves[lvl][i]);
	}
	return s;
}
int Synopsis::sanityCheckUsed()
{ //checks if there are any leaves with value = 1 with used bit set to 1 for some weird reason

	if(clock_bits==0)
		return 0;
	int sum=0;
	int usedd=0;
	//cout<<"SANITY CHECK!!!"<<endl;
	for(int i=0;i<leaves.size();i++)
	{
		int empty_leaves = 0;
		for(int j=0;j<leaves[i].size();j++)
		{
			if(leaves[i][j])
				assert(!getUsed(i,j));
			else
				usedd+=getUsed(i,j);

			empty_leaves+=!(leaves[i][j]);
			sum+=!(leaves[i][j]);

		}

		//cout<<"lvl "<<i<<": used size: "<<used[i].size()<<" epty: "<<empty_leaves<<endl;
		//cout<<"lvl "<<i<<": empty leaves: "<<used[i].size()<<" leafs: "<<leaves[i].size()<<endl;
		assert(empty_leaves == used[i].size());


	}
	cout<<"Emptu leaves(with used bit present): "<<sum<<endl;
	cout<<"used empty leaves: "<<usedd<<endl;
	return sum;
}


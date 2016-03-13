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
  if(repl_policy == RANDOM)
	return;
  vector<Synopsis::state> hist;
  vector<Synopsis::node> marked;
  mark_leaf(r,0,0,0,&hist,false,marked);
}


void Synopsis::incrementUsed(int lvl, int leaf_idx)
{
   //cout<<"setting the use dof leaf "<<lvl<<","<<leaf_idx<<endl;
   assert(!leaves[lvl][leaf_idx]);
   	used[lvl][leaf_idx] = true; //default is zero
   //when do we call that?

}

void Synopsis::setUsed(int lvl,int leaf_idx,int val)
{
	assert(!val || !leaves[lvl][leaf_idx]);
		used[lvl][leaf_idx] = val;
}

void Synopsis::decrementUsed(int lvl, int leaf_idx)
{

   used[lvl][leaf_idx] = false; //this may change slightly if we use more bits for the clock

}


int Synopsis::getUsed(int lvl,int leaf_idx)
{

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

	bool res = false;
	if(!(*visited_prev)) //if we need to start from the previous victim
		res = evict_victims(-1,0,prev_victim.lvl,prev_victim.idx,visited_prev,n);

	if(!res) // _cycle_ through leaves
	{
		//cout<<"starting from the beginning again!"<<endl;
		res = evict_victims(-1,0,0,0,visited_prev,n); //look at the leaves before the previous victime
	}

	return;
}




bool Synopsis::evict(int lvl,int idx,int left_child,int right_child,bool left,bool right,int target_size)
{

	struct node res;
	res.parent_idx = idx;
	res.lvl = lvl + 1;
	res.idx = left_child;
	res.leaf_idx = getLeafOffset(lvl+1,res.idx);
	if(left)
		res.leaf_idx+=1;

	if(getUsed(res.lvl,res.leaf_idx) && (left !=right))
  	{
		//assert(left != right);
		decrementUsed(res.lvl,res.leaf_idx);
  	}

  	else
  	{

		merge(res.lvl-1,res.idx,res.parent_idx);
		int offset = getLeafOffset(res.lvl-1,res.parent_idx);
		if(!getLeaf(res.lvl-1,res.parent_idx,offset)) // if it's a zero leaf
			incrementUsed(res.lvl-1,offset); //parents used thingey
		else
			setUsed(res.lvl-1,offset,0);
  	}

  //the leaf that was merged and now exists in the trie
  	prev_victim.idx = res.parent_idx;
  	prev_victim.lvl = res.lvl - 1;


	if(num_bits>target_size)
		return false;
	else
		return true;

}

//current policy: start removing when you find the first removable logical node
//apparently, if i haven't merged-empty then this removes some things even though it could have removed
//redundant info (found later) instead
//TODO optimize me
bool Synopsis::evict_victims(int lvl, int idx,int plvl, int pidx,bool * visited_prev,int target_size)
{


	bool done;

	if(lvl>=0 && isLeaf(lvl,idx))
	{

		if(lvl == plvl && idx == pidx)
			*visited_prev = true;
		return false;
	}
	int left_child = 0;
	if(*visited_prev && lvl>=0 && isLogicalLeaf(lvl,idx,left_child,true)) /* if candidate for truncating */
	{
		bool left,right;
		int leaf_idx;
		left = getLeaf(lvl+1,left_child,leaf_idx,true);
		right = getLeaf(lvl+1, left_child + 1,leaf_idx+1);
		//cout<<"found!"<<endl;
		return evict(lvl,idx,left_child,left_child+1,left,right,target_size);
	}
	 /* else, truncate the descendants (bottom-up) and check again ^^ */
	{

		if(!(*visited_prev) && lvl == plvl && idx == pidx)
		{
			*visited_prev = true;

		}



		if(lvl>=0)
			left_child = getLeftChild(lvl,idx);

		{
			//cout<<"looking for victims at left child "<<lvl+1<<","<<left_child<<endl;
			done = evict_victims(lvl+1,left_child,plvl,pidx,visited_prev,target_size);
			if(done)
				return done;
		}



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

//NOTE: it does not make sense to call this before I've merged empty leaves etc
//because this one deletes information - merging does not!

void Synopsis:: killRandom()
{


	//i must find a random logical leaf to kill
	// i suppose i should do that in the lower levels?
	int lvl = shape.size() -1;

	//int idx

}


void Synopsis::truncateRandom(int n)
{
	 if(num_bits>n)
 		merge(); // get rid of redundant information
	while(num_bits>n)
	{

		while(num_bits>n)
		{


			killRandom(); //PUSH THE BUTTON !
		}
	}
}


void Synopsis::truncateClock(int n)
{
 int loops = 0;
 double time;
 struct timeval start,end;

 //cout<<"--------------------TRUNCATING-------"<<endl;
 truncateCalled++;
 tick(start);
 //sanityCheckUsed();
// print_previous_victim();
 if(num_bits>n)
 	merge();
 bool visited_prev = false;
 while(num_bits>n)
 {
	  loops++; //how many times does it loop here? make a test config to see :)
	  //cout<<"Loop "<<loops<<endl;
	  //cout<<"Target size:"<<n<<" vs actual: "<<size()<<endl;
	  int prev_bits = num_bits;
	  evict_victims(n,&visited_prev); //this can become smartr
         // prettyPrint();
	 /* if(loops == 1000)
	  {
		exportGraphviz("foo.txt");
		assert(1==0);
	  }*/

 }

 time = tock(start,end,false);
if(time>truncateMax)
	truncateMax = time;
if(loops>maxLoops)
	maxLoops = loops;
truncateSum+=time;
//sanityCheckUsed();
 //cout<<"-----------------END OF TRUNCATION------------"<<endl;

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


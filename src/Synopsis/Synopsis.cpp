/* ---------------------------------------------------------------------------
 *                               CLASS IMPLEMENTATION
 *
 *   CLASS NAME:        Synopsis
 *
 *   FUNCITONS:         Synopsis::*
 *
 * ---------------------------------------------------------------------------
 */


#include "../Synopsis.h"
#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>




Synopsis::Synopsis(int domain,int limit,bool tp,Database * db,int repl_policy,int low)
{

	this->domain = domain;
	this->actual_size = domain;
	this->learn_tn = true;
	this->limit = limit;
	this->use_tp = tp;
	this->db = db;
	this->lowerb = low;
	this->repl_policy = repl_policy;
	this->clock_bits = 1;
	this->snapshots =0;
	this->outfolder ="walkthrough";
	fps = vector<vector<int> >();



	//for clock
 	prev_victim.idx = 0;
	//prev_victim.actual_idx = 0;
	prev_victim.lvl = 0;
    	//prev_victim.leaf_idx = 0; //left child of root
	num_bits = 6;
	id = 0;

}  // Constructor

Synopsis:: ~Synopsis() {

}  // Destructor

int Synopsis::max_height()
{
	int i =0;
	int d = domain+1;
	while((d>>1)>0)
	{
		i++;
		d>>=1;
	}

	return i;
}
void Synopsis::init()
{

	resetTime();
	int maxh = max_height();
	leaves.reserve(maxh);
	shape.reserve(maxh);
	lvls = vector<prefix>(maxh);
	resetPrefixHelper();
	if(clock_bits>0)
		used.reserve(maxh);
	grow();
	insertValue(0,0,false);
	insertValue(0,1,false);
	insertLeafValue(0,0,true);
	insertLeafValue(0,1,true);
//        sanityCheckUsed();

}

void Synopsis::perfect(Database * database)
{
	Query::Query_t q;
	q.left = 0;
	q.right = actual_size;
	vector<Query::Query_t> empty = database->determineEmptyRanges(q,lowerb,actual_size);
	for(int i = 0;i<empty.size();i++)
	{
		if(i%100 == 0)
			cout<<"learning from "<<i<<" of "<<empty.size()<<endl;
		learn_from_fp(empty[i]);
	}

}

void Synopsis::setUpperBound(int b)
{
	actual_size = b;
}

int Synopsis::getMaxVal()
{
	return lowerb+domain;
}
void Synopsis::resetTime()
{



	falsepos.clear();
	lookupt.clear();
	truncatet.clear();
	merget.clear();
	tp = 0;
	fp = 0;
	tn = 0;

}

bool Synopsis::isLeaf(int lvl,int idx)
{
	//return (shape[lvl][idx] == 0);
	return !shape[lvl][idx];
}

bool Synopsis::isRedundant(int lvl,int idx, int &left_child,bool assign)
{

	if(isLeaf(lvl,idx) /* || isLastLevel(lvl)*/)
		return false;

	if(assign)
		left_child = getLeftChild(lvl,idx);
	//otherwise use the already calculated value
	int right_child = left_child + 1;
#ifdef SYNDEBUG
	assert(shape.size() > lvl+1); // we are not at the last level
	assert(shape[lvl+1].size() > right_child); // the vector of the children is ok
#endif

	if(!isLeaf(lvl+1,left_child) || !isLeaf(lvl+1,right_child))
		return false;
	bool left,right;
	int leaf_offset;
	left = getLeaf(lvl+1,left_child,leaf_offset,true);
	right = getLeaf(lvl+1,right_child,leaf_offset+1);

	return (left == right);

}

bool Synopsis::isLogicalLeaf(int lvl,int idx,int & left_child,bool assign)
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

bool Synopsis::isLastLevel(int lvl)
{
	++lvl;
	return (shape.size() == lvl);
}

int Synopsis::nodes()
{

	int sum = 0;
	for(int i = 0;i<shape.size();i++)
		sum+= shape[i].size();
	return sum;

}

int Synopsis::Numleaves()
{
	int sum = 0;	
	for(int i = 0;i<leaves.size();i++)
		sum+= leaves[i].size();

	return sum;
}


int Synopsis::size() //kala tha doume edw //
{
	/*cout<<"Num bits: "<<num_bits<<endl;
	cout<<"Size of shape: "<<sizeof(shape)<<endl;
	cout<<"Size of leaves: "<<sizeof(leaves[0])<<endl;
	vector<bool> temp = vector<bool>();
	bool f = leaves[0][0];
	vector<bool>::iterator it;
	//leaves[0].erase(leaves[0].begin()+1);
	//leaves[0].insert(leaves[0].begin()+1);
	cout<<"length of leaves: "<<leaves[0].size()<<endl;
	//assert(leaves.size() == 1);
	cout<<"Size of 1 bit in sequence: "<<sizeof(f)<<endl;
	cout<<"Size of empty vector bool: "<<sizeof(temp)<<endl;
	return this->num_bits;*/
	int sum = 0;
	for(int i = 0;i<shape.size();i++)
		sum+= shape[i].size();

	for(int i = 0;i<leaves.size();i++)
		sum+= leaves[i].size();

	for(int i = 0;i<used.size();i++)
		sum+= used[i].size();
       // cout<<"Sum:"<<sum<<"vs: "<<num_bits<<endl;


	return sum;
}


bool Synopsis::getLeaf(int lvl, int node_idx,int & leaf_offset,bool uncomputed)
{
#ifdef SYNDEBUG
	assert(uncomputed);
	assert(isLeaf(lvl,node_idx));
#endif
	if(uncomputed)
		leaf_offset = getLeafOffset(lvl,node_idx);
	return leaves[lvl][leaf_offset];

}

bool Synopsis::getLeaf(int lvl, int node_idx,int leaf_offset)
{
#ifdef SYNDEBUG
	assert(isLeaf(lvl,node_idx));
#endif
	return leaves[lvl][leaf_offset];

}

int Synopsis::getParentIdx(int child_lvl,int left_idx)
{
	int lvl = child_lvl;
#ifdef SYNDEBUG
	assert(lvl>0);
	assert(left_idx % 2 == 0);
#endif
	int pos = left_idx /2 ;
	int curr = 0;
	int i=0;
	for(i=0;i<shape[lvl-1].size();i++)
	{

		if(!isLeaf(lvl-1,i))
		{
			if(curr == pos)
				return i;
			curr++;

		}

	}

#ifdef SYNDEBUG
	assert(1==0); //should be unreachable :)
#endif

	//this gives a warning
	return -1; // this should never happen
}

bool Synopsis::getLeafValue(int lvl, int leaf_idx)
{

	return leaves[lvl][leaf_idx];
}

bool Synopsis::rangeQuery(Query::Query_t q)
{
 // struct timeval start,end;
  //double time;
 // tick(start);
  uint low = q.left;
  uint high = q.right;
  vector<Synopsis::state> hist;
  struct node res= navigate(low,0,0,0,&hist);
  bool exists = res.exists;

  while(!exists && res.high<high)
  {
	struct state state = getRestartPoint(res.high+1,hist);
	res = navigate(res.high+1,state.lvl,state.idx,state.actual_idx,&hist);
	//res = navigate(res.high+1,0,0,0,&hist);
        exists = res.exists;
  }


  return exists;

}

void Synopsis::resetPrefixHelper()
{
	for(int i=0;i<lvls.size();i++)
	  	{
			lvls[i].idx = 0;
			lvls[i].prefix = 0;

	  	}
}

void Synopsis::printPrefixHelper()
{
	for(int i=0;i<lvls.size();i++)
	  	{
			cout<<"Lvl "<<i<<": idx:"<<lvls[i].idx<<", prefix: "<<lvls[i].prefix<<endl;

	  	}
}

vector<Synopsis::prefix> Synopsis::createPrefixHelper()
{
	vector<prefix> levels(shape.size());
  	for(int i=0;i<levels.size();i++)
  	{
		prefix pr;
		pr.prefix = 0;
		pr.idx = 0;
		//pr.previdx =0;
		//pr.prevfix=0;
		levels[i]=pr;

  	}
	return levels;



}
/*
bool Synopsis::rangeQueryOpt(Query::Query_t q)
{


  uint low = q.left;
  uint high = q.right;

  vector<prefix> levels = createPrefixHelper();

  int res_high,res_low;
  bool exists = navigateOpt(low,0,0,0,&levels,&res_high,&res_low);

  while(!exists && res_high<high)
  {

	exists = navigateOpt(res_high+1,0,0,0,&levels,&res_high,&res_low);

  }


  return exists;

}*/

bool Synopsis::rangeQueryOpt(Query::Query_t q)
{


  uint low = q.left;
  uint high = q.right;


  node res = navigateVerbose(low,0,0,0);

  while(!res.exists && res.high<high)
  {

	res = navigateVerbose(res.high+1,0,0,0);

  }


  return res.exists;

}




/* unused */
bool Synopsis::navigateOpt(int key,int lvl,int idx,int actual_idx,vector<Synopsis::prefix> * levels,int * res_high,int *res_low) {
	//we are at level lvl, looking at idx, idx +1, the children
	int low,high;
	getRange(lvl,actual_idx,&low,&high);
	int middle = lowMiddle(low,high);


	int next_lvl = lvl + 1;
	//int next_idx = getLeftChild(lvl,idx);
	int next_actual_idx = actual_idx<<1;
        int next_idx = levels->at(lvl).prefix + getLeftChild(lvl,levels->at(lvl).idx,idx);
#ifdef SYNDEBUG
	assert(next_idx == getLeftChild(lvl,idx));
#endif
	levels->at(lvl).prefix = next_idx;
	levels->at(lvl).idx = idx;

	if(key<=middle)
	{


		if(isLeaf(lvl,idx))
		{
			//cout<<"key :"<<key<<" lvl: "<<lvl<<" offset: "<<getLeafOffset(lvl,idx)<<endl;
                        int leaf_offset;
			*res_high = middle;
			*res_low = low;
			return getLeaf(lvl,idx,leaf_offset,true);
		}
	}
	else
	{

		if(isLeaf(lvl,idx+1))
		{
			*res_high = high;
			*res_low = ++middle;
                     	int leaf_offset;
			return 	getLeaf(lvl,idx+1,leaf_offset,true);
		}
		else //go to the right child
		{	if(!isLeaf(lvl,idx))
				next_idx+=2;
			next_actual_idx += 2;
		}
	}


	return navigateOpt(key,next_lvl,next_idx,next_actual_idx,levels,res_high,res_low);
}


long double Synopsis::getQueryTime(int mode)
{
	vector<double> * v = NULL;
	long double sum=0.0;
	if(mode == LOOKUP)
			v = &queryTimes;
	if(mode == ADAPT)
			v = &adaptTimes;
	if(mode == TRUNCATE)
			v = &truncateTimes;

	for(int i=0;i<v->size();i++)
		sum+=v->at(i);
	cout<<"Queries measured: "<<v->size()<<endl;
	return sum* 1000000.0 /v->size();

}

Synopsis::node Synopsis::navigateOpt(int key,int lvl,int idx,int actual_idx,vector<Synopsis::prefix> * levels) {
  	//we are at level lvl, looking at idx, idx +1, the children

	int low,high,middle,next_actual_idx,next_idx;
	struct node result;
    while(true)
    {


      	getRange(lvl,actual_idx,&low,&high);
      	middle = lowMiddle(low,high);

      	next_actual_idx = actual_idx<<1;

        int next_idx = levels->at(lvl).prefix + getLeftChild(lvl,levels->at(lvl).idx,idx);
      #ifdef SYNDEBUG
      	assert(next_idx == getLeftChild(lvl,idx));
      #endif
      	levels->at(lvl).prefix = next_idx;
      	levels->at(lvl).idx = idx;

  	if(key<=middle)
  	{


  		if(isLeaf(lvl,idx))
  		{
  			//cout<<"key :"<<key<<" lvl: "<<lvl<<" offset: "<<getLeafOffset(lvl,idx)<<endl;
                          int leaf_offset;

  			result.exists = getLeaf(lvl,idx,leaf_offset,true);
  			result.low = low;
  			result.high = middle;
  			result.lvl = lvl;
  			result.idx = idx;
  			result.leaf_idx = leaf_offset;
  			return 	result;
  		}
  	}
  	else
  	{

  		if(isLeaf(lvl,idx+1))
  		{
  			 int leaf_offset;

  			result.exists = getLeaf(lvl,idx+1,leaf_offset,true);
  			result.low = middle+1;
  			result.high = high;
  			result.lvl = lvl;
  			result.idx = idx+1;
  			result.leaf_idx = leaf_offset;
  			return 	result;
  		}
  		else //go to the right child
  		{	if(!isLeaf(lvl,idx))
  				next_idx+=2;
  			next_actual_idx += 2;
  		}
  	}
  	lvl++;
  	idx = next_idx;
  	actual_idx = next_actual_idx;

    } //while true loop :)

  }

Synopsis::state Synopsis::getRestartPoint(int bound,vector<Synopsis::state> &hist)
{
 // cout<<"hist"<<endl;
  struct state state;
  state.idx = 0;
  state.actual_idx = 0;
  state.lvl = 0;
  while(hist.size()>0) //find where to re-start the search
  {
	//cout<<"pop"<<endl;
	state = hist.back();
	int curr_high,curr_low;
	getRange(state.lvl,state.actual_idx,&curr_low,&curr_high);
	if(curr_high>= bound) //there is stuff to be found here
	{
		//if(first)
#ifdef SYNDEBUG
		//assert(state.wentLeft);
#endif
		hist.pop_back();
		break;
	}
	else
		hist.pop_back();


  }
  return state;
}


Synopsis::node Synopsis::navigate(int key,int lvl,int idx,int actual_idx,vector<Synopsis::state> * history, bool useHist) {
	//we are at level lvl, looking at idx, idx +1, the children
	int low,high;
	getRange(lvl,actual_idx,&low,&high);
	int middle = lowMiddle(low,high);

        //cout<<"range:"<<low<<"-"<<high<<endl;
	//cout<<"middle:"<<middle<<endl;
	int next_lvl = lvl + 1;
	int next_idx = getLeftChild(lvl,idx);
	int next_actual_idx = 2 * actual_idx;
	struct node result;
        struct state state;
	state.lvl = lvl;
        state.idx = idx;
        state.actual_idx = actual_idx;

	if(key<=middle)
	{
		state.wentLeft = true;
		state.lvl = lvl;
		 state.idx = idx;
		state.actual_idx = actual_idx;
		if(useHist)
			history->push_back(state);

		if(isLeaf(lvl,idx))
		{
			//cout<<"key :"<<key<<" lvl: "<<lvl<<" offset: "<<getLeafOffset(lvl,idx)<<endl;
                        int leaf_offset;
			result.exists = getLeaf(lvl,idx,leaf_offset,true);
			result.low = low;
			result.high = middle;
			result.lvl = lvl;
			result.idx = idx;
			result.leaf_idx = leaf_offset;

			return 	result;
		}
	}
	else
	{
		state.wentLeft = false;

		state.lvl = lvl;
				 state.idx = idx+1;
				state.actual_idx = 1 *(actual_idx) + 1;
		if(useHist)
			history->push_back(state);
		if(isLeaf(lvl,idx+1))
		{
			state.wentLeft = false;
			int leaf_offset;
			result.exists = getLeaf(lvl,idx+1,leaf_offset,true);
			result.low = middle+1;
			result.high = high;
  			result.lvl = lvl;
			result.idx = idx + 1;
                        result.leaf_idx = leaf_offset;

			return 	result;
		}
		else //go to the right child
		{	if(!isLeaf(lvl,idx))
				next_idx+=2;// getLeftChild(lvl,idx+1);
			next_actual_idx = 2 * (actual_idx + 1);
		}
	}


	return navigate(key,next_lvl,next_idx,next_actual_idx,history,useHist);
}


Synopsis::node Synopsis::navigateVerbose(int key,int lvl,int idx,int actual_idx) {
	//we are at level lvl, looking at idx, idx +1, the children
	int low,high;
	getRange(lvl,actual_idx,&low,&high);
	int middle = lowMiddle(low,high);
	int leaf_offset;
	int next_idx = -1;
	int next_actual_idx = actual_idx<<1;

	if(key<=middle)
	{

		if(isLeaf(lvl,idx))
		{
			struct node result;
			result.exists = getLeaf(lvl,idx,leaf_offset,true);
			result.low = low;
			result.high = middle;
			result.lvl = lvl;
			result.idx = idx;
			result.leaf_idx = leaf_offset;
			return result;
		}
		else
			next_idx = getLeftChild(lvl,idx);
	}
	else
	{
		if(isLeaf(lvl,idx+1))
		{
			struct node result;
			result.exists = getLeaf(lvl,idx+1,leaf_offset,true);
			result.low = middle+1;
			result.high = high;
			result.lvl = lvl;
			result.idx = idx+1;
			result.leaf_idx = leaf_offset;
			return result;
		}

		else //go to the right child
		{	if(next_idx == -1)
				next_idx = getLeftChild(lvl,idx+1);
			else
				if(!isLeaf(lvl,idx))
					next_idx+=2;

			next_actual_idx+= 2;
		}
	}

	return navigateVerbose(key,lvl+1,next_idx,next_actual_idx);
}


bool Synopsis::navigate(int key,int lvl,int idx,int actual_idx) {
	//we are at level lvl, looking at idx, idx +1, the children
	int low,high;
	getRange(lvl,actual_idx,&low,&high);
	int middle = lowMiddle(low,high);
	int leaf_offset;
	int next_idx = -1;
	int next_actual_idx = actual_idx<<1;

	if(key<=middle)
	{

		if(isLeaf(lvl,idx))
			return getLeaf(lvl,idx,leaf_offset,true);
		else
			next_idx = getLeftChild(lvl,idx);
	}
	else
	{
		if(isLeaf(lvl,idx+1))
			return getLeaf(lvl,idx+1,leaf_offset,true);

		else //go to the right child
		{	if(next_idx == -1)
				next_idx = getLeftChild(lvl,idx+1);
			else
				if(!isLeaf(lvl,idx))
					next_idx+=2;

			next_actual_idx+= 2;
		}
	}

	return navigate(key,lvl+1,next_idx,next_actual_idx);
}


int Synopsis::getRightChild(int left_child_idx,int parent_lvl,int parent_idx)
{

	if(!isLeaf(parent_lvl,parent_idx))
		return left_child_idx + 2;
	else
		return left_child_idx;
}

void Synopsis::print_vector(vector<bool> v,bool newline)
{
	for(int i=0;i<v.size();i++)
		cout <<v[i]<<" ";
	if(newline)
		cout<<endl;

}


int Synopsis::getLeftChild(int parent_lvl,int parent_idx)
{

	int idx = 0;
	for(int i=0;i<parent_idx;i++) // a sort of prefix
		//if(!isLeaf(parent_lvl,i))
			idx+=shape[parent_lvl][i];

	return idx<<1;
}

int Synopsis::getLeftChild(int parent_lvl,int start_idx,int parent_idx)
{

	int idx = 0;
	for(int i=start_idx;i<parent_idx;i++) // a sort of prefix
		//if(!isLeaf(parent_lvl,i))
			idx+=shape[parent_lvl][i];

	return idx<<1;
}



int Synopsis::getLeafOffset(int lvl,int idx)
{
	int offset = 0;
	for(int i=0;i<idx;i++) // a sort of prefix
			offset+=isLeaf(lvl,i);

	return offset;
}



int Synopsis::lowMiddle(int low, int high)
{

	//return low + floor((high-low)/2.0);
	return low + ((high-low)>>1);

}


void Synopsis::getRange(int lvl,int idx, int * low, int * high) //returns the range of values covered
// by the nodes [idx,  idx +1] in level lvl
{

	idx = idx >> 1;
	int step = (domain+1) >> (lvl);

	*low = idx* step;
	*high = (idx+1)*(step) -1;

}

void Synopsis::getSingleRange(int lvl,int idx, int * low, int * high) //returns the range of values covered
// by the node [idx] in level lvl
{
	int orig = idx;
	idx = idx >> 1;
	int step = (domain+1) >> (lvl);

	*low = idx* step;

	*high = (idx+1)*(step) -1;
	if(orig % 2 == 0)
		*high = lowMiddle(*low,*high);
	else
		*low = lowMiddle(*low,*high) + 1;


}


// simulates execution of range query on Synopsis
// (may result false positive; must not result in false negative)
bool Synopsis::pointQuery(int key) {
 	//struct timeval start,end;
	//double time;
	//tick(start);
	//bool res = navigate(key,0,0,0);
	//time = tock(start,end,false);
	//queryTime.push_back(time);
	return navigate(key,0,0,0);
}  // rangeQuery




void Synopsis::prettyPrint()
{
 	cout <<"TRIE (size = "<<size()<<")"<<endl;
	vector<int> positions; //positions at the top level
	positions.push_back(0);
	positions.push_back(1);
	prettyPrint(0,positions);
}

void Synopsis::prettyPrint(int lvl, vector<int> positions) //deprecated
{

	//we should not rely on the lenght of the shape/leaf vector
	//though, we should take care not to create extra levels by checking before doing the vector.append...
	int spaces = 1<<(shape.size() - lvl);
	string sep = " ";
	//cout<<"level: "<<lvl<<endl;
	string half="";
	for(int i=0; i<(spaces>>1)-1;i++ )
		half = half+sep;

	//print current level (without leaves for the time being)
	int prev = -1;
	/*for(int i=0;i<positions.size();i++)
		cout<<positions[i];
	cout<<endl;*/
	for(int i=0;i<positions.size();i++)
	{
		int pos = positions[i];
                //cout<<"leaf "<<i<<"has real pos:"<<pos<<endl;

		if(prev < pos -1)
		{
			for(int j=prev;j<pos-1;j++)
				cout<<(half+sep+sep+half);
		}
		string val = "1";
		if(isLeaf(lvl,i))
			val="0";
		cout<<(half+sep+val+half);
		prev = pos;
	}
	//cout <<" \t\t\tleaves["<<lvl<<"]:";
	//print_vector(leaves[lvl],false);
	//cout <<" \t\t\tused["<<lvl<<"]:";
	//print_vector(used[lvl],false);
        /*for(int i=0;i<leaves.size();i++)
	{
		//cout<<leaves[lvl][i]<<"-"<<used[lvl][i]<<" ";


	}*/

	cout<<endl;
	//find the new positions :)
	vector<int> new_pos;
	for (int i=0;i<shape[lvl].size();i++)
	{
		if(!isLeaf(lvl,i))
		{
			new_pos.push_back(2 * positions[i]);
			new_pos.push_back((2 * positions[i])+1);
		}
	}

	if(lvl < shape.size() -1)
		prettyPrint(lvl+1,new_pos);
}

void::Synopsis::resetFp()
{
	fp = 0;
	tn = 0;
	tp = 0;

}

void Synopsis::setDatabase(Database * d)
{
	database = d;
}


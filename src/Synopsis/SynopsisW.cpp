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
#include <algorithm>

void Synopsis::setId(int i)
{
	this->id=id;
}

bool Synopsis::handleQuery(Query::Query_t q,Database * database, bool doAdapt,bool qR)
{
   //cout<<"----Query :----"<<q.left<<"-"<<q.right<<endl;
    bool sR ;//rangeQuery(q);

#ifdef DOTRACE
    current_query = q;


#endif

#ifdef SYNDEBUG
    assert(q.left>=0 && q.right<=domain && q.right<=actual_size);
#endif

#ifdef TICKTOCK
 	 const uint64_t lookup0 = rdtscp();

#endif
	if(q.right>q.left)
		sR = rangeQueryOpt(q);
	else
	{
		sR = pointQuery(q.left);
		#ifdef SYNDEBUG
		/*node result = navigate(q.left,0,0,0,NULL,false);
		if(sR && !qR)
		{
			fps[result.lvl][result.leaf_idx]++;
		}*/
		#endif
	}

#ifdef TICKTOCK
	const uint64_t lookup1 = rdtscp();
	 if(lookup1>lookup0)
	 {
		 lookupt.push_front(lookup1-lookup0);
	 }

#endif
    
    assert(!(!sR && qR));
    if(sR && !qR)
    {
    	//cout<<"fp is "<<fp<<endl;
    	(fp)++;
    }
    if(!sR && !qR)
    	(tn)++;
    if(sR && qR)
    	(tp)++;
    if(!doAdapt)
    	return sR;

    if(sR && !qR) //false positive
    {

	#if defined(DOTRACE) && defined(TRACEQUERY)
		stringstream ss;
		ss<<"Trie "<<id<<" - Query ["<<q.left+lowerb<<"-"<<q.right+lowerb<<"] is a FALSE POSITIVE ";
		takeTextSnapshot(ss.str());
		vector<pair<int,int> > v;
		takeSnapshot(v); /*the before */

	#endif
	#ifdef TICKTOCK
		const uint64_t adapt0 = rdtscp();

	#endif

	learn_from_fp(q);

	#ifdef TICKTOCK
		uint64_t adapt1 = rdtscp();
		if(adapt1>adapt0)
		{
		 falsepos.push_front(adapt1-adapt0);
		}

	#endif

        
#ifdef SYNDEBUG
	assert(!rangeQuery(q)); //assert that we fixed teh false positive problem
#endif

    }
    if(sR && qR) //true positive
    {
		#if defined(DOTRACE) && defined(TRACEQUERY)
			stringstream ss;
			ss<<"Trie "<<id<<" - Query ["<<q.left+lowerb<<"-"<<q.right+lowerb<<"] is a TRUE POSITIVE ";
			takeTextSnapshot(ss.str());
			vector<pair<int,int> > v;
			takeSnapshot(v); /*the before */
		#endif

		#ifdef LEARNFROMTP
		vector<Query::Query_t> empty_ranges = database->determineEmptyRanges(q,lowerb,actual_size);
		//cout<<"learning from fp"<<endl;
		#ifdef TICKTOCK
		 //adapt0 = rdtscp();

		#endif
		learn_from_tp(&empty_ranges);
		#endif


    }
    if(!sR && !qR && this->learn_tn) //true negative -> adjust used values for Clock
    {
		#if (defined(DOTRACE) && defined(TRACEQUERY))
			stringstream ss;
			ss<<"Trie "<<id<<" -Query ["<<q.left+lowerb<<"-"<<q.right+lowerb<<"] is a TRUE NEGATIVE ";
			takeTextSnapshot(ss.str());
			vector<pair<int,int> > v;
			takeSnapshot(v); /*the before */
		#endif
		#ifdef TICKTOCK
		// adapt0 = rdtscp();

		#endif
		learn_from_tn(q);
		#ifdef SYNDEBUG
				//verify_tn(q);
		#endif

    }


    return sR;

} 



void Synopsis::recordNewKeys(uint * keys,int num_keys,int strategy) //Could be optimized if the keys were sorted...
//Complexity: O(N*num_keys)
{
	//vector<Synopsis::state> hist;
	//cout<<"num keys:"<<num_keys<<endl;
	for(int i=0;i<num_keys;i++)
	{
		struct node res = navigate(keys[i],0,0,0,NULL,false);

		//assert(res.exists == getLeafValue(res.lvl,res.leaf_idx));

		if(getLeafValue(res.lvl,res.leaf_idx))
		{
			continue;
		}

		//if(!getUsed(res.lvl,res.idx))
		if(strategy == STRAT0)
		{

			/*if(getUsed(res.lvl,res.leaf_idx))
				cout<<"---overwriting used----"<<endl;*/

			setLeafValue(res.lvl,res.leaf_idx,true); //now the leaf is occupied

		}


	}

}


void Synopsis::learn_from_fp(Query::Query_t q) //Goal: O(N) as opposed to O(range*N)
{
  //  cout<<"false positive"<<endl;
	current_query =  q;

	expand(q.left,true);
	expand(q.right,false);
	//uint64_t m1 = rdtscp();



	//m0 = rdtscp();
	//mark_leaf(q,0,0,0,true,true);
	mark_empty(q);
	 //m1 = rdtscp();
	//cout<<"time for mark empty:"<<m1-m0<<endl;
}

void Synopsis::mark_empty(Query::Query_t r)
{

		vector<prefix> levels = createPrefixHelper();
		//resetPrefixHelper();
		struct node res = navigateOpt(r.left,0,0,0,&levels);

		while( res.high <= r.right)
		{
		   if(res.low>=r.left)
		   {
		   	setLeafValue(res.lvl,res.leaf_idx,false);
		   }

		#if defined(DOTRACE) && ( defined(TRACEMARKEMPTY)) /* after marking empty */
			vector<pair<int,int> > special_nodes;
			pair<int,int> p;
			p.first =res.lvl;
			p.second = res.idx;
			special_nodes.push_back(p);
			takeSnapshot(special_nodes,TRACEMARKEMPTY);

		#endif

		   if(res.high == r.right) //we don't need to mark any more leaves, we did it :D
			break;

		   res = navigateOpt(res.high+1,0,0,0,&levels);
		   //printPrefixHelper();
		}


}

void Synopsis::learn_from_tp(vector<Query::Query_t> * empty_ranges) //exploit info about empty ranges
{ 
   //cout<<"true positive"<<endl;
   if(!use_tp || empty_ranges->empty())
	return;

   do
   {
    
    	/*int curr_high = */mark_leaf(empty_ranges->front(),0,0,0,true,false);
    	empty_ranges->erase(empty_ranges->begin());
	
	/*
	if(!empty_ranges.empty())
	{
		while(!empty_ranges.empty() && empty_ranges[0].right <= curr_high)
			empty_ranges.erase(empty_ranges.begin());
		if(!empty_ranges.empty())
			if(empty_ranges[0].left<=curr_high)
				state = getRestartPoint(curr_high + 1,hist);
			else
				state = getRestartPoint(empty_ranges[0].left,hist);
	}*/

   }while(!empty_ranges->empty());

   //Merge marked: see the leaves that were marked empty and see if they are mergeable
   // that would mean, if they are sibling leaves (same level) and continuous shape indices ie, 2,3
   // that means we merge them
 
   //merge_marked(marked);
}

void Synopsis::merge_marked(vector<Synopsis::node> marked) //FIXME: this is broken & therefore unused
{
 return; 

 if(marked.size()<2)
	return;

 vector<Synopsis::node> newmarked;
 newmarked.reserve(marked.size()/2);
 for(int i=marked.size()-1;i>1;i--)
 {

	if (marked[i].lvl>0 && marked[i].lvl == marked[i-1].lvl && (marked[i-1].idx % 2 == 0) 
		&& marked[i-1].idx + 1  == marked[i].idx 
		&& getLeafValue(marked[i].lvl,marked[i].leaf_idx) == getLeafValue(marked[i-1].lvl,marked[i-1].leaf_idx))
	{
		
		int parent_idx = getParentIdx(marked[i-1].lvl,marked[i-1].idx);
		merge(marked[i].lvl-1,marked[i-1].idx, parent_idx);
		//what i would need is the parent_leaf_idx
		int parent_leaf_offset = getLeafOffset(marked[i].lvl-1,parent_idx);
		struct node node;
		node.lvl = marked[i].lvl - 1;
		node.exists =getLeafValue(node.lvl,parent_leaf_offset);
		node.idx = parent_idx;
		assert(isLeaf(node.lvl,node.idx));
		newmarked.push_back(node);
		i--;
		
		//assert(getLeaf(marked[i].lvl-1,parent_idx,v, true) == (left|right));

	} 

 }
  //TODO: See if the sibling of every merged leaf can be merge with their parent ... //
	
  reverse(newmarked.begin(),newmarked.end());
  
 // cout<<"leafes"<<endl;
  for(int i=0;i<newmarked.size();i++)
	assert(isLeaf(newmarked[i].lvl,newmarked[i].idx));
 // cout<<"--------------------"<<endl;*/
 // merge_marked(newmarked);

}


uint Synopsis::mark_leaf(Query::Query_t r,int lvl,int idx,int actual_idx,bool markEmpty,bool falsePos) //if markEmpty  = true we mark the leaf as empty. otherwise we mark the used vector as used :)
{
	int sum = 0;
	vector<prefix> levels = createPrefixHelper();
   //cout<<"query: "<<r.left<<"-"<<r.right<<endl;
	struct node res = navigateOpt(r.left,lvl,idx,actual_idx,&levels);
        
	while((markEmpty && res.high <= r.right) ||
			(!markEmpty && !(res.low>r.right || res.high<r.left ))) /*res.low>=r.left &&*/
	{  //mark leafy as empty
	  // cout<<"Leaf range for marking:"<<res.low<<","<<res.high<<endl;
	   if(markEmpty && res.low>=r.left) //(falsePos || res.low>=r.left))
	   {	
		sum++;
		//cout<<"marked"<<endl;

		//assert(isLeaf(res.lvl,res.idx));
	   	setLeafValue(res.lvl,res.leaf_idx,false);
		#if defined(DOTRACE) && (defined(TRACETRUEPOS) || defined(TRACEMARKEMPTY)) /* after marking empty */
			vector<pair<int,int> > special_nodes;
			pair<int,int> p;
			p.first =res.lvl;
			p.second = res.idx;
			special_nodes.push_back(p);
			if(falsePos)
				takeSnapshot(special_nodes,TRACEMARKEMPTY);
			else
				takeSnapshot(special_nodes,TRACETRUEPOS);
		#endif

	   }
	   if(!markEmpty) 
	   {
		sum++;	
		//assert(!getLeafValue(res.lvl,res.leaf_idx));
		incrementUsed(res.lvl,res.leaf_idx);
		#if defined(DOTRACE) && defined(TRACEMARKUSED) /* after marking USED */
			vector<pair<int,int> > special_nodes;
			pair<int,int> p;
			p.first =res.lvl;
			p.second = res.idx;
			special_nodes.push_back(p);
			//cout<<"mark "<<res.lvl<<","<<res.idx<<" as used"<<endl;
			takeSnapshot(special_nodes,TRACEMARKUSED);
		#endif
			if(res.high>=r.right)
				break;


       	}


	   
	   if(res.high == r.right) //we don't need to mark any more leaves, we did it :D
		break; 
	  // vector<Synopsis::state> hist = *history;
	  // state = getRestartPoint(res.high+1,*history);
	   //cout<<"looking for"<<res.high+1<<endl;
	   res = navigateOpt(res.high+1,0,0,0,&levels);
	   //cout<<"res high:"<<res.high<<endl;
	   //cout<<"res.low:"<<res.low<<endl;

	  // res = navigate(res.high+1,0,0,0,NULL,false);

	}
	if(!markEmpty)
	{
		//assert(!getLeafValue(res.lvl,res.leaf_idx));
		incrementUsed(res.lvl,res.leaf_idx);
	}
	//cout<<"marked "<<sum<<endl;
 	
	return res.high;
}




void Synopsis::expand(int key,bool isLeft,bool expandAnyway)
{
  // vector<Synopsis::state> hist;
	vector<prefix> levels = createPrefixHelper();
	//resetPrefixHelper();
   struct node res= navigateOpt(key,0,0,0,&levels);
   //int middle = lowMiddle(res.low,res.high);
   int lvl = res.lvl;
   int idx = res.idx;
   bool exists = res.exists;
   int right_idx,left_idx;
   int low = res.low;
   int high = res.high;
   int splitted = 0;

   int existing_leaf = res.leaf_idx;
   //while(exists && ((isLeft && low!=key) || (!isLeft && high!=key)))
   while((exists || expandAnyway) && ((isLeft && low<key) || (!isLeft && high>key )))
   { 


	   splitted++;
	   cout<<"splitting containing leaf:"<<endl;
	   cout<<low<<" "<<high<<endl;
	//split
	//cout<<"range:"<<low<<"-"<<high<<endl;
	int middle = lowMiddle(low,high);
	bool left,right;
	left = true;
	right = true;
	if(expandAnyway)
	{
		if(key<=middle)
		{
			//cout<<"set empty range:"<<middle+1<<"-"<<high<<endl;

			right = false;
		}
		else
		{
			//cout<<"set empty range:"<<low<<"-"<<middle<<endl;
			left = false;
		}
	}
	if(db!=NULL)
	{
		//cout<<"we have a db"<<endl;
		left = db->rangeQuery(lowerb+low,lowerb+middle);
		right = db->rangeQuery(lowerb+middle+1,lowerb+high);
	}

    	if(key<=middle)
	{
		//uint64_t s0=rdtscp();
		split(lvl,idx,left,right,&left_idx,&right_idx,existing_leaf);
		//uint64_t s1=rdtscp();
		//cout<<"split lvl"<<lvl<<", idx "<<idx<<": "<<s1-s0<<endl;
		lvl = lvl +1;
		idx = left_idx;
		int offs;
		exists = getLeaf(lvl,idx,offs,true);
		high = middle;
		existing_leaf = offs;
	}
	else
	{
		  //uint64_t s0=rdtscp();
		 split(lvl,idx,left,right,&left_idx,&right_idx,existing_leaf);
		 //uint64_t s1=rdtscp();
		 //cout<<"split lvl"<<lvl<<", idx "<<idx<<": "<<s1-s0<<endl;
		 lvl = lvl + 1;
		 idx = right_idx;
		 int offs;
		 exists = getLeaf(lvl,idx,offs,true);
		 low = middle + 1;
		 existing_leaf = offs;
		
	}
   }

   cout<<"splitted:"<<splitted<<endl;

}


void Synopsis::merge() /*could also not be used because it's so pricey
 in terms of time */
{
  vector<prefix> levels = createPrefixHelper();
  mergeOpt(0,0,&levels);
  mergeOpt(0,1,&levels);
  /*merge(0,0);
  merge(0,1);*/
}


void Synopsis::mergeOpt(int lvl,int idx, vector<prefix> * levels) //we are at a parent, so we check the kids
{
	/*cout<<"merging empty , parent :"<<lvl<<","<<idx<<endl;
	cout<<"size of lvl :"<<shape[lvl].size()<<endl;
	for(int i=0;i<shape[lvl].size();i++)
		cout<<isLeaf(lvl,i)<<" ";
	cout<<endl;*/
	if(isLeaf(lvl,idx))
		return;
	int left_idx = levels->at(lvl).prefix + getLeftChild(lvl,levels->at(lvl).idx,idx); 
#ifdef SYNDEBUG
	assert(left_idx == getLeftChild(lvl,idx));
#endif
	levels->at(lvl).prefix = left_idx;
	levels->at(lvl).idx = idx;

        //so here, maybe we can reuse the already calculated offset..
	int right_idx = left_idx + 1;
	//empty leaves are merged in a bottom-up fashion :)
	//if it's the last level it should have only leaves, so I don't check for that
	if(!isLeaf(lvl+1,left_idx)) //check left child
	{
		mergeOpt(lvl+1,left_idx,levels);
		
	}
	if(!isLeaf(lvl+1,right_idx)) //check right child
	{
		mergeOpt(lvl+1,right_idx,levels);
	}

	int leaf_idx;

#ifndef ONLYEMPTY /* if we merge all redundant information */
	if(isRedundant(lvl,idx,left_idx,false))
#endif

#ifdef ONLYEMPTY /*if we only merge empty leaves */

	if(isLogicalLeaf(lvl,idx,left_idx,false) && getLeaf(lvl+1,left_idx,leaf_idx,true) == 0 && getLeaf(lvl+1,right_idx,leaf_idx+1)==0)
#endif
	{
		
		merge(lvl,left_idx,idx); //we merge something a level down, so how does this affect our prefixes
		//easy way: just reset them on the current level
		levels->at(lvl).idx = 0;
		levels->at(lvl).prefix = 0;
		
		//what about the next lvel?
		
	}
}


void Synopsis::merge(int lvl,int idx) //we are at a parent, so we check the kids
{
	////cout<<"merging empty , parent :"<<lvl<<","<<idx<<endl;
	if(isLeaf(lvl,idx))
		return;
	int left_idx;


	
 	left_idx = getLeftChild(lvl,idx);
	int right_idx = left_idx + 1;



	//empty leaves are merged in a bottom-up fashion :)
	//if it's the last level it should have only leaves, so I don't check for that
	if(!isLeaf(lvl+1,left_idx)) //check left child
	{
		merge(lvl+1,left_idx);
	}
	if(!isLeaf(lvl+1,right_idx)) //check right child
	{
		merge(lvl+1,right_idx);
	}

	int leaf_idx;
	
	if(isRedundant(lvl,idx,left_idx,false))
	/*if(isLogicalLeaf(lvl,idx,left_idx,false) && getLeaf(lvl+1,left_idx,leaf_idx,true) == 0 && getLeaf(lvl+1,right_idx,leaf_idx+1) == 0)*/
	 //we can merge it :)
        {	
		
		merge(lvl,left_idx,idx); //level of parent, left child idx, parent_idx
	}
}

void Synopsis::merge(int parent_lvl, int left_idx, int parent_idx) 
{
  int lvl = parent_lvl;
 // //cout<<"merging the children of "<<lvl<<","<<parent_idx<<endl;
  bool updateClock = false;

  if(/*repl_policy == CLOCK &&*/ lvl+1 == prev_victim.lvl && (prev_victim.idx == left_idx || prev_victim.idx == left_idx+1))
	updateClock = true;
  int right_idx = left_idx + 1;
#ifdef SYNDEBUG
  assert(isLeaf(lvl+1,left_idx) && isLeaf(lvl+1,right_idx));
#endif
  int leaf_idx;
  bool left = getLeaf(lvl+1,left_idx,leaf_idx,true);
  int right_leaf = leaf_idx + 1;
  bool right = getLeaf(lvl+1,left_idx,right_leaf);
  bool merged_val = left | right;
 // int used_val = std::max(getUsed(lvl+1,leaf_idx),getUsed(lvl+1,right_leaf));
  int used_val = getUsed(lvl+1,leaf_idx) + getUsed(lvl+1,right_leaf);
  //bool lu = getUsed(lvl+1,leaf_idx);
  //bool ru = getUsed(lvl+1,right_leaf);
  
#if defined(DOTRACE) && defined(TRACEMERGE)
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

#endif


  removeValue(lvl+1,left_idx);
  removeValue(lvl+1,left_idx); //this is not a mistake 
  removeLeafValue(lvl+1,leaf_idx);
  removeLeafValue(lvl+1,leaf_idx);//this is not a mistake
  setValue(lvl,parent_idx,false); //parent is turned into a leaf node
#ifdef SYNDEBUG
  assert(isLeaf(lvl,parent_idx));
#endif
  int leaf_offset = getLeafOffset(lvl,parent_idx);
  insertLeafValue(lvl,leaf_offset ,merged_val); //used will initially be zero or N/A

  if(!merged_val) //if it is an empty leaf, adjust its used counter
  {
	  setUsed(lvl,leaf_offset,used_val);
  }

#ifdef SYNDEBUG
  assert(!merged_val || !getUsed(lvl,getLeafOffset(lvl,parent_idx)));
#endif	
  if(clock_bits ==0)
	  num_bits-=3 ;
  if(clock_bits==1)
  {
	  num_bits = num_bits - 3 + !merged_val - !left - !right;
  }

  //if we merged a prev.victim -> the prev victim is the parent
  if(updateClock)
  {
	prev_victim.lvl = lvl;
	prev_victim.idx = parent_idx;
	//print_previous_victim();

  }

#if defined(DOTRACE) && defined(TRACEMERGE)

				special_nodes.pop_back();
				special_nodes.pop_back();
				takeSnapshot(special_nodes,TRACEMERGE);

#endif
   

}

void Synopsis::split(int lvl, int idx,bool left, bool right,int *lidx,int *ridx,int existing_leaf)
//TODO: assert we don't split a node that's already [x,x]
{	//cout<<"splitting:"<<lvl<<" , "<<idx<<endl;
	bool updateClock = false;



#if defined(DOTRACE) && defined(TRACESPLIT) /* before splitting */
	vector<pair<int,int> > special_nodes;
	pair<int,int> p;
	p.first =lvl;
	p.second = idx;
	special_nodes.push_back(p);
	takeSnapshot(special_nodes,TRACESPLIT);
#endif

#ifdef SYNDEBUG
	assert(1<<(lvl+1) < domain + 1);
	assert(existing_leaf == getLeafOffset(lvl,idx));
#endif
	if(prev_victim.lvl == lvl && prev_victim.idx == idx)
		updateClock = true;
	//int existing_leaf = getLeafOffset(lvl,idx);
	int parent_val = getLeafValue(lvl,existing_leaf);
	removeLeafValue(lvl,existing_leaf); //we remove the leaf from current level

#ifdef SYNDEBUG
	assert(isLeaf(lvl,idx));
#endif
	setValue(lvl,idx,true); //we overwrite the value of the node
#ifdef SYNDEBUG
	assert(!isLeaf(lvl,idx));
#endif

	
	//put children in the shape lvl+1
	//and leaf values in the leaves[lvl+1]



	int left_idx = getLeftChild(lvl,idx);
	insertValue(lvl+1,left_idx,false);
	insertValue(lvl+1,left_idx+1,false);
	int leaf_idx = getLeafOffset(lvl+1,left_idx);
	//cout<<"leaf idx:"<<leaf_idx<<endl;
	insertLeafValue(lvl+1,leaf_idx,left);
	insertLeafValue(lvl+1,leaf_idx+1,right);	
	*lidx = left_idx;
	*ridx = left_idx + 1;

	 if(clock_bits ==0)
		  num_bits+=3 ;
	  if(clock_bits==1)
	  {
		  num_bits = num_bits + 3 - (!parent_val) +(!left)  +(!right);
	  }


	//if we split the prev victim  prev victim is the left child
	if(/*repl_policy == CLOCK &&*/ updateClock)
	{ 
		prev_victim.lvl = lvl + 1;
		prev_victim.idx = left_idx;
		//print_previous_victim();
	}

#if defined(DOTRACE) && defined(TRACESPLIT)

	pair<int,int> pl,pr;
	pl.first =lvl+1;
	pl.second = left_idx;
	pr.first =lvl+1;
	pr.second = left_idx+1;
	special_nodes.push_back(pl);
	special_nodes.push_back(pr);
	takeSnapshot(special_nodes,TRACESPLIT);
#endif

	

}

void Synopsis::grow()
{
  shape.push_back(vector<bool> ());
  leaves.push_back(vector<bool> ());
  if(clock_bits>0)
	  used.push_back(vector<bool> ());

  //shape.back().reserve(256);
  //leaves.back().reserve(256);
  //used.back().reserve(256);
  //reserve space to make a point about memory alloc'
  //slowing down our progrum :(
}

 void Synopsis::setValue(int lvl,int idx,bool val)
{
#ifdef SYNDEBUG
  assert(lvl<shape.size() && idx <shape[lvl].size());
#endif
  shape[lvl][idx]=val;
}
 

void Synopsis::setLeafValue(int lvl,int leaf_idx,bool val)
{
  int idx = leaf_idx;
  bool prev = getLeafValue(lvl,leaf_idx);
#ifdef SYNDEBUG
  assert(lvl<leaves.size() && idx <leaves[lvl].size());
  assert(clock_bits ==0 || (getEmptyLeaves(lvl) == used[lvl].size()));
#endif

  if(clock_bits>0)
  {
	  if(prev && !val)
	  {
		  int offset = getUsedOffset(lvl,idx);
		  used[lvl].insert(used[lvl].begin() + offset,0);
	  }
	  if(!prev && val)
	  {
		  int offset = getUsedOffset(lvl,idx);
		  used[lvl].erase(used[lvl].begin() + offset);
	  }
  }
  leaves[lvl][idx]=val;
#ifdef SYNDEBUG
  assert(clock_bits ==0 || (getEmptyLeaves(lvl) == used[lvl].size()));
#endif
}

 void Synopsis::insertValue(int lvl,int idx,bool val)
{
	// uint64_t t0 = rdtscp();
  if(shape.size() -1 < lvl)
  {
	//cout<<"grew!"<<endl;
	grow();
  }
  shape[lvl].insert(shape[lvl].begin() + idx,val);
  //uint64_t t1 = rdtscp();
  //cout<<"clock cycles for inserting a value in vector bool:"<<t1-t0<<endl;
}
 

void Synopsis::insertLeafValue(int lvl,int idx,bool val)
{
#ifdef SYNDEBUG
	 assert(clock_bits ==0 || (getEmptyLeaves(lvl) == used[lvl].size()));
#endif
    if(leaves.size() -1 < lvl)
    {
	//cout<<"grew!"<<endl;
	grow();
    }
  leaves[lvl].insert(leaves[lvl].begin() + idx,val);
  if(clock_bits>0 && !val)
  {
	  int offset = getUsedOffset(lvl,idx);
	  used[lvl].insert(used[lvl].begin() + offset,0);
  }
#ifdef SYNDEBUG
  assert(clock_bits ==0 || (getEmptyLeaves(lvl) == used[lvl].size()));
#endif
}

 void Synopsis::removeValue(int lvl,int idx)
{
  ////cout<<"removing "<<lvl<<",idx: "<<idx<<endl;
#ifdef SYNDEBUG
  assert(lvl<shape.size() && idx <shape[lvl].size());
#endif
  shape[lvl].erase(shape[lvl].begin() + idx);
}
 

void Synopsis::removeLeafValue(int lvl,int idx)
{
/*#ifdef SYNDEBUG
  assert(lvl<leaves.size() && lvl<used.size() && idx <leaves[lvl].size() && idx<used[lvl].size());
#endif */
	 //assert(getEmptyLeaves(lvl) == used[lvl].size());
  if(clock_bits>0 && !getLeafValue(lvl,idx))
  {
	  int offset = getUsedOffset(lvl,idx);
	  used[lvl].erase(used[lvl].begin() + offset);
  }

  leaves[lvl].erase(leaves[lvl].begin() + idx);
 // assert(getEmptyLeaves(lvl) == used[lvl].size());
}


void Synopsis::verify_tn(Query::Query_t q)
{
	//vector<state> hist;
	uint l = q.left;
	uint r = q.right;

	for(uint i=l;i<=r;i++)
	{
		struct node res = navigate(i,0,0,0,NULL,false);
		assert(getUsed(res.lvl,res.leaf_idx));
	}

}





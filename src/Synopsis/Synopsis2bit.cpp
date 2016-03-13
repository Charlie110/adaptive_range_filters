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

Synopsis2bit::Synopsis2bit(int domain,int limit,bool tp,Database * db,int low):Synopsis(domain,limit,tp,db,CLOCK,low) {

cout<<" i r different!!"<<endl;
this->clock_bits = 2;
num_bits = 8;
}

void Synopsis2bit::init()
{

	resetTime();

	grow();
        insertValue(0,0,false);
        insertValue(0,1,false);
        insertLeafValue(0,0,true);
    	assert(leaves[0].size() == 1);
	assert(used[0].size() == 2 );
        insertLeafValue(0,1,true);
	assert(leaves[0].size() == 2);
	assert(used[0].size() ==4 );

}

Synopsis2bit:: ~Synopsis2bit(){
}


void Synopsis2bit::incrementUsed(int lvl, int leaf_idx)
{
#ifdef SYNDEBUG
   assert(!getLeafValue(lvl,leaf_idx));
#endif
   int curr = getUsed(lvl,leaf_idx);
   if(curr==3)
	return;
  setUsed(lvl,leaf_idx,curr+1);

}

void Synopsis2bit::setUsed(int lvl,int leaf_idx,int val)
{
#ifdef SYNDEBUG
	assert(val<4 && val>=0);
	assert(val ==0 || !getLeafValue(lvl,leaf_idx));
#endif

	int bit0 = val % 2;
	int bit1 = val / 2;
	used[lvl][leaf_idx<<1] = bit1;
	used[lvl][(leaf_idx<<1)+1]= bit0;

}

void Synopsis2bit::decrementUsed(int lvl, int leaf_idx)
{

   int curr = getUsed(lvl,leaf_idx);
   if(curr==0)
	return;
  setUsed(lvl,leaf_idx,curr-1);



}


int Synopsis2bit::getUsed(int lvl,int leaf_idx)
{

  int bit[2];
  bit[1] = used[lvl][leaf_idx<<1];
  bit[0] = used[lvl][(leaf_idx<<1)+1];
  int val =  bit[1]<<1 | bit[0];
  //cout<<"value:"<<val<<endl;
  return val;

}

void Synopsis2bit::insertLeafValue(int lvl,int idx,bool val)
{

    if(leaves.size() -1 < lvl)
    {
	//cout<<"grew!"<<endl;
	grow();
    }
  leaves[lvl].insert(leaves[lvl].begin() + idx,val);
  used[lvl].insert(used[lvl].begin() + (2*idx),false);
  used[lvl].insert(used[lvl].begin() + (2*idx)+1,false);
  assert(2*leaves[lvl].size() == used[lvl].size());

}

void Synopsis2bit::removeLeafValue(int lvl,int idx)
{
 int prev_u = used[lvl].size();
 int prev_l = leaves[lvl].size();
#ifdef SYNDEBUG
  assert(lvl<leaves.size() && lvl<used.size() && idx <leaves[lvl].size() /*&& idx<2*used[lvl].size()*/);
  assert(used[lvl].size()>=(2*idx) +1);
#endif
  leaves[lvl].erase(leaves[lvl].begin() + idx);
  used[lvl].erase(used[lvl].begin() + (2*idx));
  used[lvl].erase(used[lvl].begin() + (2*idx));
  assert(2*leaves[lvl].size() == used[lvl].size());
}




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

Synopsis0bit::Synopsis0bit(int domain,int limit,bool tp,Database * db,int low):Synopsis(domain,limit,tp,db,RANDOM,low) {

//cout<<" i no haz clock"<<endl;
num_bits = 4;
this->clock_bits = 0;
}

Synopsis0bit::Synopsis0bit(Synopsis * s, vector< vector<int> > weights):Synopsis(s,weights,0)
{
	cout<<"Converted integer clock into 0-bit clock"<<endl;
}

void Synopsis0bit::grow()
{
  shape.push_back(vector<bool> ());
  leaves.push_back(vector<bool> ());

}

void Synopsis0bit::init()
{

	resetTime();

	grow();
        insertValue(0,0,false);
        insertValue(0,1,false);
        insertLeafValue(0,0,true);
        insertLeafValue(0,1,true);

}

Synopsis0bit:: ~Synopsis0bit(){
}


void Synopsis0bit::incrementUsed(int lvl, int leaf_idx)
{

	return;

}

void Synopsis0bit::setUsed(int lvl,int leaf_idx,int val)
{

	return;

}

void Synopsis0bit::decrementUsed(int lvl, int leaf_idx)
{
   return;
}


int Synopsis0bit::getUsed(int lvl,int leaf_idx)
{

  return 0;

}

void Synopsis0bit::insertLeafValue(int lvl,int idx,bool val)
{

    if(leaves.size() -1 < lvl)
    {
	//cout<<"grew!"<<endl;
	grow();
    }
  leaves[lvl].insert(leaves[lvl].begin() + idx,val);


}

void Synopsis0bit::removeLeafValue(int lvl,int idx)
{

  leaves[lvl].erase(leaves[lvl].begin() + idx);

}




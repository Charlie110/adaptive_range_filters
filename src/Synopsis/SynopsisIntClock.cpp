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

SynopsisIntClock::SynopsisIntClock(int domain,int limit,bool tp,Database * db,int low):Synopsis(domain,limit,tp,db,CLOCK,low) {

//cout<<" i haz integer weights!"<<endl;
//this->clock_bits = 0;
//num_bits = 4;
this->clock_bits = 1;
this->num_bits = 6;

}

void SynopsisIntClock::grow()
{
  shape.push_back(vector<bool> ());
  leaves.push_back(vector<bool> ());
  weights.push_back(vector<int>());

}

Synopsis::Synopsis(Synopsis * s, vector< vector<int> > weights,int clock_bits)
{

	cout<<"converting integer clock to "<<clock_bits<<"-bit clock..."<<endl;

	/* TODO */


}

void Synopsis::convertToCompact(int cl)
{
	this->clock_bits = cl;
	if(cl==0)
	{

		return;
	}
	if(cl == 1)
	{
		for(int i=0;i<used.size();i++)
			used[i].clear();
		used.clear();

		for(int i=0;i<weights.size();i++)
		{
			used.push_back(vector<bool>());
			for(int j=0;j<weights[i].size();j++)
			{
				if(!getLeafValue(i,j))
					used[i].push_back(weights[i][j]>0);
			}
		}
		//sanityCheckUsed();
	}

}

void SynopsisIntClock::init()
{

	resetTime();

	grow();
        insertValue(0,0,false);
        insertValue(0,1,false);
        insertLeafValue(0,0,true);
        insertLeafValue(0,1,true);


}

SynopsisIntClock:: ~SynopsisIntClock(){
}


void SynopsisIntClock::incrementUsed(int lvl, int leaf_idx)
{
	if(getLeafValue(lvl,leaf_idx))
		  return;
	 weights[lvl][leaf_idx]++;

}

void SynopsisIntClock::setUsed(int lvl,int leaf_idx,int val)
{


  if(getLeafValue(lvl,leaf_idx))
	  return;
  weights[lvl][leaf_idx] = val;

}

void SynopsisIntClock::decrementUsed(int lvl, int leaf_idx)
{
	if(getLeafValue(lvl,leaf_idx))
		  return;
	if(weights[lvl][leaf_idx]==0)
		return;
#ifdef SYNDEBUG
	assert(weights[lvl][leaf_idx]>0);
#endif
	weights[lvl][leaf_idx]--;


}


int SynopsisIntClock::getUsed(int lvl,int leaf_idx)
{

 // cout<<"integer used:"<<weights[lvl][leaf_idx]<<endl;
	if(getLeafValue(lvl,leaf_idx))
		  return 0;
  return weights[lvl][leaf_idx];

}

void SynopsisIntClock::setLeafValue(int lvl,int idx,bool val)
{

	#ifdef SYNDEBUG
	  assert(lvl<leaves.size() && idx <leaves[lvl].size());

	#endif



	  leaves[lvl][idx]=val;


}

void SynopsisIntClock::insertLeafValue(int lvl,int idx,bool val)
{

    if(leaves.size() -1 < lvl)
    {
	//cout<<"grew!"<<endl;
	grow();
    }
  leaves[lvl].insert(leaves[lvl].begin() + idx,val);
  weights[lvl].insert(weights[lvl].begin() + idx,0);


}

void SynopsisIntClock::removeLeafValue(int lvl,int idx)
{


  leaves[lvl].erase(leaves[lvl].begin() + idx);
  weights[lvl].erase(weights[lvl].begin() + idx);
}




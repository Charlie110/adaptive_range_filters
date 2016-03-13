/*
 * QuadSynopsisescalate.cpp
 *
 *  Created on: Dec 19, 2012
 *      Author: carolinux
 */


#include "QuadSynopsis.h"



void QuadSynopsis::learn_from_fp(vector<int> low, vector<int> high)
{

	cq.low = low;
	cq.high = high;
	//expand

	/*expand(low,true);
	takeSnapshot("after left expand");
	expand(high,false);
	takeSnapshot("after right expand");*/

	expandQueryBox(low,high);

	//takeSnapshot("after mark empty");

	//mark empty


}



void QuadSynopsis::expandQueryBox(vector<int> low, vector<int> high)
{
	return expandQueryBox(0,0,low,high,lowp,highp);
}

void QuadSynopsis::perfect(Database *database)
	{
		if(database==NULL)
		{
			cout<<"Warning: can't make perfect."<<endl;
		}
		else
		{

			currentMark = false;
			learn_from_fp(lowp,highp);
			currentMark = true;
			takeSnapshot("after marking emptie");

			vector< vector<int> > pts = database->getPoints();

			for(int i=0;i<pts.size();i++)
			{
				//mark as occupied
				//cout<<"Marking point as occupied"<<endl;
				//database->curve->printp(pts[i]);
				learn_from_fp(pts[i],pts[i]);
				//getchar();
			}


			//TODO: merge all equal thingeys for graet justice

			currentMark = false;
			for(int i=0;i<shape.size();i++)
				merge_equal();

		}
	}

void QuadSynopsis::carve(int lvl,int idx,vector<int> low,vector<int> high,
		vector<int> clow,vector<int> chigh)
{

	/*cout<<"curr raeng:"<<endl;
	cout<<clow[0]<<","<<clow[1]<<"-"<<chigh[0]<<","<<chigh[1]<<endl;
	cout<<"point:"<<endl;
	cout<<low[0]<<","<<low[1]<<endl;*/


	int leaf_idx = getLeafOffset(lvl,idx);


	vector<vector<int> > highs = getChildrenUpperBounds(clow,chigh);
	vector<vector<int> > lows = getChildrenLowerBounds(clow,chigh);
	int nc = lows.size();

	vector<bool> leaf_vals(nc);
		std::fill(leaf_vals.begin(),leaf_vals.end(),!currentMark);




	curr_leaf.lvl = lvl;
		curr_leaf.idx = idx;
	//takeSnapshot("before one split");

	int first_child;
	if(isLeaf(lvl,idx))
	{
		if(getLeafValue(lvl,leaf_idx)==currentMark) /* it is already empty/occupied */
		{
			return;
		}
		else
		{
			if(db!=NULL)
			{
				for(int i=0;i<numChildren();i++)
					if(lows[i][0]>=0 && currentMark == false)
						leaf_vals[i] = db->rangeQuery(lows[i],highs[i],cdim);
					else
						leaf_vals[i] = false;
			}
			split(lvl,idx,leaf_vals,&first_child,leaf_idx);
			//takeSnapshot("after one split");
		}
	}
	else
		first_child = getFirstChild(lvl,idx);

	//continue carving overlapping childrens :)



	for(int i=0;i<nc;i++)
	{
		//cout<<"range of child :"<<i<<endl;
		//cout<<lows[i][0]<<","<<lows[i][1]<<"-"<<highs[i][0]<<","<<highs[i][1]<<endl;

		if(leaf_vals[i]!=currentMark && overlap(lows[i],highs[i],low,high) &&
				!isContained(lows[i],highs[i],low,high))
		{


			carve(lvl+1,first_child+i,low,high,lows[i],highs[i]);
		}
		else
		{
			if(isContained(lows[i],highs[i],low,high))
				mark_empty(lvl+1, first_child+i);
		}

	}


}


bool QuadSynopsis::exceedsAllDims(vector<int> key, vector<int> threshold)
{
	for(int i=0;i<dim;i++)
	{
		if(key[i]<=threshold[i])
			return false;
	}
	return true;
}

void QuadSynopsis::expandQueryBox(int lvl, int first_idx,
		vector<int> lowpt, vector<int> highpt,vector<int> clow, vector<int> chigh)
{

	//cout<<"expand query box!"<<endl;

	bool res =false;
	vector<vector<int> > highs = getChildrenUpperBounds(clow,chigh);
	vector<vector<int> > lows = getChildrenLowerBounds(clow,chigh);



	for(int i=0;i<lows.size();i++)
	{
		int idx = first_idx +i;

		//if(exceedsAllDims(lowpt,high))
		//	return;
		//cout<<lows[i][0]<<" "<<lows[i][1]<<"-"<<highs[i][0]<<" "<<highs[i][1]<<endl;

		if(overlap(lows[i],highs[i],lowpt,highpt) && !isContained(lows[i],highs[i],lowpt,highpt))
		{

			//cout<<"carving!!"<<endl;

			carve(lvl,idx,lowpt,highpt,lows[i],highs[i]);
		}
		else
		{
			if(isContained(lows[i],highs[i],lowpt,highpt))
			{
				mark_empty(lvl,idx);
			}
		}
	}
}

void QuadSynopsis::mark_empty(int lvl, int idx)
{

	if(isLeaf(lvl,idx))
	{
		setLeafValue(lvl,getLeafOffset(lvl,idx),currentMark);
	}
	else
	{
		int first_idx = getFirstChild(lvl,idx);
		for(int i=0;i<numChildren();i++)
		{
			mark_empty(lvl+1,first_idx+i);
		}

	}


}

bool QuadSynopsis::sanityCheck2()
{
	return true;
	vector<int> q(2),p(2);
	q[0] =394;
	q[1] =168;
	p[0] = 426;
	p[1] = 223;

	bool r = db->rangeQuery(q,p);
	assert(r == true);
	bool quad = rangeQuery(q,p);
	assert(quad == true);
	return true;


	int MAX = 1023;
	for(int i=0;i<MAX;i++)
	{
		for(int j=0;j<MAX;j++)
		{
			vector<int> pt(2);
			pt[0] = i;
			pt[1] = j;
			bool quad_res = pointQuery(pt);
			bool db_res = db->rangeQuery(pt,pt);
			assert(!(!quad_res && db_res));
		}
	}

	return true;
}


bool QuadSynopsis::sanityCheck()
{
	return sanityCheck(0,0,lowp,highp);
}

bool QuadSynopsis::sanityCheck(int lvl,int idx, vector<int> clow, vector<int> chigh)
{


	vector<vector<int> > highs = getChildrenUpperBounds(clow,chigh);
	vector<vector<int> > lows = getChildrenLowerBounds(clow,chigh);



	for(int i=0;i<numChildren();i++)
	{
		if(isLeaf(lvl,idx+i))
		{
			bool leaf_val = getLeaf(lvl,idx+i);
			bool db_val = db->rangeQuery(lows[i],highs[i],cdim);
			assert(!(!leaf_val && db_val));
		}
		else
		{
			sanityCheck(lvl+1,getFirstChild(lvl,idx+i),lows[i],highs[i]);
		}
	}

	return true;
}


bool QuadSynopsis::handleQuery(Query::Query_t q,
		bool doAdapt,bool qR)
{
	Query::QueryMD_t qm;
	qm.low = vector<int>(1);
	qm.high = vector<int>(1);
	qm.low[0] = q.left;
	qm.high[0] = q.right;
	return handleQuery(qm,doAdapt,qR);


}


bool QuadSynopsis::handleQuery(Query::QueryMD_t q, bool doAdapt,bool qR)
{


	sanityCheck2();
	cq = q;
	bool sR;
	bool pointq = (compareAllDim(q.low,q.high) == QEQUAL);
	if(pointq)
	{
		sR = pointQuery(q.low);
	}
	else
	{
		ranges_seen = 0;
		sR = rangeQuery(q.low,q.high);
		stats.updateRange(ranges_seen);
		ranges_seen = 0;
	}
    stats.update(sR,qR);


	if(!sR && qR) //axreiasto na'nai :)
	{
		rangeQuery(q.low,q.high);
		takeSnapshot("o noes");
		cout<<"error on query:"<<endl;
	    	print_point(q.low);
	    	print_point(q.high);

	}
	assert(!(!sR && qR));
    if(!doAdapt)
    	return sR;

    if(sR && !qR) //false positive
    {
    	//cout<<"FP for query:"<<endl;
    	//print_point(q.low);
    	//print_point(q.high);


    	currentMark = false;
    	learn_from_fp(q.low,q.high);
    	//takeSnapshot("after esc");
    	assert(!pointQuery(q.low));
    	assert(!pointQuery(q.high));
    	assert(!rangeQuery(q.low,q.high));



    }
    if(sR && qR) //true positive
    {


    	//we don't need to
    	//do something, after using the db for splitting, i think..

    }
    if(!sR && !qR)
    {
		learn_from_tn(q.low,q.high);
    }


    return sR;

}


void QuadSynopsis::split(int lvl, int idx,
		vector<bool> leaf_vals,int *fidx,int existing_leaf_idx)
{


	bool updateClock = false;


	assert(existing_leaf_idx == getLeafOffset(lvl,idx));

	if(prev_victim.lvl == lvl && prev_victim.idx == idx)
		updateClock = true;

	int parent_val = getLeafValue(lvl,existing_leaf_idx);
	removeLeafValue(lvl,existing_leaf_idx); //we remove the leaf from current level
	setValue(lvl,idx,true); // now it's an internal node


	int first_idx = getFirstChild(lvl,idx);
	for(int i=0;i<numChildren();i++)
		insertValue(lvl+1,first_idx,false);

	int leaf_idx = getLeafOffset(lvl+1,first_idx);

	for(int i=0;i<numChildren();i++)
		insertLeafValue(lvl+1,leaf_idx+i,leaf_vals[i]);
	//TODO: OR insert the vector

	*fidx = first_idx;


	num_bits+=(-1 + numChildren()*2) + (-1 + numChildren())*used_bit_size;



	if(updateClock)
	{
		prev_victim.lvl = lvl + 1;
		prev_victim.idx = first_idx;

	}



	}






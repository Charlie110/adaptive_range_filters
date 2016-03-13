/*
 * SynopsisDFS.cpp
 *
 *  Created on: Nov 19, 2012
 *      Author: carolinux
 */

#include "SynopsisDFS.h"
#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>


bool SynopsisDFS::handleQuery(Query::Query_t q)
{   bool result;
    const uint64_t lookup0 = rdtscp();
	if(q.left == q.right)
		result = pointQuery(q.left);
	else
		result = rangeQuery(q.left,q.right);
	const uint64_t lookup1 = rdtscp();
	if(lookup1>lookup0)
		lookupt.push_back(lookup1-lookup0);

	return result;
}
void SynopsisDFS::serialize(Synopsis& syn,int idx,int lvl)
{

		shape.push_back(syn.shape[lvl][idx]);
		shape.push_back(syn.shape[lvl][idx+1]);
		if(syn.isLeaf(lvl,idx))
		{
			int offset;
			leaves.push_back(syn.getLeaf(lvl,idx,offset,true));
		}
		if(syn.isLeaf(lvl,idx+1))
		{
			int offset;
			leaves.push_back(syn.getLeaf(lvl,idx+1,offset,true));
		}

		if(!syn.isLeaf(lvl,idx))
		{
			serialize(syn,syn.getLeftChild(lvl,idx),lvl+1);
		}

		if(!syn.isLeaf(lvl,idx+1))
		{
			serialize(syn,syn.getLeftChild(lvl,idx+1),lvl+1);
		}
}


SynopsisDFS::SynopsisDFS(Synopsis & syn){

	this->fp = 0;
	this->domain = syn.domain;
	this->lowerb = syn.lowerb;
	serialize(syn,0,0);

}

bool SynopsisDFS::isLeaf(int idx)
{
	assert(idx<shape.size());
	return !shape[idx];
}

bool SynopsisDFS::leafValue(int idx)
{
	assert(idx<leaves.size());
	return leaves[idx];
}

int SynopsisDFS::skipSubtree(int left, int * leaf)
{


	int right = left +1;
	char sum = isLeaf(left) + isLeaf(right);
	if(sum == 2)
	{
		(*leaf)+=2;
		return left + 2;
	}

	if(sum == 1)
	{
		(*leaf)++;
		return skipSubtree(left+2,leaf);
	}


	if(sum == 0)
	{
		int idx = skipSubtree(left+2,leaf);
		return skipSubtree(idx,leaf);
	}


}




bool SynopsisDFS::pointQuery(uint key)
{
	int left_child = 0; //idx of the current left thing
	int leaf = 0; //leaf idx of the next leaf (either left or right)
	int low = lowerb;
	int high = lowerb+domain;
	while(true)
	{
		int middle = low + ((high-low)>>1);
		if(key<=middle)
		{
			if(isLeaf(left_child))
				return leafValue(leaf);
			else //go to left child
			{
				leaf+=isLeaf(left_child+1);
				left_child = left_child + 2;
				high = middle;
			}
		}
		else //go right
		{
			if(isLeaf(left_child+1))
			{
				leaf+=isLeaf(left_child);
				return leafValue(leaf);

			}
			else //right child is not a leaf
			{
				if(isLeaf(left_child))
				{
					left_child+=2;
					leaf++;
				}
				else
				{

					left_child =skipSubtree(left_child+2,&leaf);
				}
				low = middle + 1;
			}


		}

	}

}
bool SynopsisDFS::rangeQuery(uint low, uint high)
{
	int curr_high;
	//int iters = 1;
	bool result = pointQuery(low,&curr_high);
	while(!result && curr_high<high)
	{
		result = pointQuery(curr_high+1,&curr_high);
		//iters++;
	}
	//cout<<"range query dives: "<<iters<<endl;
	return result;
}

void SynopsisDFS::buildGraph(string & dotty)
{


}

void SynopsisDFS::exportGraphViz(string file)
{
	string dotty =" digraph g {node [shape = record,height=.1];\n label=\"Trie State\";\n";

	 dotty+="}";

	 buildGraph(dotty);

	  ofstream File(file);
	  File<<dotty;
	  File.close();
	  cout<<"written output to file "<<file<<endl;

}
bool SynopsisDFS::pointQuery(uint key,int * curr_high)
{
	int left = 0; //idx of the current left thing
	int leaf = 0; //leaf idx of the next leaf (either left or right)
	int low = lowerb;
	int high = lowerb+domain;
	while(true)
	{
		int middle = low + ((high-low)>>1);
		if(key<=middle)
		{
			if(isLeaf(left))
			{
				*curr_high = middle;
				return leafValue(leaf);
			}
			else //go to left child
			{
				leaf+=isLeaf(left+1);
				left = left + 2;
				high = middle;
			}
		}
		else //go right
		{
			if(isLeaf(left+1))
			{
				leaf+=isLeaf(left);
				*curr_high = high;
				return leafValue(leaf);

			}
			else //right child is not a leaf
			{
				if(isLeaf(left))
				{
					left+=2;
					leaf++;
				}
				else
				{

					left =skipSubtree(left+2,&leaf);
				}
				low = middle + 1;
			}


		}

	}

}
SynopsisDFS::~SynopsisDFS() {
	// TODO Auto-generated destructor stubh
}

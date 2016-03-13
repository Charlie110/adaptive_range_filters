/*
 * QuadViz.cpp
 *
 *  Created on: Dec 19, 2012
 *      Author: carolinux
 */

#include "QuadSynopsis.h"

#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>



void QuadSynopsis::takeTextSnapshot(string text)
{
	writeGraph(exportText(text));
}





string QuadSynopsis::getQueryText()
{

	stringstream ss;
	ss<<"Query: [";
	ss<<"|";
	for(int i=0;i<cq.low.size();i++)
	{
		ss<<" "<<cq.low[i];
		if(i<cq.low.size()-1)
			ss<<",";
	}
	ss<<"|";

	ss<<"|";

	for(int i=0;i<cq.low.size();i++)
		{
			ss<<" "<<cq.high[i];
			if(i<cq.low.size()-1)
				ss<<",";
		}

	ss<<"|";

	ss<<"]";

	ss<<" Curr leaf: "<<curr_leaf.lvl<<","<<curr_leaf.idx<<" ";
	ss<<"Size:"<<num_bits<<" ";

	return ss.str();

}


void QuadSynopsis::takeSnapshot(string text)
{
	stringstream ss;
	//ss<<"\n label=\"Trie "<<id<<"\";";
	ss<<"\n label=\"example QUAD ARF\";";
	string output=ss.str();

  // output+=exportText(text);
  // output+=exportText(getQueryText()+text);



    output+=firstNode();

    vector< vector<int> > low = getChildrenLowerBounds(lowp,highp);
    vector< vector<int> > high = getChildrenUpperBounds(lowp,highp);

    for(int i=0;i<low.size();i++)

    	buildGraph(0,i,low[i],high[i],output);

    writeGraph(output);

}



string QuadSynopsis::firstNode()
{

	stringstream ss;


	//make the node //
	string style="filled";
	string fillcolor="white";
	ss<<"id"<<id<<"nodefirst";
	ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",label = \"";
			ss<<"["<<getVector(lowp)<<","<<getVector(highp)<<"]";
			ss<<"\"];\n";
	// add children //

			for(int i=0;i<numChildren();i++)
			 ss<<"\"id"<<id<<"nodefirst\" -> \"id"<<id<<"node"<<0<<"idx"<<i<<"\";\n";
			 //ss<<"\"id"<<id<<"nodefirst\" -> \"id"<<id<<"node"<<0<<"idx"<<1<<"\";\n";


	return ss.str();

}

void QuadSynopsis::buildGraph(int lvl,int idx,vector<int> low, vector<int> high,string & graph)
{
	stringstream ss;
	int leaf_offset;
	ss<<"id"<<id<<"node"<<lvl<<"idx"<<idx;


	string fillcolor ="white";
	string style = "filled";
	bool isClock = false;
	bool isCurr = false;
	if(lvl == prev_victim.lvl && idx == prev_victim.idx)
	{
		//fillcolor="#B4CDCD"; // awlays keep track of the clock //
		isClock = true;
	}

	//isClock = false;

	if(lvl == curr_leaf.lvl && idx == curr_leaf.idx)
		{
			//fillcolor="#B4CDCD"; // awlays keep track of the clock //
			isCurr = true;
		}

	if(isLeaf(lvl,idx))
	{


		bool val = getLeaf(lvl,idx);

		if(val)
			fillcolor="green";
		else
			fillcolor="tomato";

		//if(isClock)
		//					fillcolor="#B4CDCD";
		if(isCurr)
			fillcolor="yellow";


		if(low[0]<0)
		{
			fillcolor = "gray";
			ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",color=black,label = \"";
					ss<<"[ UNUSED ] ";
					unused++;

		}
		else
		{
			int used_val = getUsed(lvl,getLeafOffset(lvl,idx));

			ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",color=black,label = \"";
			ss<<"[ "<<getVector(low)<<","<<getVector(high)<<"]";// used ="<<used_val;
			//if(val)
			//	ss<<" [HAZ]";
			//if(isClock)
			//					ss<<", CLOCK";
			if(low.size() ==1)
			{
				int len = high[0]-low[0]+1;
				ss<<", len = "<<len;
			}
		}


		ss<<"\"];\n";
		graph+= ss.str();

	}
	else //we need the kids too
	{

		vector< vector<int> > lowb= getChildrenLowerBounds(low,high);
		vector< vector<int> > highb = getChildrenUpperBounds(low,high);

		int first_child = getFirstChild(lvl,idx);
		//if(isClock)
		//			fillcolor="#B4CDCD";
		if(isCurr)
				fillcolor="yellow";
		ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",label = \"";
		ss<<"[ "<<getVector(low)<<","<<getVector(high)<<"]";
		//if(isClock)
			//		ss<<", CLOCK";
		ss<<"\"];\n";
		//connect it to its kids :))
		//"node1" -> "node2";

		for(int i=0;i<lowb.size();i++)
                ss<<"\"id"<<id<<"node"<<lvl<<"idx"<<idx<<"\" -> \"id"<<id<<"node"<<lvl+1<<"idx"<<first_child+i<<"\";\n";

		graph+= ss.str();

		for(int i=0;i<lowb.size();i++)

			buildGraph(lvl+1,first_child+i,lowb[i],highb[i],graph);



	}


}

string QuadSynopsis::getVector(vector<int> pt)
{
	stringstream ss;
	assert(pt.size()==dim);
	ss<<"|";
	for(int i=0;i<pt.size();i++)
	{
		if(i<pt.size()-1)
			ss<<pt[i]<<",";
		else
			ss<<pt[i]<<"|";
	}
	return ss.str();
}

void QuadSynopsis::writeGraph(string text)
{

	string format="png";
	snapshots++;
	stringstream tt;
	tt<<outfolder<<"/snapshot"<<setfill('0') << setw(5)<<snapshots<<".dot";
	stringstream ss;
	string time;
	ss<<outfolder<<"/snapshot"<<setfill('0') << setw(5)<<snapshots<<"."<<format;
	string file = ss.str();

	string output = "digraph g {node [shape = record,height=.1];\n";
	output+=text;
	output+="}";

	/* output as dotty file */
	ofstream File(tt.str());
	File<<output;
	File.close();

	/* create picture (if possible)*/
	stringstream command;
	//command<<"dot "<<temp<<" -Tpng >"<<file;
	command<<"dot "<<tt.str()<<" -T"<<format<<"  >"<<file;
	system(command.str().c_str()); //how nice, I love it !!
	cout<<"Snap! ["<<snapshots<<"]"<<endl;


}

/*
void QuadSynopsis::exportGraphViz(string file,string text)
{

  string dotty =" digraph g {node [shape = record,height=.1];\n label=\"Trie State\";\n";


  dotty+=exportText(text);
  vector<int> midpoint1,midpoint2;
    int d = getDimension(0,0);
    	getMidpoints(lowp,highp,getMiddle(lowp,highp,d),d,&midpoint1,&midpoint2);

 dotty+=firstNode();
 buildGraph(0,0,lowp,midpoint1,dotty);
 buildGraph(0,1,midpoint2,highp,dotty);

  dotty+="}";

  ofstream File(file);
  File<<dotty;
  File.close();
  cout<<"written output to file "<<file<<endl;

  //cout<<dotty;


}*/



string QuadSynopsis::exportText(string text)
{

	return "text[label=\""+text+"\"];\n";
}





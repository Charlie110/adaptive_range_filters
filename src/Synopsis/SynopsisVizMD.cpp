/*
 * SynopsisDebug.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: carolinux
 */


#include "SynopsisMD2.h"
#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>



void SynopsisMD2::takeTextSnapshot(string text)
{
	writeGraph(exportText(text));
}





string SynopsisMD2::getQueryText()
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

	ss<<" Curr leaf: "<<curr_leaf.lvl<<","<<curr_leaf.idx;

	return ss.str();

}


void SynopsisMD2::takeSnapshot(string text)
{
	stringstream ss;
	//ss<<"\n label=\"Trie "<<id<<"\";";
	ss<<"\n label=\"example ARF\";";
	string output=ss.str();

  // output+=exportText(text);
   output+=exportText(getQueryText()+text);


   vector<int> midpoint1,midpoint2;
   int d = getDimension(0,0);
   	getMidpoints(lowp,highp,getMiddle(lowp,highp,d),d,&midpoint1,&midpoint2);

    output+=firstNode();
    buildGraph(0,0,lowp,midpoint1,output);
    buildGraph(0,1,midpoint2,highp,output);
    writeGraph(output);

}



string SynopsisMD2::firstNode()
{

	stringstream ss;

	int low = 0;
	int high = this->domain;
	//make the node //
	string style="filled";
	string fillcolor="white";
	ss<<"id"<<id<<"nodefirst";
	ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",label = \"";
			ss<<"["<<getVector(lowp)<<","<<getVector(highp)<<"]";
			ss<<"\"];\n";
	// add children //

			 ss<<"\"id"<<id<<"nodefirst\" -> \"id"<<id<<"node"<<0<<"idx"<<0<<"\";\n";
			 ss<<"\"id"<<id<<"nodefirst\" -> \"id"<<id<<"node"<<0<<"idx"<<1<<"\";\n";


	return ss.str();

}

void SynopsisMD2::buildGraph(int lvl,int idx,vector<int> low, vector<int> high,string & graph)
{
	stringstream ss;
	int leaf_offset;
	ss<<"id"<<id<<"node"<<lvl<<"idx"<<idx;
	int d = getDimension(lvl,idx);

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
		//if(isCurr)
			//fillcolor="yellow";

		int used_val = getUsed(lvl,getLeafOffset(lvl,idx));

		ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",color=black,label = \"";
		ss<<"[d= "<<d<<getVector(low)<<","<<getVector(high)<<"] ";//used ="<<used_val;
		//if(isClock)
			//				ss<<", CLOCK";


		ss<<"\"];\n";
		graph+= ss.str();

	}
	else //we need the kids too
	{


		int left_child = getLeftChild(lvl,idx);
		//if(isClock)
		//			fillcolor="#B4CDCD";
		if(isCurr)
				fillcolor="yellow";
		ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",label = \"";
		ss<<"[d= "<<d<<getVector(low)<<","<<getVector(high)<<"]";
		if(isClock)
					ss<<", CLOCK";
		ss<<"\"];\n";
		//connect it to its kids :))
		//"node1" -> "node2";

                ss<<"\"id"<<id<<"node"<<lvl<<"idx"<<idx<<"\" -> \"id"<<id<<"node"<<lvl+1<<"idx"<<left_child<<"\";\n";
		ss<<"\"id"<<id<<"node"<<lvl<<"idx"<<idx<<"\" -> \"id"<<id<<"node"<<lvl+1<<"idx"<<left_child+1<<"\";\n";
		graph+= ss.str();

		vector<int> midpoint1,midpoint2;
		int dc = getDimension(lvl+1,left_child);
		getMidpoints(low,high,getMiddle(low,high,dc),dc,&midpoint1,&midpoint2);

		buildGraph(lvl+1,left_child,low,midpoint1,graph);
		buildGraph(lvl+1,left_child+1,midpoint2,high,graph);


	}


}

string SynopsisMD2::getVector(vector<int> pt)
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

void SynopsisMD2::writeGraph(string text)
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


void SynopsisMD2::exportGraphViz(string file,string text)
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


}



string SynopsisMD2::exportText(string text)
{

	return "text[label=\""+text+"\"];\n";
}




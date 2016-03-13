/*
 * SynopsisDebug.cpp
 *
 *  Created on: Oct 22, 2012
 *      Author: carolinux
 */


#include "../Synopsis.h"
#include <string>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>


int Synopsis::getScore(int low,int high,Database *d) /* the lower the better */
{
	assert(low>= lowerb && high<=actual_size + lowerb);
	int score = 0;
	for(int i=low;i<=high;i++)
	{

		bool res = pointQuery(i);

		bool real = d->rangeQuery(i,i);

		assert(!(!res && real));
		if(res && !real)
		{
			/* augment score */
			score++;
		}

	}

	return score;

}

void Synopsis::takeTextSnapshot(string text)
{
	writeGraph(exportText(text));
}


void Synopsis::takeSnapshot()
{
	takeSnapshot(vector<pair<int,int> >());
}


void Synopsis::takeSnapshotPartial(uint left,int lvls)
{
    string output="";
    vector<pair<int,int> > special_nodes = vector<pair<int,int> >();
    vector<Synopsis::state> hist;
    navigate(left,0,0,0,&hist,true);
    stringstream text;
    text<<"Size: "<<this->size();
    output+=exportText(text.str());
    struct state st= hist.back();
    hist.pop_back();
    //cout<<"start from:"<<st.idx<<""

    cout<<"lvl:"<<st.lvl<<endl;
    cout<<"actual idx:"<<st.actual_idx<<endl;
    //if(!st.wentLeft)
    //	st.actual_idx++;
    int low,high;
    getSingleRange(st.lvl,st.actual_idx,&low,&high);
    cout<<"range of thing: "<<low<<"-"<<high<<endl;
    int LEVELS = lvls;
    for(int i=1;i<LEVELS;i++)
    {
    	if(hist.size()==0)
    		break;
    	else
    		st = hist.back();
    		hist.pop_back();

    }

    //if(!st.wentLeft)
    //	st.actual_idx++;

    string fillcolor = "#AEEEEE";
    cout<<"Parent lvl:"<<st.lvl<<", idx:"<<st.idx<<endl;
    stringstream ss;
    ss<<"id"<<id<<"nodefirst";
    	ss<<"[style=dashed,fillcolor=\""<<fillcolor<<"\",label = \"";
    			ss<<"[rest of ARF]";
    			ss<<"\"];\n";
    	// add children //

 ss<<"\"id"<<id<<"nodefirst\" -> \"id"<<id<<"node"<<st.lvl<<"idx"<<st.idx<<"\";\n";


   output+=ss.str();


    buildGraph(st.lvl,st.idx,st.actual_idx,output,special_nodes,TRACETRUNCATE);

    writeGraph(output);



}

void Synopsis::takeSnapshot(vector<pair<int,int> > special_nodes,int mode)
{
	stringstream ss;
	//ss<<"\n label=\"Trie "<<id<<"\";";
	ss<<"\n label=\"example ARF\";";
	string output=ss.str();
    if(mode == TRACESPLIT || mode == TRACEMARKEMPTY)
    {
    	output+= exportQuery(current_query,database,true,false,"");
    }
    if(mode == TRACETRUEPOS)
    	output+= exportQuery(current_query,database,true,true,"");
    if(mode == TRACEMARKUSED)
       	output+= exportQuery(current_query,database,false,false,"");
    if(mode == TRACEMERGE)
    {
    	stringstream text;
    	text<<"Merging (size = "<<num_bits<<")";
    	output+=exportText(text.str());
    }

    if(mode == TRACEEVAL)
    {
       	stringstream text;
       	text<<"Trie Score: "<<getScore(0,actual_size,database);
       	output+=exportText(text.str());
    }

    output+=firstNode();
    buildGraph(0,0,0,output,special_nodes,mode);
    buildGraph(0,1,1,output,special_nodes,mode);
    writeGraph(output);

}

int Synopsis::getFps(int lvl,int leaf_idx)
{
	if(fps.empty())
		return 0;
	else
		return fps[lvl][leaf_idx];
}

string Synopsis::firstNode()
{

	stringstream ss;
	int low = 0;
	int high = this->domain;
	//make the node //
	string style="filled";
	string fillcolor="white";
	ss<<"id"<<id<<"nodefirst";
	ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",label = \"";
			ss<<"["<<lowerb+low<<","<<lowerb+high<<"]";
			ss<<"\"];\n";
	// add children //

			 ss<<"\"id"<<id<<"nodefirst\" -> \"id"<<id<<"node"<<0<<"idx"<<0<<"\";\n";
			 ss<<"\"id"<<id<<"nodefirst\" -> \"id"<<id<<"node"<<0<<"idx"<<1<<"\";\n";


	return ss.str();

}

void Synopsis::buildGraph(int lvl,int idx,int actual_idx,string & graph,vector<pair<int,int> > special_nodes,int mode)
{
	stringstream ss;
	int leaf_offset;
	ss<<"id"<<id<<"node"<<lvl<<"idx"<<idx;
	bool isSpecial = false;
	for(int i=0;i<special_nodes.size();i++)
	{
		if(lvl == special_nodes[i].first && idx == special_nodes[i].second)
		{
			isSpecial = true;
			break;
		}
	}
	string fillcolor ="white";
	string style = "filled";
	if(isSpecial)
	{
		if(mode == TRACESPLIT )
			fillcolor="yellow"; /*split or merge i guess */
		if(mode == TRACEMARKEMPTY || mode == TRACETRUEPOS)
			fillcolor="#CD5555"; /* marking as empty */
		if(mode == TRACEMERGE)
			fillcolor="#B4CDCD";
		if(mode == TRACEMARKUSED)
			fillcolor = "green";
	}


	if(isLeaf(lvl,idx))
	{
		/*if(lvl == prev_victim.lvl && idx == prev_victim.idx && fillcolor.compare("#CD5555")!=0
				&& fillcolor.compare("green")!=0)
		{
			fillcolor="#B4CDCD"; // awlays keep track of the clock //
		}*/
		bool val = getLeaf(lvl,idx,leaf_offset,true);
		int low,high;
		getSingleRange(lvl,actual_idx,&low,&high);

		assert(database!=NULL);
		bool real = database->rangeQuery(lowerb+low,lowerb+high);
		assert(!(!val && real));
		if(val && !real)
		{
			//style ="dashed"; //FIXME: If the thing is inconsistent.. //
			if(mode == TRACEEVAL)
				fillcolor="red";

		}

		if(fillcolor.compare("yellow") !=0)
		{
			if(val)
				fillcolor="green";
			else
				fillcolor="tomato";
		}

		ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",color=black,label = \"";
		ss<<"["<<lowerb+low<<","<<lowerb+high<<"]";

		/*if(val)
			ss<<" = 1";
		else
			ss<<" = 0";
		if(getUsed(lvl,leaf_offset))
			ss<<", used="<<getUsed(lvl,leaf_offset);
		if(lvl == prev_victim.lvl && idx == prev_victim.idx)
			ss<<" [CLOCK]";


		    ss<<" -- fps: "<<getFps(lvl,leaf_offset);*/

		ss<<"\"];\n";
		graph+= ss.str();

	}
	else //we need the kids too
	{

		int low,high;
		getSingleRange(lvl,actual_idx,&low,&high);
		int left_child = getLeftChild(lvl,idx);
		ss<<"[style="<<style<<",fillcolor=\""<<fillcolor<<"\",label = \"";
		ss<<"["<<lowerb+low<<","<<lowerb+high<<"]";
		ss<<"\"];\n";
		//connect it to its kids :))
		//"node1" -> "node2";

                ss<<"\"id"<<id<<"node"<<lvl<<"idx"<<idx<<"\" -> \"id"<<id<<"node"<<lvl+1<<"idx"<<left_child<<"\";\n";
		ss<<"\"id"<<id<<"node"<<lvl<<"idx"<<idx<<"\" -> \"id"<<id<<"node"<<lvl+1<<"idx"<<left_child+1<<"\";\n";
		graph+= ss.str();
		int next_actual = 2*actual_idx;

		buildGraph(lvl+1,left_child,next_actual,graph,special_nodes,mode);
		buildGraph(lvl+1,left_child+1,next_actual+1,graph,special_nodes,mode);


	}


}

void Synopsis::writeGraph(string text)
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

	ofstream File(tt.str());
	File<<output;
	File.close();

	/* create picture */
	stringstream command,foo;
	//command<<"dot "<<temp<<" -Tpng >"<<file;
	command<<"dot "<<tt.str()<<" -T"<<format<<"  >"<<file;
	system(command.str().c_str()); //how nice, I love it !!
	cout<<"Snap! ["<<snapshots<<"]"<<endl;
	//system("pwd");


}

void Synopsis::exportDatabase()
{
	takeTextSnapshot("----EMPTY RANGES IN COLD STORE---");
	Query::Query_t full_query;
	full_query.left =0;
	full_query.right = actual_size;
	string content = exportQuery(full_query,database,true,true,"");
	writeGraph(content);
}

string Synopsis::exportQuery(Query::Query_t q,Database * dbase,bool sR,bool qR, string file)
{

	string res="";
	stringstream ss;
	//ss<<"graph query {\n query[label=\"Query:";
	ss<<"query[label=\"Query:";
	if(qR && sR)
	{
		res =" true positive [empty ranges]";
		vector<Query::Query_t> empty = dbase->determineEmptyRanges(q,lowerb,actual_size);
		ss<<"["<<q.left+lowerb<<"-"<<q.right+lowerb<<"] is a "<<res<<"\",color = green];\n";
		for(int i=0;i<empty.size();i++)
		{
			/* make the empty ranges children of this graph thing */
			ss<<"query"<<i<<"[color=red, label=\"["<<lowerb+empty[i].left<<"-";
			ss<<lowerb+empty[i].right<<"]\"];\n";
			ss<<"\"query\"->\"query"<<i<<"\";\n";

		}
	}

	if(!qR && sR)
	{
		res = "false positive";
		ss<<"["<<q.left+lowerb<<"-"<<q.right+lowerb<<"] is a "<<res<<"\", color = red];\n";

	}
	if(!qR && !sR)
	{
		res = "true negative";
		ss<<"["<<q.left+lowerb<<"-"<<q.right+lowerb<<"] is a "<<res<<"\",color = blue];\n";
	}
	assert(!(qR && !sR));
   // ss<<"}";
	if(!file.empty())
	{
	 ofstream File(file);
	 File<<ss.str();
	 File.close();
	}
	 return ss.str();


}

void Synopsis::exportGraphViz(string file, vector<pair<int,int> > special_nodes)
{

  string dotty =" digraph g {node [shape = record,height=.1];\n label=\"Trie State\";\n";


  buildGraph(0,0,0,dotty,special_nodes);
  buildGraph(0,1,1,dotty,special_nodes);

  dotty+="}";

  ofstream File(file);
  File<<dotty;
  File.close();
  cout<<"written output to file "<<file<<endl;

  //cout<<dotty;


}



string Synopsis::exportText(string text)
{

	return "text[label=\""+text+"\"];\n";
}




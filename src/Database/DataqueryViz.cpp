/*
 * DataqueryViz.cpp
 *
 *  Created on: Dec 28, 2012
 *      Author: carolinux
 */

#include "DataqueryViz.h"

DataqueryViz::DataqueryViz(Database *db, Query* q, bool pq) {
	// TODO Auto-generated constructor stub
	this->db = db;
	this->qgen = q;
	this->pointq = pq;

}


void DataqueryViz::setColor(int x,int y,int r,int g,int b)
{
	graph(x,y)->Red = r;
	graph(x,y)->Green = g;
	graph(x,y)->Blue = b;
}

void DataqueryViz::drawLine(int x1,int y1,int x2, int y2,int r,int g,int b)
{
	/*cout<<x1<<","<<x2<<endl;
		cout<<y1<<","<<y2<<endl;*/
		assert(y2>=y1);

	if(x1 == x2)
	{


		int len = y2-y1;
		if(len<0)
			len = -len;

		for(int i=0;i<len;i++)
		{
			setColor(x1,y1+i,r,g,b);
		}
		return;

	}


	if(y1 == y2)
	{
		int len = x2-x1;
		if(len<0)
					len = -len;
		for(int i=0;i<len;i++)
		{
			setColor(x1+i,y1,r,g,b);
		}
		return;
	}



	//TODO: What about crooked lines? not needed right now
}


void DataqueryViz::drawRect(int x1,int y1, int x2, int y2, int r,int g,int b)
{
	drawLine(x1,y1,x1,y2,r,g,b);
	drawLine(x1,y1,x2,y1,r,g,b);

	drawLine(x1,y2,x2,y2,r,g,b);
	drawLine(x2,y1,x2,y2,r,g,b);
	//TODO: Write me: to visualize query boxes :)
}

void DataqueryViz::plot2D(string file)
{

	if(db!=NULL)
	{
		if(db->attr.size()!=2)
			return;
	}



	int sf = 4; /* scale factor */


	// plot data points //

	int maxX;
	int maxY;

	if(db!=NULL)
	{
		maxX = db->attr[0].higherb;
		maxY = db->attr[1].higherb;
	}
	else
	{
		maxX= domains[0];
		maxY =domains[1];
	}

	graph.SetSize((maxX +(sf*1))/sf,(maxY+(sf*1))/sf);




	if(qgen!=NULL)
	{
		//qgen->numQueries()/10
		for(int i=0;i<500;i++)
		{

			Query::QueryMD_t q = qgen->nextQueryMD();

			if(pointq)
			{
				q.low=q.high;
			}


			graph(q.low[0]/sf,(maxY -q.low[1])/sf)->Green = 0;
			graph(q.low[0]/sf, (maxY - q.low[1])/sf)->Red = 255;
			graph(q.low[0]/sf,(maxY -q.low[1])/sf)->Blue = 0;


			drawRect(q.low[0]/sf,(maxY -q.high[1])/sf,
					q.high[0]/sf,(maxY -q.low[1])/sf,
			255,0,0);
			//cout<<q.low[0]<<" "<<q.low[1]<<endl;

		}
	}


	if(db!=NULL)
	{
		for(int i=0;i<=db->curve->getDomain();i++)
		{
			vector<int> pt = db->curve->delinearizeCurveValue(i);

			assert(pt.size() == 2); // alliws ti plot na kanw??

			if(db->contains(i))
			{
				graph(pt[0]/sf,(maxY-pt[1])/sf)->Green = 0;
				graph(pt[0]/sf,(maxY-pt[1])/sf)->Red = 0;
				graph(pt[0]/sf,(maxY-pt[1])/sf)->Blue = 255;
			}

			//cout<<pt[0]<<" "<<pt[1]<<endl;

		}
	}

	graph.WriteToFile(file.c_str());

}

DataqueryViz::~DataqueryViz() {
	// TODO Auto-generated destructor stub
}

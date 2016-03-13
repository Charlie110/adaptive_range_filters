/*
 * DataqueryViz.h
 *
 *  Created on: Dec 28, 2012
 *      Author: carolinux
 */

#include "../Database.h"
#include "../Query.h"
#include <string>
#include "../EasyBmp/EasyBMP.h"


#ifndef DATAQUERYVIZ_H_
#define DATAQUERYVIZ_H_

class DataqueryViz {

	Database * db;
	Query * qgen;
	vector<int> domains;
	BMP graph;
	bool pointq;
public:
	DataqueryViz(Database * db, Query * q, bool p =false);
	DataqueryViz(Database *db)
	{
		this->db = db;
		qgen = NULL;
	}
	void drawRect(int x1,int y1, int x2, int y2,int r,int g,int b);
	void drawLine(int x1,int y1,int x2, int y2,int r,int g,int b);
	DataqueryViz(Query *q,vector<int> doms)
	{
		this->db = NULL;
		qgen = q;
		domains = doms;
	}
	virtual ~DataqueryViz();
	void plot2D(string file);
	void setColor(int x,int y,int r,int g,int b);

};

#endif /* DATAQUERYVIZ_H_ */

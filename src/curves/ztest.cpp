#include <iostream>
#include <fstream>
#include "Zcurve.h"
#include "Curve.h"
#include "DatagenMD.h"
#include "../Synopsis/SynopsisMD.h"
#include "../Query.h"
using namespace std;


vector<std::pair<int,int> > to_pair(vector<Curve::attribute> a)
{


	vector<pair<int,int> > res;
	for(int i=0;i<a.size();i++)
	{
		pair<int,int> t;
		t.first = a[i].lowerb;
		t.second = a[i].higherb;

		res.push_back(t);
	}
	return res;
}

int main(int argc, char* argv[])
{
int MAX = 255;
char curve_type =argv[1][0];
int dim = 2;


vector<Curve::attribute> attr(dim);
for(int i=0;i<dim;i++)
{
	attr[i].lowerb = 0;
	attr[i].higherb = MAX;
}


int DBSIZE = 100;
int RANGEFILTERSIZE = 300;
DatagenMD dg = DatagenMD();
string file ="curves/data/temp.txt";



dg.GenerateData(attr.size(),'e',DBSIZE,"curves/data/temp.txt");

Database db = Database(attr,curve_type,file,DBSIZE);

SynopsisMD synmd = SynopsisMD(attr,curve_type,RANGEFILTERSIZE,&db);

//we have the db
db.plot();
synmd.listValues();

int TRAININGQUERIES = 500;
int QUERIES = 2000;
Query tqueryGen =Query(to_pair(attr),TRAININGQUERIES,2,1,1.2,true,query_type);

for(int i=0;i<TRAININGQUERIES;i++)
{



	Query::QueryMD_t q = tqueryGen.nextQueryMD();
    synmd.handleQuery(q.low,q.high,true);
    cout<<"query:"<<i<<endl;
}

string curve0 = synmd.exportCurve();
ofstream myfile;
 myfile.open ("zcurve0.txt");
 myfile << curve0;
 myfile.close();

synmd.truncateClock(RANGEFILTERSIZE);
synmd.reset();

FILE * f = fopen("foo.txt","w");


Query queryGen =Query(to_pair(attr),QUERIES,2,1,1.2,true,query_type);
for(int i=0;i<QUERIES;i++)
{

		Query::QueryMD_t q = queryGen.nextQueryMD();




			cout<<"query:"<<i<<endl;
			fprintf(f,"%d %d \n",q.low[0],q.low[1]);


		    synmd.handleQuery(q.low,q.high,false);



}
//system("gnuplot plot 'foo.txt' using 1:2");
fclose(f);
cout<<"Query type:"<<query_type<<endl;
cout<<"False pos rate (z curve): "<<synmd.fp/(double)(synmd.tn+synmd.fp)<<endl;
//assert(1==0);



string curve = synmd.exportCurve();

 myfile.open ("zcurve.txt");
 myfile << curve;
 myfile.close();


 synmd.syn->takeSnapshot(vector<std::pair<int,int> >());

return 0;

}

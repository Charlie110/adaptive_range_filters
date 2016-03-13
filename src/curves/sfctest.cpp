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
bool ZIPFDATA = 1;
int DBDOMAIN = MAX;
double ZIPFDATAFACTOR = 1.2;

Database db = Database(attr,curve_type);
Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
 Uniform unif(DBDOMAIN+1);
 if(ZIPFDATA)
 	db.populate(DBSIZE, &zipf);
 else
	db.populate(DBSIZE, &unif);

SynopsisMD synmd = SynopsisMD(attr,curve_type,RANGEFILTERSIZE,&db);

//we have the db
db.plot();
synmd.listValues();
stringstream ss;
ss<<"zcurve"<<ZIPFDATA<<".txt";
synmd.exportCurveFile(ss.str(), true);


//return 0;

int TRAININGQUERIES = 50000;
int QUERIES = 2000;
int ZIPFQUERIES = 1;
Query tqueryGen =Query(db.curve->getDimensions(),db.curve->getDomainOfDim(),
		TRAININGQUERIES,30,10,
		1.2,ZIPFQUERIES);

for(int i=0;i<TRAININGQUERIES;i++)
{



	Query::QueryMD_t q = tqueryGen.nextQueryMD();
    synmd.handleQuery(q.low,q.high,true);


    if(i%100==0)
    	cout<<"query "<<i<<endl;

   /* cout<<"query:"<<i<<endl;
    cout<<"Low:"<<endl;
	for(int i=0;i<dim;i++)
		cout<<q.low[i]<<",";
	cout<<endl;

	cout<<"High:"<<endl;
		for(int i=0;i<dim;i++)
			cout<<q.high[i]<<",";
		cout<<endl;*/
}


synmd.truncateClock(RANGEFILTERSIZE);
synmd.reset();


return 0;

/*


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

return 0;*/

}

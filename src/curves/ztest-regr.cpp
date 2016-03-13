#include <iostream>
#include "Zcurve.h"
#include "DatagenMD.h"
#include "../Synopsis/SynopsisMD.h"
using namespace std;


int main(int argc, char* argv[])
{

	int x1 = atoi(argv[1]);
	int x2 = atoi(argv[2]);
	int y1 = atoi(argv[3]);
		int y2 = atoi(argv[4]);
		int MAX = atoi(argv[5]);


cout<<"Hello space filling curve"<<endl;

int dim = 2;

vector<Zcurve::attribute> attr(dim);
for(int i=0;i<dim;i++)
{
	attr[i].lowerb = 0;
	attr[i].higherb = MAX;
}


Zcurve z(attr);

for(int i=0;i<64;i++)
{
	vector<int> v =z.Zinv(i);
	assert(z.Z(v) == i);
}


cout<<"Next-Jump-Out"<<endl;
/*assert(z.nextJumpOut(14,11,50) == 15 ); //constellation1
assert(z.nextJumpOut(12,12,50) == 15 ); //constellation2
assert(z.nextJumpOut(11,9,38) == 12); // constellation 3
assert(z.nextJumpOut(59,27,63) == 63);

assert(z.nextJumpIn(12,11,50)==14);
assert(z.nextJumpIn(27,11,50) ==33);
assert(z.nextJumpIn(49,11,50) ==50);*/


cout<<"Validating range queries ..."<<endl;

vector<int> low(dim);
vector<int> high(dim);
low[0] = x1;
low[1] = x2;
high[0] = y1;
high[1]= y2;
//low[2] = 5;
//high[2] = 7;


/*int j = z.nextJumpIn(34,z.Z(low),z.Z(high));
z.printBinary(j);
z.printDims(z.Zinv(j));



return 0;*/

//cout<<"current:"<<z.Zinv(16)[0]<<","<<z.Zinv(16)[1]<<endl;
//assert(!z.isContained(z.Zinv(16),low,high));
//int j = z.nextJumpIn(16,z.Z(low),z.Z(high));
//assert(j==26);
//assert(z.isContained(z.Zinv(16),low,high));
//cout<<"Next jump in: "<<j<<endl;
//cout<<"Next jump in:"<<z.Zinv(j)[0]<<","<<z.Zinv(j)[1]<<endl;

//return 0;

 //constellation 4

cout<<"----RANGE QUERIES -----"<<endl;


vector<Query::Query_t> qs = z.linearizeRangeQuery(low,high);
z.validateRangeQuery(low,high,qs);


cout<<"Domain of Z curve:"<<z.getDomain()<<endl;
cout<<"dims?:"<<dim<<endl;

int DBSIZE;
int RANGEFILTERSIZE = 1000;
DatagenMD dg = DatagenMD();
char * file ="curves/data/temp.txt";

dg.GenerateData(dim,'e',DBSIZE,"curves/data/temp.txt");

Database db = Database(file,true);

//we have the db

SynopsisMD synmd = SynopsisMD(attr,'z',RANGEFILTERSIZE,&db);



return 0;

}

/*
 * Experiment6.cpp
 *
 *  Created on: Oct 30, 2012
 *      Author: carolinux
 */

#include <iostream>
using namespace std;
#include<string>
#include "Util.h"
#include "Database.h"
#include "Query.h"
#include "Synopsis.h"
#include "Bloom.h"
#include <assert.h>
#include <fstream> //for parsing config file
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "MultiTrie.h"
#include <sstream>
#include "Synopsis/FastSynopsis.h"


string plotScript(char*);
string getFilename();


bool ZIPFDATA;
bool ZIPFQUERIES ;
int JUSTPOINTQUERIES;
int RANGEFILTERSIZE = 8000;

int EXPQUERIES = 300000;
int TRAININGQUERIES = 500200;
double MEANT = 3000;
double STDDEVT = 1000;
double MEAN = 30;
double STDDEV = 10;
double ZIPFQUERIESFACTOR = 1.2;
double ZIPFDATAFACTOR = 1.2;
int P = 20;
char * FOLDER;
int TR = 5100;
int SEED = 62;



int main(int argc, char* argv[]) {

	P = atoi(argv[1]);
        uint64 keys = atoi(argv[2]);
	uint64 step = keys;
	RANGEFILTERSIZE = 8*keys;
	ZIPFDATA = atoi(argv[4]);
	ZIPFQUERIES = atoi(argv[5]);
	MEAN = atoi(argv[3]);
	cout<<"MEAN: "<<MEAN<<endl;
	STDDEV = MEAN/3.0;

	uint64 DBDOMAIN = (1<<P) -1;


	Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
	int DBSIZE;



	int power = P;

	for(DBSIZE=1*keys;DBSIZE<=10*step;DBSIZE+=step)
	{

		int factor = 1;
	  cout<<"Factor: "<<factor<<endl;
	  assert(closestPower2(DBDOMAIN) - 1 == DBDOMAIN );
	  cout<<"Domain: "<<DBDOMAIN<<endl;

	EXPQUERIES = 3*DBSIZE;
	TRAININGQUERIES = 100000*(DBSIZE/step);

	cout<<"TRAINIG QUERIES"<<TRAININGQUERIES<<endl;

	  cout<<"Dbsize: "<<DBSIZE<<endl;

	  Database db(DBDOMAIN+1);
	 // TRAININGQUERIES = TR * (DBSIZE/1000);


	  if(ZIPFDATA)
	  {

		  db.populate(DBSIZE, &zipf);
	  }

	  else
	  {
		  Uniform unif(DBDOMAIN+1);
		  db.populate(DBSIZE, &unif);
	  }

	  db.plot();


	FastSynopsis fsyn(0, DBDOMAIN, &db);
	fsyn.perfect(&db);

        FastSynopsis fsyn1(1, DBDOMAIN, &db);
        fsyn1.perfect(&db);

      FastSynopsis fsyn0(0, DBDOMAIN, &db);
        fsyn0.perfect(&db);
	   /* traaain */
	   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);




	     cout<<"1st training.."<<endl;

	     Query qTrain2(DBDOMAIN, 0,MEANT*factor,STDDEVT*factor,
	   		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES*factor/3,
	   		  ZIPFQUERIES,false, SEED);
	     for (int i=0; i<2*TRAININGQUERIES*factor/3; i++) {
	       Query::Query_t q = qTrain2.nextQuery();
	       bool qR = db.rangeQuery(q);
	      	     fsyn.handle_query(q.left, q.right, qR, true);
		  fsyn1.handle_query(q.left, q.right, qR, true);
		fsyn0.handle_query(q.left, q.right, qR, true);
	       if(i%1000 == 0)
	    	   cout<<i<<endl;
		

	     }

	     cout<<"second training"<<endl;
	     Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
	     	   		  ZIPFQUERIESFACTOR, TRAININGQUERIES*factor/3,
	     	   		  ZIPFQUERIES,false, SEED);
	     	     for (int i=0; i<TRAININGQUERIES*factor/3; i++) {
	     	       Query::Query_t q = qTrain1.nextQuery();
	     	       if(JUSTPOINTQUERIES)
	     	       	    	   q.right=q.left;
	     	       bool qR = db.rangeQuery(q);
	     	      	     fsyn.handle_query(q.left, q.right, qR, true);
		  fsyn1.handle_query(q.left, q.right, qR, true);
fsyn0.handle_query(q.left, q.right, qR, true);
if(MEAN ==1)
			assert(q.left==q.right);



	     	     }


		fsyn.reset_training_phase();
	fsyn.truncate(RANGEFILTERSIZE);
	fsyn.end_training_phase();
                     fsyn0.reset_training_phase();
        fsyn0.truncate(RANGEFILTERSIZE);
        fsyn0.end_training_phase();
                fsyn1.reset_training_phase();
        fsyn1.truncate(RANGEFILTERSIZE);
        fsyn1.end_training_phase();

   /* benchmark */
	     cout<<"--------------END OF TRAINING----------------------"<<endl;

	      /* Real experiment - Tries are frozen (no adaptation) */
	       Query qGen(DBDOMAIN,0,MEAN,STDDEV,
	    		   ZIPFQUERIESFACTOR, EXPQUERIES*factor, ZIPFQUERIES,false, SEED);

	      int fpBloom = 0;
	      int tp = 0;
	      int tn = 0;
	      int tnBloom = 0;
	      int tpBloom = 0;

	        for (int i=0; i<EXPQUERIES*factor; i++)
	       {

	    		Query::Query_t q = qGen.nextQuery();
			if(i%10000 ==0)
			printf("query %d out of %d \n", i, EXPQUERIES);

			bool dR = db.rangeQuery(q);
			bool qR = bl.handleQuery(q, dR);
			fsyn.handle_query(q.left, q.right, dR, false);
			fsyn1.handle_query(q.left, q.right, dR, true);
			bool res1 = fsyn0.handle_query(q.left, q.right, dR, true);
			if(i%5 ==0) {
			fsyn0.truncate(RANGEFILTERSIZE);
			fsyn1.truncate(RANGEFILTERSIZE);
			}


	
	        }
	      /* end of benchmark */


	/* write iteration to fiel */

/*
	        fprintf(results,"%d %f %f \n",
	       	    			DBSIZE,syn.fp*100.0/(syn.fp + syn.tn),
	       	    			fpBloom*100.0/(fpBloom +tnBloom));*/
	        fprintf(stderr, "%f %f %f %f %f \n",
	        	       	    			(RANGEFILTERSIZE+0.0)/DBSIZE,
	        	       	    			fsyn.stats.getFpr(),
	        	       	    			fsyn0.stats.getFpr(),
	        	       	    			fsyn1.stats.getFpr(),
	        	       	    			bl.stats.getFpr());



	} //end of all the iterations

	
	/* and nao, plot! */
     /*

	string pl = plotScript(OUTPUTFILE);
	FILE * t = fopen("temp_plot.txt","w");
	for(int i=0;i<pl.size();i++)
		fprintf(t,"%c",pl.at(i));
	fclose(t);


	system("gnuplot temp_plot.txt");

	stringstream ss;

	ss<<"cp "<<getFilename()<<" "<<FOLDER<<"/";
	system(ss.str().c_str());

	ss.clear();
	ss<<"cp "<<OUTPUTFILE<<" "<<FOLDER<<"/";
	system(ss.str().c_str());



*/



	return 0;


}



string plotScript(char* OUTPUTFILE)
{

	stringstream ss,title,fn;

	if(JUSTPOINTQUERIES)
	{
		title<<"Point Queries ";
		fn<<"point";
	}
	else
	{
		title<<"Range Queries (m ="<<MEAN<<") ";
		fn<<"range"<<MEAN;
	}
	if(ZIPFDATA)
	{
		title<<"Zipf Data /";
		fn<<"z";
	}
	else
	{
		title<<"Uniform Data /";
		fn<<"u";
	}
	if(ZIPFQUERIES)
	{
		title<<"Zipf Queries";
		fn<<"z";
	}
	else
	{
		title<<"Uniform Queries";
		fn<<"u";
	}

	fn<<".png";

	 ss<<" set yrange[-1:129] \n";
	 ss<<" set ytics (0,20,40,60,80,100)\n";
    ss<<"set title \""<<title.str()<<"\" \n";
	ss<<"set xlabel \"Number of distinct keys\"\n";
	ss<<"set ylabel \"False Positive Rate % \"\n";
	ss<<"set term png\n";
	ss<<"set output \""<<getFilename()<<"\"\n";

	ss<<"plot \""<<OUTPUTFILE<<"\"  using 1:2 title"
				" \"ARF non-adaptive\""
				" with linespoints pt 6 linecolor rgb \"blue\" linewidth 1,\\";
		ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:3 title \"ARF adaptive 0-bit\""
					" with linespoints pt 8 linecolor rgb \"#7D26CD\" linewidth 1,\\";
		ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:4 title \"ARF adaptive 1-bit\""
						" with linespoints pt 9 linecolor rgb \"purple\" linewidth 1,\\";


		ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:5 title \"Bloom\""
				" with linespoints pt 4 linecolor rgb \"#610000\" linewidth 1";



	return ss.str();

}


string getFilename()
{
	stringstream fn;

	fn<<"exp6-";

	if(JUSTPOINTQUERIES)
		{

			fn<<"point";
		}
		else
		{

			fn<<"range"<<MEAN;
		}
		if(ZIPFDATA)
		{

			fn<<"z";
		}
		else
		{

			fn<<"u";
		}
		if(ZIPFQUERIES)
		{

			fn<<"z";
		}
		else
		{

			fn<<"u";
		}

		fn<<".png";

	return fn.str();
}

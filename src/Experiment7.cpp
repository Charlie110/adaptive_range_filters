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

int EXPQUERIES = 3000000;
int TRAININGQUERIES = 10100;//12000;// 5100;
double MEANT = 3000;
double STDDEVT = 1000;
double MEAN = 30;
double STDDEV = 10;
double ZIPFQUERIESFACTOR = 1.2;
double ZIPFDATAFACTOR = 1.2;
int P = 20;
char * FOLDER;
int BITS = 8; //2?

//vary dbsize


int main(int argc, char* argv[]) {

    bool test=false;
	ZIPFDATA = false;
	ZIPFQUERIES = false;
	MEAN = 30;
	STDDEV = 10;
	int P = atoi(argv[1]);

	uint DBDOMAIN = (1<<P) -1;

	int DBSIZE;



	int power = P;
	int f = 10;
        uint64 keys = atoi(argv[2]);
	uint64 step = keys;

	//5000,10000 1000
	for(DBSIZE=keys;DBSIZE<=10*step;DBSIZE+=step)
	{

		RANGEFILTERSIZE = BITS * DBSIZE;
		int factor = 1;
	  cout<<"Factor: "<<factor<<endl;
	  assert(closestPower2(DBDOMAIN) - 1 == DBDOMAIN );
	  cout<<"Domain: "<<DBDOMAIN<<endl;

	  if(DBSIZE>1000)
		  TRAININGQUERIES = (DBSIZE+0.0)/10000 * 5100;
	 // int DBSIZE = 100 * 1<<(power - P);
	  cout<<"Dbsize: "<<DBSIZE<<endl;


	  Database db(DBDOMAIN+1);


		  Uniform unif(DBDOMAIN+1);
		  db.populate(DBSIZE, &unif);


	  cout<<"Database populated."<<endl;
	 // db.plot();



	      FastSynopsis fsyn(0, DBDOMAIN, &db);
        fsyn.perfect(&db);
      FastSynopsis fsyn2(0, DBDOMAIN, &db);
        fsyn2.perfect(&db);


	   /* traaain */
	   Bloom    bl2(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE/2);
	   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);
	   cout<<"fp rate of big bloom:"<<bl.fpRate()<<endl;



	   cout<<"Generatin queries"<<endl;

	    // cout<<"2nd training.."<<endl;

	     Query qTrain2(DBDOMAIN, 0,MEANT*factor,STDDEVT*factor,
	   		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES*factor/3,
	   		  ZIPFQUERIES,false);
	     for (int i=0; i<2*TRAININGQUERIES*factor/3; i++) {
	       Query::Query_t q = qTrain2.nextQuery();
	       bool qR = db.rangeQuery(q);
fsyn.handle_query(q.left, q.right, qR, true);
fsyn2.handle_query(q.left, q.right, qR, true);
	       

	     }

	     Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
	    	   		  ZIPFQUERIESFACTOR, TRAININGQUERIES*factor/3,
	    	   		  ZIPFQUERIES,false);
	    	     for (int i=0; i<TRAININGQUERIES*factor/3; i++) {
	    	       Query::Query_t q = qTrain1.nextQuery();

	    	       if(JUSTPOINTQUERIES)
	    	    	   q.right=q.left;
	    	       bool qR = db.rangeQuery(q);
	    	    
	    	       if(i%100 == 0)
	    	       	        cout<<"Training 2: "<<i<<" out of "<<TRAININGQUERIES*factor/3<<endl;
		fsyn.handle_query(q.left, q.right, qR, true);
		fsyn2.handle_query(q.left, q.right, qR, true);



	    	     }

	fsyn.reset_training_phase();
	fsyn.truncate(RANGEFILTERSIZE);
	fsyn.end_training_phase();
                     fsyn2.reset_training_phase();
        fsyn2.truncate(RANGEFILTERSIZE/2);
        fsyn2.end_training_phase();





   /* benchmark */
	     cout<<"--------------END OF TRAINING----------------------"<<endl;

	      /* Real experiment - Tries are frozen (no adaptation) */
	       Query qGen(DBDOMAIN,0,MEAN,STDDEV,
	    		   ZIPFQUERIESFACTOR, EXPQUERIES*factor, ZIPFQUERIES,false);

	      int fpBloom1 = 0, fpBloom2 = 0;
	      int tp = 0;
	      int tn = 0;
	      int tnBloom1 = 0, tnBloom2 = 0,tpBloom2=0;


	        for (int i=0; i<EXPQUERIES*factor; i++)
	       {

	    		Query::Query_t q = qGen.nextQuery();

	    		if(i%10000 == 0)
	    			        cout<<"Benchmark: "<<i<<" out of "<<EXPQUERIES<<endl;

			bool dR = db.rangeQuery(q);
			 bl.handleQuery(q, dR);
			bl2.handleQuery(q, dR);
			fsyn.handle_query(q.left, q.right, dR, false);
			fsyn2.handle_query(q.left, q.right, dR, false);





	        }
	      /* end of benchmark */


	/* write iteration to fiel */
	        fprintf(stderr,"%d %f %f %f %f \n",
	       	    			DBSIZE, fsyn.stats.getFpr(), fsyn2.stats.getFpr(), bl.stats.getFpr(), bl2.stats.getFpr());
	       	    			

	       /* fprintf(results,"%d %f %f %f %f \n",
	        	       	    			DBSIZE,syn1.fp*100.0/(EXPQUERIES),
	        	       	    			syn2.fp*100.0/(EXPQUERIES),
	        	       	    			fpBloom1*100.0/(EXPQUERIES),
	        	       	    			fpBloom2*100.0/(EXPQUERIES));
			*/



	} //end of all the iterations

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
			" \"ARF non-adaptive ("<<BITS/2<<" bits/key)\""
			" with linespoints pt 7 ps 1.3  linecolor rgb \"#8FD8D8\" linewidth 1,\\";

	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:3 title"
				" \"ARF non-adaptive ("<<BITS<<" bits/key)\""
				" with linespoints pt 6 ps 1.3  linecolor rgb \"blue\" linewidth 1,\\";

	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:8 title"
				" \"Bloom ("<<BITS/2<<" bits/key)\""
				" with linespoints pt 5 ps 1.3 linecolor rgb \"#CD5555\" linewidth 1,\\";

	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:9 title \"Bloom ( "<<BITS<<" bits/key)\""
			" with linespoints pt 4 ps 1.3  linecolor rgb \"#610000\" linewidth 1";




	return ss.str();

}


string getFilename()
{
	stringstream fn;

	fn<<"exp7-";


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

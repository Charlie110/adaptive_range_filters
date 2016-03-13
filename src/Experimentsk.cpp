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

string plotScript(char*);
string getFilename();


bool ZIPFDATA;
bool ZIPFQUERIES ;
int JUSTPOINTQUERIES;
int RANGEFILTERSIZE = 4000;

int EXPQUERIES = 20000;
int TRAININGQUERIES = 10200;
double MEANT = 3000;
double STDDEVT = 1000;
double MEAN = 30;
double STDDEV = 10;
double ZIPFQUERIESFACTOR = 1.00;
double ZIPFDATAFACTOR = 1.2;
int P = 20;
char * FOLDER;
int TR = 5100;


int main(int argc, char* argv[]) {

	 if(!strcmp(argv[1],"replot"))
		{
	    	string s =plotScript("FILE.TXT");
	    	cout<<s;
	    	return 0;
		}



	ZIPFDATA = false;
	ZIPFQUERIES = true;
	JUSTPOINTQUERIES = atoi(argv[1]);

	if(JUSTPOINTQUERIES!=1)
		{
			MEAN = atoi(argv[1]);
			cout<<"MEAN: "<<MEAN<<endl;
			STDDEV = MEAN/3.0;
			JUSTPOINTQUERIES=0;
		}
		else
		{
			cout<<"POINT QUEREIS!"<<endl;
		}


	uint DBDOMAIN = (1<<P) -1;
	char * OUTPUTFILE= argv[2]; //"results.txt";
	//char * FOLDER= argv[3];
	FILE * results = fopen(OUTPUTFILE,"w");
	fclose(results);
	results = fopen(OUTPUTFILE,"a");
	 fprintf(results,"QuerySkew\t TRIE\t TRIE0\t TRIE1\t BLOOM\t\n");

	//Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);
	int DBSIZE = 1000;



	int power = P;

	for(ZIPFQUERIESFACTOR = 0;ZIPFQUERIESFACTOR<=2.09; ZIPFQUERIESFACTOR+=0.05)
	{

		int factor = 1;
	  cout<<"Factor: "<<ZIPFQUERIESFACTOR<<endl;
	  cout<<"Point q?"<<JUSTPOINTQUERIES<<endl;
	  assert(closestPower2(DBDOMAIN) - 1 == DBDOMAIN );
	  cout<<"Domain: "<<DBDOMAIN<<endl;


	  cout<<"Dbsize: "<<DBSIZE<<endl;

	  Database db(DBDOMAIN+1);
	 // TRAININGQUERIES = TR * (DBSIZE/1000);



		  Uniform unif(DBDOMAIN+1);
		  //Zipf zipf(DBDOMAIN+1, 0.01);
		  db.populate(DBSIZE, &unif);


	  db.plot();

	 // return 0;


	  SynopsisIntClock synStart(DBDOMAIN,RANGEFILTERSIZE,true,&db);
	   synStart.init();
	   synStart.perfect(&db);

	   /* traaain */
	   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);




	     cout<<"1st training.."<<endl;

	     Query qTrain2(DBDOMAIN, 0,MEANT*factor,STDDEVT*factor,
	   		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES*factor/3,
	   		  ZIPFQUERIES,false);
	     for (int i=0; i<2*TRAININGQUERIES*factor/3; i++) {
	       Query::Query_t q = qTrain2.nextQuery();
	       bool qR = db.rangeQuery(q);
	       synStart.handleQuery(q,&db,true,qR);
	       if(i%1000 == 0)
	    	   cout<<i<<endl;

	     }

	     cout<<"second training"<<endl;
	     Query qTrain1(DBDOMAIN, 0,MEAN,STDDEV,
	     	   		  ZIPFQUERIESFACTOR, TRAININGQUERIES*factor/3,
	     	   		  ZIPFQUERIES,false);
	     	     for (int i=0; i<TRAININGQUERIES*factor/3; i++) {
	     	       Query::Query_t q = qTrain1.nextQuery();
	     	       if(JUSTPOINTQUERIES)
	     	       	    	   q.right=q.left;
	     	       bool qR = db.rangeQuery(q);
	     	       synStart.handleQuery(q,&db,true,qR);



	     	     }


	     cout<<"end"<<endl;

	     SynopsisIntClock synStart1(synStart);
	     synStart.set(0);
	     synStart.truncateClock(RANGEFILTERSIZE);
	     synStart1.truncateClock(RANGEFILTERSIZE);

	     synStart.resetTime();
	     synStart1.resetTime();

	     synStart1.convertToCompact(1);
	     synStart.convertToCompact(0);

	     Synopsis syn(synStart);
	     syn.set(0);
	     Synopsis syn0(syn);
	     Synopsis syn1(synStart1);

   /* benchmark */
	     cout<<"--------------END OF TRAINING----------------------"<<endl;

	      /* Real experiment - Tries are frozen (no adaptation) */
	       Query qGen(DBDOMAIN,0,MEAN,STDDEV,
	    		   ZIPFQUERIESFACTOR, EXPQUERIES*factor, ZIPFQUERIES,false);

	      int fpBloom = 0;
	      int tp = 0;
	      int tn = 0;
	      int tnBloom = 0;
	      int tpBloom = 0;

	        for (int i=0; i<EXPQUERIES*factor; i++)
	       {

	    		Query::Query_t q = qGen.nextQuery();

	    		if(JUSTPOINTQUERIES)
	    			q.right=q.left;

	    		bool qR = db.rangeQuery(q);

	    		if(qR)
	    		  tp++;
	    		else
	    		  tn++;

				syn0.handleQuery(q,&db,true,qR);
				syn1.handleQuery(q,&db,true,qR);
				syn.handleQuery(q,&db,false,qR);
				syn0.truncateClock(RANGEFILTERSIZE);
				syn1.truncateClock(RANGEFILTERSIZE);


	    		bool sR;
	    		if(JUSTPOINTQUERIES)
	    			sR = bl.pointQuery(q.left);
	    		else
	    			sR = bl.rangeQuery(q);

	    		if(sR && !qR)
	    			 fpBloom++;
	    		if(!sR && !qR)
	    			tnBloom++;
	        }
	      /* end of benchmark */


	/* write iteration to fiel */

/*
	        fprintf(results,"%d %f %f \n",
	       	    			DBSIZE,syn.fp*100.0/(syn.fp + syn.tn),
	       	    			fpBloom*100.0/(fpBloom +tnBloom));*/
	        fprintf(results,"%f %f %f %f %f \n",
	        	       	    			ZIPFQUERIESFACTOR,
	        	       	    			syn.fp*100.0/(syn.fp + syn.tn),
	        	       	    			syn0.fp*100.0/(syn0.fp + syn0.tn),
	        	       	    			syn1.fp*100.0/(syn1.fp + syn1.tn),
	        	       	    			fpBloom*100.0/(fpBloom +tnBloom));

	       /* printf("%d %f %f \n",
	    	       	    			DBSIZE,syn.fp*100.0/(syn.fp + syn.tn),
	    	       	    			fpBloom*100.0/(fpBloom +tnBloom));
*/

	        fflush(results);


	} //end of all the iterations

	fclose(results);

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

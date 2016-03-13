/*
 * Experiment8.cpp
 *
 *  Created on: Nov 1, 2012
 *      Author: carolinux
 */

/*
 * Experiment6.cpp
 *
 *  Created on: Oct 30, 2012
 *      Author: carolinux
 */

#include <iostream>
using namespace std;
#include <string>
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
int RANGEFILTERSIZE = 8000;

int EXPQUERIES = 20000;
int TRAININGQUERIES = 5100;//12000;// 5100;
double MEANT = 3000;
double STDDEVT = 1000;
double MEAN = 30;
double STDDEV = 10;
double ZIPFQUERIESFACTOR = 1.2;
double ZIPFDATAFACTOR = 1.2;
int P = 20;
char * FOLDER;
int BITS = 4; //2?



int main(int argc, char* argv[]) {

    bool test=false;
	ZIPFDATA = 0;
	ZIPFQUERIES = 0;
    int factor = 1;
    char * OUTPUTFILE ="FILE.TXT";

	 if(!strcmp(argv[1],"replot"))
		{
	    	string s =plotScript("FILE.TXT");
	    	cout<<s;
	    	return 0;
		}


	uint DBDOMAIN = (1<<P) -1;

	OUTPUTFILE= argv[1];
	char * FOLDER= argv[2];

	FILE * results = fopen(OUTPUTFILE,"w");
	fclose(results);
	results = fopen(OUTPUTFILE,"a");
	fprintf(results,"INTRVL\t ARF\t ARF0\t ARF1\t BLOOM\n");
	cout<<"DBDOMAIN:"<<DBDOMAIN<<endl;

	//Zipf zipf(DBDOMAIN+1,ZIPFDATAFACTOR);

	int DBSIZE = 1000;
	RANGEFILTERSIZE = atoi(argv[3]);
	MEAN = 30;
	STDDEV = 0;
    int step = 5;

	//5000,10000 1000
	for(MEAN=0;MEAN<=300;MEAN+=step)
	{

		if(MEAN ==0)
			JUSTPOINTQUERIES=1;
		else
			JUSTPOINTQUERIES=0;

		if(MEAN == 60)
			step =10;

		STDDEV = MEAN/3.0;
		Database db(DBDOMAIN+1);




	 /* if(ZIPFDATA)
	  {

		  db.populate(DBSIZE, &zipf);
	  }

	  else*/
	  {
		  Uniform unif(DBDOMAIN+1);
		  db.populate(DBSIZE, &unif);
	  }


	  SynopsisIntClock synStart(DBDOMAIN,RANGEFILTERSIZE,true,&db);
	   synStart.init();
	   synStart.setDatabase(&db);
	   synStart.perfect(&db);


	   /* traaain */
	   Bloom    bl(DBDOMAIN, DBSIZE, &db, RANGEFILTERSIZE);

	   cout<<"fp rate of bloom:"<<bl.fpRate()<<endl;


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

	    // cout<<"2nd training.."<<endl;

	     Query qTrain2(DBDOMAIN, 0,MEANT*factor,STDDEVT*factor,
	   		  ZIPFQUERIESFACTOR, 2*TRAININGQUERIES*factor/3,
	   		  ZIPFQUERIES,false);
	     for (int i=0; i<2*TRAININGQUERIES*factor/3; i++) {
	       Query::Query_t q = qTrain2.nextQuery();
	       bool qR = db.rangeQuery(q);
	       synStart.handleQuery(q,&db,true,qR);
	       //cout<<i<<" out of "<<2*TRAININGQUERIES*factor/3<<endl;

	     }


	     SynopsisIntClock synStart1(synStart);
	     synStart.set(0);
	     synStart.truncateClock(RANGEFILTERSIZE);
	     synStart1.truncateClock(RANGEFILTERSIZE);


	     synStart.resetTime();
	     synStart.convertToCompact(0);
	     synStart1.resetTime();
	     synStart1.convertToCompact(1);


	     Synopsis syn(synStart);
	       syn.set(0);


	       syn.resetTime();
	       syn.setDatabase(&db);

	   Synopsis syn0(synStart);
	   syn0.set(0);


	   syn0.resetTime();
	   syn0.setDatabase(&db);

	   Synopsis syn1(synStart1);
	   syn1.set(1);


	   syn1.resetTime();
	  syn1.setDatabase(&db);


	       assert(syn.fp==0);


   /* benchmark */
	     cout<<"--------------END OF TRAINING----------------------"<<endl;

	      /* Real experiment - Tries are frozen (no adaptation) */
	       Query qGen(DBDOMAIN,0,MEAN,STDDEV,
	    		   ZIPFQUERIESFACTOR, EXPQUERIES*factor, ZIPFQUERIES,false);

	      int fpBloom = 0;
	      int tp = 0;
	      int tn = 0;
	      int tnBloom = 0, tpBloom=0;


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

	    		syn.handleQuery(q,&db,false,qR);
	    		syn0.handleQuery(q,&db,true,qR);
	    		syn1.handleQuery(q,&db,true,qR);
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
	    		if(sR && qR)
	    			tpBloom++;




	        }
	      /* end of benchmark */

	/* write iteration to fiel */
	        int m = MEAN;
	        if(JUSTPOINTQUERIES)
	        	m=1;
	        fprintf(results,"%d %f %f %f %f \n",
	       	    			m,syn.fp*100.0/(syn.fp + syn.tn),
	       	    			syn0.fp*100.0/(syn0.fp + syn0.tn),
	       	    			syn1.fp*100.0/(syn1.fp + syn1.tn),
	       	    			fpBloom*100.0/(fpBloom +tnBloom));

	        printf("%d %f %f \n",
	        	       	    			m,syn.fp*100.0/(syn.fp + syn.tn),
	        	       	    			fpBloom*100.0/(fpBloom +tnBloom));


	        cout<<"True positives: "<<tp<<"/ "<<EXPQUERIES<<endl;
	        cout<<"True negatives: "<<tn<<"/ "<<EXPQUERIES<<endl;
	        cout<<"fp syn: "<<syn.fp<<endl;
	        cout<<"fp bloom: "<<fpBloom<<endl;

	        fflush(results);


	} //end of all the iterations

	fclose(results);



	/* and nao, plot! */

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







	return 0;


}



string plotScript(char* OUTPUTFILE)
{

	stringstream ss,title,fn;

	if(JUSTPOINTQUERIES)
	{
		title<<"";
		fn<<"point";
	}
	else
	{
		title<<"";
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

    ss<<" set yrange[0:129] \n";
    ss<<" set xrange[1:300] \n";
    ss<<" set xtics(1,50,100,150,200,250,300)\n";
    ss<<" set ytics (0,20,40,60,80,100)\n";
    ss<<"set title \""<<title.str()<<"\" \n";
	ss<<"set xlabel \"Mean interval size\"\n";
	ss<<"set ylabel \"False Positive Rate % \"\n";
	ss<<"set term png\n";
	ss<<"set output \""<<getFilename()<<"\"\n";
	ss<<"plot \""<<OUTPUTFILE<<"\"  using 1:2 title"
			" \"ARF non-adaptive\""
			" with linespoints pt 6 ps 1.3 linecolor rgb \"blue\" linewidth 1,\\";
	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:3 title"
				" \"ARF adaptive 0-bit\""
				" with linespoints pt 8 ps 1.3 linecolor rgb \"#7D26CD\" linewidth 1,\\";
	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:4 title"
				" \"ARF adaptive 1-bit\""
				" with linespoints pt 9 ps 1.3 linecolor rgb \"purple\" linewidth 1,\\";

	ss<<"\n \""<<OUTPUTFILE<<"\"  using 1:5 title \"Bloom\""
			" with linespoints pt 4 ps 1.3 linecolor rgb \"#610000\" linewidth 1";




	return ss.str();

}


string getFilename()
{
	stringstream fn;

	fn<<"exp8-";


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

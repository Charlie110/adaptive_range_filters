/* ---------------------------------------------------------------------------
 *                               CLASS IMPLEMENTATION
 *
 *   CLASS NAME:        Query
 *
 *
 * ---------------------------------------------------------------------------
 */

#include "../Query.h"
#include <assert.h>




double Query::getAverageArea()
{
	int sum = 0;



	for(int i=0;i<n;i++)
	{
		QueryMD_t q = logMD[i];

		if(q.high[0] == q.low[0] && q.high[1] == q.low[1])
			sum+=1;
		else
		{

			int s = 1;
			for(int j=0;j<logMD[0].low.size();j++)
				s*=(logMD[i].high[j] +1 - logMD[i].low[j]);

			sum+= s;
		}


	}


 return (sum+0.0)/n;

}

double Query::getAverageLength()
{
	int sum = 0;
	for(int i=0;i<n;i++)
	{
			Query_t q = log[i];
			sum+= 1 + q.right - q.left;

	}

	return (sum+0.0)/n;
}

Query::Query(uint inDomain,int numQ)
{
	 domain = inDomain;
	  n = numQ;
	  log = (Query_t*) malloc(n * sizeof(Query_t));
	  logPos = 0;

}

void Query::generateHotZipf(double zipf,double mean,double stddev)
{

	  Zipf zipfLeft(domain+1, zipf);
	  Normal norm(domain+1,mean,stddev);

	  for(long i=0;i<n;i++)
	  {
		  log[i].left = zipfLeft.probe();
		  int size = norm.probe();
		  if(size<=0)
			  size=1;

		if (log[i].left + size > domain + 1)
		{

		  log[i].right = domain;

		}
		else
		  log[i].right = log[i].left + size - 1;
	  }
	  return;
}

void Query::generateUniform(double mean, double stddev)
{

	  Uniform uniformLeft(domain+1);
	  Normal norm(domain+1,mean,stddev);

	  for(long i=0;i<n;i++)
	  {
		  log[i].left = uniformLeft.probe();
		  int size = norm.probe();
		  if(size<=0)
			  size=1;

		if (log[i].left + size > domain + 1)
		{

		  log[i].right = domain;

		}
		else
		  log[i].right = log[i].left + size - 1;
	  }
	  return;
}

Query::Query(int dim,vector <Distribution *> distr,
		int noQueriesInWorkload,
		double mean,double stddev,bool skew)
{
	//this creates independently distributed queries
	// dimensions can have different domains
	cout<<"QUERY GEN [ potentially different domains ]"<<endl;
	  n = noQueriesInWorkload;
	  logMD.reserve(n);
	  logPos = 0;


	  Normal norm(-1,mean,stddev);

	  for (long i=0; i<n; i++)
	  {
		  vector<int> high(dim);
		  vector<int> low(dim);

		  for(int d=0;d<dim;d++)
		  {
			  int dom = distr[d]->getDomain() - 1;

			  uint size = norm.probe();

			  if(size<=0)
				size = 1;
			  if (skew)
				low[d] = dom - distr[d]->probe();
			  else
				low[d] = distr[d]->probe();


			  if (low[d] + size > dom + 1)
			  {

				high[d] = dom;
				low[d] = dom - size + 1;
			  }
			  else
				high[d] = low[d] + size - 1;
			  if(low[d]<0)
				  low[d] = 0;

		  }
	      QueryMD_t q;
	      q.low = low;
	      q.high = high;
	      logMD.push_back(q);


	    }


}

Query::QueryMD_t convertToMD(Query::Query_t q)
{
	Query::QueryMD_t qm;
		qm.low = vector<int>(1);
		qm.high = vector<int>(1);
		qm.low[0] = q.left;
		qm.high[0] = q.right;
		return qm;
}


Query::Query(int dim, int domainPerDim,
		int noQueriesInWorkload,
		double mean,double stddev,
		double inZipf, bool skew)
{
	//this creates independently distributed queries
	// requirement: dimensions have the same domain
	cout<<"QUERY GEN!!"<<endl;
	  n = noQueriesInWorkload;
	  logMD.reserve(n);
	  logPos = 0;
	 // int dim = c->getDimensions();
	  //int domainPerDim = c->getDomainOfDim();

	  Zipf zipfLeft(domainPerDim+1, inZipf);
	  Uniform uniformLeft(domainPerDim+1);
	  Normal norm(domainPerDim+1,mean,stddev);

	  for (long i=0; i<n; i++)
	  {
		  vector<int> high(dim);
		  vector<int> low(dim);

		  for(int d=0;d<dim;d++)
		  {

			  uint size = norm.probe();

			  if(size<=0)
				size = 1;
			  if (skew)
				low[d] = domainPerDim - zipfLeft.probe();
			  else
				low[d] = uniformLeft.probe();


			  if (low[d] + size > domainPerDim + 1)
			  {
				// log[i].right = domain - 1;
				high[d] = domainPerDim;
				low[d] = domainPerDim - size + 1;
			  }
			  else
				high[d] = low[d] + size - 1;

		  }
	      QueryMD_t q;
	      q.low = low;
	      q.high = high;
	      logMD.push_back(q);
	     // cout<<"crated query "<<i<<endl;

	    }


}


Query::Query(vector<int > domains,
		  int noQueriesInWorkload,double mean,double stddev,char correlation)
{

	DatagenMD datagen = DatagenMD();
	int dims = domains.size();

	  vector< vector<double> > coeff;

	  cout<<"!!!!!"<<endl;


	  if(correlation =='a')
	  coeff = datagen.GenerateDataAnticorrelated(NULL,noQueriesInWorkload,dims);
	  if(correlation =='c')
		  coeff = datagen.GenerateDataCorrelated(NULL,noQueriesInWorkload,dims);
	  if(correlation =='e')
		  coeff = datagen.GenerateDataEqually(NULL,noQueriesInWorkload,dims);
	  Normal norm(domains[0]+1,mean,stddev);

	  n = noQueriesInWorkload;
	  	  logMD.reserve(n);
	  	  logPos = 0;


	  	  for (long i=0; i<n; i++)
	  	  {
	  		  vector<int> high(dims);
	  		  vector<int> low(dims);

	  		  for(int d=0;d<dims;d++)
	  		  {

	  			  uint size = norm.probe();

	  			  if(size<=0)
	  				size = 1;

	  				low[d] = coeff[i][d]* domains[d];


	  			  if (low[d] + size > domains[d] + 1)
	  			  {
	  				// log[i].right = domain - 1;
	  				high[d] = domains[d];
	  				low[d] = domains[d] - size + 1;
	  			  }
	  			  else
	  				high[d] = low[d] + size - 1;
	  			  if(low[d]<0)
	  				  low[d] = 0;

	  		  }
	  	      QueryMD_t q;
	  	      q.low = low;
	  	      q.high = high;
	  	      logMD.push_back(q);
	  	     // cout<<"crated query "<<i<<endl;

	  	    }

	  	  assert(logMD.size() ==n);


}

//cor/anti corr
Query::Query(vector<std::pair<int,int> > attr,int noQueriesInWorkload,
		double mean,double stddev,double inZipf, bool skew,char correlation)
{

	//i want the queries to be with a given correlation and starting at a given point (clusteredness)
	  n = noQueriesInWorkload;
	  logMD.reserve(n);
	  logPos = 0;
	  int dim = attr.size();
	  vector<int> domains(dim);


	  DatagenMD datagen = DatagenMD();
	  for(int i=0;i<dim;i++)
	  {
		  domains[i] = attr[i].second - attr[i].first ;
	  }

	  Zipf zipfLeft(domains[0]+1, inZipf);
	  Uniform uniformLeft(domains[0]+1);
	  Normal norm(domains[0]+1,mean,stddev);


	  vector< vector<double> > coeff;


	  if(correlation =='a')
	  coeff = datagen.GenerateDataAnticorrelated(NULL,noQueriesInWorkload,dim);
	  if(correlation =='c')
		  coeff = datagen.GenerateDataCorrelated(NULL,noQueriesInWorkload,dim);
	  if(correlation =='e')
		  coeff = datagen.GenerateDataEqually(NULL,noQueriesInWorkload,dim);





	  double coeffl;
	  double coeffh;
	  // generate ALL the queries
	for (long i=0; i<n; i++) {


		vector<int> high(dim);
		vector<int> low(dim);
		cout<<"coefficients ("<<correlation<<")"<<endl;
		for(int k=0;k<dim;k++)
		{
			cout<<coeff[i][k]<<" ";
		}
		cout<<endl;

		for(int j=0;j<dim;j++)
		{
	    uint size;

	    	size = norm.probe(); //yes :)

	    if(size<=0)
	    	size = 1;

	    if(j == 0)
	    {
			if (skew)
			 low[j] = domains[j] - zipfLeft.probe();
			else
			  low[j] = uniformLeft.probe();



	    }

	    else
	    {
	    	double coeffc = (coeffl * coeff[i][j])/coeff[i][0];

	    	//cout<<"Coeff:" <<coeffc<<endl;
	    	low[j] = ((double)coeff[i][j]/(double)coeff[i][0]) * low[0];
	    }


	    {
			if (low[j] + size > domains[j] + 1) {
			  // log[i].right = domain - 1;
			  high[j] = domains[j];
			  low[j] = domains[j] - size + 1;
			}
			else
			  high[j] = low[j] + size - 1;
	    }

	    if(j==0)
	    {
	    	coeffl = (double)low[j]/(double)domains[j];
	    	//cout<<"Coefl:" <<coeffl<<endl;
	    }



		}

		//add this query to the vector
		QueryMD_t q;
		q.low = low;
		q.high = high;
		logMD.push_back(q);

		cout<<"Low:"<<endl;
				for(int i=0;i<dim;i++)
					cout<<q.low[i]<<",";
				cout<<endl;

				cout<<"High:"<<endl;
					for(int i=0;i<dim;i++)
						cout<<q.high[i]<<",";
					cout<<endl;


	}

}



Query::Query(uint inDomain, uint inMin,double mean,double stddev, double inZipf, long noQueriesInWorkload, bool skew,bool old) {
  domain = inDomain;
  n = noQueriesInWorkload;
  log = (Query_t*) malloc(n * sizeof(Query_t));
  logPos = 0;
  //int MEAN = mean;


  Zipf zipfInterval(2, inZipf);
  // for point in domain use either Zipf or Uniform; depending on value of skew
 // Zipf zipfLeft(domain+1, inZipf);
  //FIXME: there!

  Zipf * zipfLeft;

if(skew)
   zipfLeft = new Zipf(domain+1, inZipf, false); //false = no shufflE

  Uniform uniformLeft(domain+1);
  Normal norm(domain+1,mean,stddev);




  // generate ALL the queries
for (long i=0; i<n; i++) {

	QueryMD_t q;
    uint size;
    if(old)
    	size = 3 + zipfInterval.probe();
    else
    	size = /*inMin +*/ norm.probe();

    if(size<=0)
    	size = 1;
    if (skew)
      log[i].left = domain - zipfLeft->probe();
    else
      log[i].left = uniformLeft.probe();


    if (log[i].left + size > domain + 1) {
      // log[i].right = domain - 1;
      log[i].right = domain;
      log[i].left = domain - size + 1;
    }
    else
      log[i].right = log[i].left + size - 1;
  // generate all queries

    assert(log[i].right<=domain && log[i].left>=0 && log[i].right>=log[i].left);
  }  // generate all queries
}  // Constructor

Query::~Query() {
	if(logMD.size() == 0)
			free(log);
}  // Destructor

Query::QueryMD_t Query::nextQueryMD()
{
	 // roll over at end of log
	  if (logPos == n-1) {
	    logPos = 0;
	    return logMD[n-1];
	  }  // end of log

	  // advance log and return current position
	  logPos++;
	  return logMD[logPos-1];
}
Query::Query_t Query::nextQuery() {
  // roll over at end of log
  if (logPos == n-1) {
    logPos = 0;
    return log[n-1];
  }  // end of log

  // advance log and return current position
  logPos++;
  return log[logPos-1];
}

void Query::reset() {
  logPos = 0;
}


//  prettyPrint:  show the whole database
void Query::prettyPrint(Query_t q) {
  cout << endl << endl << "--------------------------------------------------------------" << endl;
  cout << "Query: [" << q.left << ", " << q.right << "]";
  cout << endl << "--------------------------------------------------------------" << endl << endl;;
}  // prettyPrint


Query::Query(char * filename) {

	 int num_queries = 0;
	vector < uint > values = vector<uint>();
	ifstream file;
	file.open(filename);
	 string line;
	 while(std::getline(file, line)) {
		uint num = atoi(line.c_str());
		num_queries++;
		values.push_back(num);

	}
	file.close();
	n = values.size();
	log = (Query_t*) malloc(n * sizeof(Query_t));
	logPos = 0;
	for (int i = 0; i < values.size(); i++) {
		log[i].left = values[i];
		log[i].right = values[i];
	}
	cout<<"Queries: "<<n<<endl;

}
Query::Query(uint inDomain, uint inMin,double mean,double stddev,
		double inZipf, long noQueriesInWorkload, bool skew,bool old,
		int seed) {
  domain = inDomain;
  n = noQueriesInWorkload;
  log = (Query_t*) malloc(n * sizeof(Query_t));
  logPos = 0;
  //int MEAN = mean;


  Zipf zipfInterval(2, inZipf);
  // for point in domain use either Zipf or Uniform; depending on value of skew
 // Zipf zipfLeft(domain+1, inZipf);
  //FIXME: there!



   Zipf zipfLeft(domain+1, inZipf,seed, true); //false = no shufflE

  Uniform uniformLeft(domain+1);
  Normal norm(domain+1,mean,stddev);




  // generate ALL the queries
for (long i=0; i<n; i++) {

	QueryMD_t q;
    uint size;
    if(old)
    	size = 3 + zipfInterval.probe();
    else
    	size = /*inMin +*/ norm.probe();

    if(size<=0)
    	size = 1;
    if (skew)
      log[i].left = domain - zipfLeft.probe();
    else
      log[i].left = uniformLeft.probe();


    if (log[i].left + size > domain + 1) {
      // log[i].right = domain - 1;
      log[i].right = domain;
      log[i].left = domain - size + 1;
    }
    else
      log[i].right = log[i].left + size - 1;
  // generate all queries

    assert(log[i].right<=domain && log[i].left>=0 && log[i].right>=log[i].left);
  }  // generate all queries
}  // Constructor


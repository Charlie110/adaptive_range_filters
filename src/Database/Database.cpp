/* ---------------------------------------------------------------------------
 *                               FUNCTION IMPLEMENTATION
 *
 *   CLASS NAME:        Database
 *
 *   FUNCTIONS:         Database::*
 *
 * ---------------------------------------------------------------------------
 */

#include "../Util.h"
#include "../Database.h"
#include "../Distribution.h"
#include "../curves/DatagenMD.h"


#include <string>
#include <iomanip> //for pretty plotting of the keys :)
#include <iostream>
#include <fstream>



bool Database::exists(int low,int high,int dim)
{
	for(int i=low;i<=high;i++)
	{
		if(exists(i,dim))
			return true;
	}
	return false;
}

bool Database::exists(int val, int dim)
{

	assert(curve->type =='c');
	//assert(curve->getDimensions()==2);
	int other = 1;
	if(dim==1)
		other = 0;

	for(int i=0;i<curve->getDomainOfDim(other);i++)
	{
		vector<int> pt(dims);
		pt[dim] = val;
		pt[other] = i;
		if(this->rangeQuery(pt,pt))
			return true;
	}
	return false;


}
FastDB::FastDB(uint64 inDomain):Database(1)
{
	domain = inDomain;
	values = std::set<uint64>();
	cout<<"fast db initialized"<<endl;
}

bool FastDB::contains(uint64 k)
{
	return (values.count(k)>0);
}

void Database::addPoint(vector<int> pt)
{
	int i = curve->linearizePointQuery(pt);
	set(i,true);
}

void FastDB::set(uint64 k , bool v)
{
	if(v && !contains(k))
		values.insert(k);

	if(!v && contains(k))
		values.erase(k);
}

Database::Database(Database * d, char curve_type)
{
	assert(d->curve->type =='c');


	if(curve_type =='c')
	{
		curve = new Ccurve(d->attr);
	}
	if(curve_type =='z')
	{
		curve = new Zcurve(d->attr);
	}

	if(curve_type =='h')
	{
		curve = new Hcurve(d->attr);
	}

	num_keys = 0;
	domain = d->curve->getDomain()+1;
	db = (bool*) malloc((domain) * sizeof(bool));
	clear();

	for(int i=0;i<=d->curve->getDomain();i++)
	{
		if(d->contains(i))
		{
			vector<int> pt = d->curve->delinearizeCurveValue(i);
			int new_i = curve->linearizePointQuery(pt);

			assert(!contains(new_i));
			set(new_i,true);
			num_keys++;
		}

	}
	cout<<"Database with "<<curve_type<<"-curve now has: "<<num_keys<<"keys"<<endl;
	//getchar();
}

void Database::populateMD(uint64 n,vector<int> domains,char correlation)
{


	/* TODO: write me */
	assert(dims == curve->getDimensions() );
	vector< vector<double> > coeff;



	num_keys = n;
	  DatagenMD datagen = DatagenMD();
	  if(correlation =='a')
	  coeff = datagen.GenerateDataAnticorrelated(NULL,2*n,dims);
	  if(correlation =='c')
		  coeff = datagen.GenerateDataCorrelated(NULL,2*n,dims);
	  if(correlation =='e')
		  coeff = datagen.GenerateDataEqually(NULL,2*n,dims);

	  int idx = 0;
	  for(int i=0;i<n;i++)
	  {
		  vector<int> pt(curve->getDimensions());
		 // cout<<"Populated: "<<i<<endl;

		  /* kapws twra ta ftiahnoume */

		  for(int j=0;j<dims;j++)
		  {
			  pt[j] = coeff[idx][j] * domains[j];
			 // cout<<"Val: "<<pt[j]<<endl;
		  }


		  int key =  curve->linearizePointQuery(pt);

		  if(!contains(key))
		  {
			  cout<<"DB val:"<<endl;
			  curve->printp(pt);
			  set(key,true);
		  }
		  else
		  {
			  i--;
		  }
		  idx++;


	  }


}


/* initiliaze from R data */

Database::Database(vector<int> columns,string file, char curve_type,
		bool sameDimForAll,int multiplier)
{
	   //assert(sameDimForAll || curveType=='c')

		dims = columns.size();
	    int minval = (2<<31);
	    int maxval = -2<<31;
	    int maxDim = 0;
	    num_keys = 0;

		/* parse the required columns */


	    vector<vector<double> > datapoints = parseCsv(columns,file);

	    cout<<"Data points read:"<<datapoints.size()<<endl;
	    vector<Curve::attribute> attr(dims);

	    /* normalize + find min/max */
	    for(int j=0;j<dims;j++)
	    {

	    	int minvald = (1<<30);
	    	int maxvald = -(1<<30);
			for(int i=0;i<datapoints.size();i++)
			{

				datapoints[i][j] = (int) (datapoints[i][j]*multiplier);
				if(j ==1)
				{
					/*cout<<"i:"<<i<<endl;
					cout<<"dim: "<<datapoints[i][j]<<endl;
					cout<<"minvald:"<<minvald<<endl;
					getchar();*/
				}
				if(datapoints[i][j]>maxvald)
					maxvald = datapoints[i][j];

				if(datapoints[i][j]<minvald)
									minvald = datapoints[i][j];


			}
			cout<<"Dimension: "<<j+1<<endl;
			cout<<"["<<minvald<<"-"<<maxvald<<"]"<<endl;
			attr[j].lowerb = 0;
			attr[j].higherb = closestPower2((int)(maxvald - minvald))-1;
			maxDim = std::max(maxDim,attr[j].higherb);
			maxDim = std::max(maxDim,(maxvald - minvald));
			cout<<"Domain of dimension "<<j+1<<": "<<attr[j].higherb<<endl;

			for(int i=0;i<datapoints.size();i++)
			{
				datapoints[i][j]-=minvald;
			}
	    }


	    if(sameDimForAll)
	    {
	    	for(int i=0;i<dims;i++)
	    	{
	    		attr[i].higherb = maxDim;
	    	}

	    }

	    /* create underlying space filling curve */

	    this->dims = attr.size();
	    this->attr = attr;
		if(curve_type =='z')
		{
			curve = new Zcurve(attr);
		}
		if(curve_type =='c')
		{
			curve = new Ccurve(attr);
		}
		if(curve_type =='h')
		{
			curve = new Hcurve(attr);
		}
		this->domain = curve->getDomain()+1;
		assert(dims == curve->getDimensions());
		db = (bool*) malloc((domain) * sizeof(bool));
		clear();

		 /* fill with datapoints */

	    for(int i=0;i<datapoints.size();i++)
	    {

	    	vector<int> pt(dims);
	    	for(int j=0;j<dims;j++)
	    	{
	    		pt[j] = (int) (datapoints[i][j]);
	    	}

	    	int val = curve->linearizePointQuery(pt);
	    	if(!contains(val))
	    	{
	    		set(val,true);
	    		num_keys++;

	    	}
	    	/*else
	    	{
	    		/*cout<<"duplicate key: "<<val<<endl;
	    		curve->printp(pt);
	    		if(curve_type =='h')
	    			getchar();
	    	}*/

	    }

	    cout<<"Database now has: "<<num_keys<<" distinct keys"<<endl;




}

Database::Database(uint64 inDomain, int dims) {
  domain = inDomain;
  db = (bool*) malloc((inDomain) * sizeof(bool));
  num_keys = 0;
  this->dims = dims;
  clear(); //set all entries of db to zero
  cout<<"base class of db initialized"<<endl;
} // Constructor


bool Database::contains(uint64 key)
{
	if(key>=domain) {
		cout<<"----WARNING: query exceeded database upper bound!----"<<endl;
		return false;
	}
	return db[key];
}


void Database::populateMD(uint64 n,Distribution * distr) //this has the domain of 1 dimension
{
	   num_keys = n;
	   PointGen ptgen = PointGen(this->dims,distr);
	  for (uint64 i=0; i<n; i++)
	  {
	   vector<int> newPt = ptgen.generate();
	   int newK = curve->linearizePointQuery(newPt);

	    if (contains(newK)) //if it's already there, we need a different one
	      i--;
	    else
	      set(newK,true);

	  }
}

void Database::populateMD(uint64 n,vector<Distribution *> distr)
{
	   num_keys = n;
	   PointGen ptgen = PointGen(this->dims,distr);
	  for (uint64 i=0; i<n; i++)
	  {
	   vector<int> newPt = ptgen.generate();
	   int newK = curve->linearizePointQuery(newPt);

	    if (contains(newK)) //if it's already there, we need a different one
	      i--;
	    else
	      set(newK,true);

	  }
}


Database::Database(vector<Curve::attribute> attr, char curve_type)
{

    this->dims = attr.size();
    this->attr = attr;
	if(curve_type =='z')
	{
		curve = new Zcurve(attr);
	}
	if(curve_type =='c')
	{
		curve = new Ccurve(attr);
	}
	if(curve_type =='h')
	{
		curve = new Hcurve(attr);
	}
	this->domain = curve->getDomain()+1;
	assert(dims == curve->getDimensions());
	db = (bool*) malloc((domain) * sizeof(bool));
	num_keys = 0;
	clear();
	cout<<"Multi dimensional database (with space filling curve) initialized..domain[): "<<domain<<endl;
	//cout<<"Size:"<<num
	//the end


}

Database::Database(vector<Curve::attribute> attr,char curve_type,string filename,int size)
{
	Curve * c = NULL;
	num_keys = 0;
	int maxdim = 0;
	for(int i=0;i<attr.size();i++)
	{
		assert(attr[i].lowerb == 0);
		maxdim = std::max(maxdim, attr[i].higherb+1);
	}
	if(curve_type =='z')
	{
		c = new Zcurve(attr);
	}
	if(curve_type =='c')
		{
			c = new Ccurve(attr);
		}
	if(curve_type =='h')
	{
		c = new Hcurve(attr);
	}

	domain = c->getDomain();
	 db = (bool*) malloc((domain) * sizeof(bool));
	int dims = c->getDimensions();
	 ifstream file;
	 file.open(filename);
	 assert(file.good());
	 int t;
	 file>>t;
	 file>>t;

	 while(!file.eof())
	 {
		 vector<int> nums(dims);

		 for(int i=0;i<dims;i++)
		 {
			 double num;
			 file>>num;
			 cout<<"num:"<<num<<endl;
			 nums[i] = (int) (num* maxdim);
		 }

		 //getchar();
		 int curveValue = c->linearizePointQuery(nums);
		 if(!contains(curveValue))
			 num_keys++;
		 cout<<"curveValue: "<<curveValue<<endl;
		 cout<<"dims:"<<dims<<endl;
		 cout<<"dom:"<<c->getDomain()<<endl;
		 set(curveValue,true);
	 }
	 file.close();


}

Database::Database(char * filename, uint64 inDomain)
{
	 domain = inDomain+1;
	 db = (bool*) malloc((domain) * sizeof(bool));
	 num_keys = 0;
	 clear();
	 ifstream file(filename);

	 string line;
	 while(std::getline(file, line))
	 {
		 uint64 num = atoi(line.c_str());

		 if(db[num] == false)
			 num_keys++;
		 db[num]= true;
	 }
	 file.close();
}


Database::~Database() {
  free(db);
}  // Destructor


// Warning may result in endless loop if no more space
// no duplicate keys because it doesnt matter

vector<uint64> Database::getKeys()
{

	vector<uint64> res;
	for(int i=0;i<domain;i++)
	{

		if(contains(i))
			res.push_back(i);

	}


	return res;

}

int Database::size()
{
	return num_keys;
}

vector<Query::Query_t> Database::determineEmptyRanges(Query::Query_t r,int lowerb, int domain)
{
	vector<Query::Query_t> res;
	//cout<<"Query:"<<r.left<<"-"<<r.right<<endl;
	//cout<<"trie range:"<<lowerb<<"-"<<lowerb + domain<<endl;

	if(r.left<0)
		r.left =0;
	if(r.right>domain)
		r.right=domain;

	r.left+=lowerb;
	r.right+=lowerb;


	//cout<<"adj Query:"<<r.left<<"-"<<r.right<<endl;
	for(int i=r.left;i<=r.right;i++)
	{
		if(!contains(i))
		{
			Query::Query_t empty;
			empty.left = i;
			empty.right = i;
			int j=i+1;
			while(j<=r.right && !contains(j))
			{
				empty.right = j;
				j++;
			}
			i = j; //so that
			res.push_back(empty);
		}
	}

	for(int i=0;i<res.size();i++)
	{
		res[i].left-=lowerb;
		res[i].right-=lowerb;

	}

	return res;
}



vector<Query::Query_t> Database::determineEmptyRanges(Query::Query_t r)
{
	vector<Query::Query_t> res;
	if(r.left<0)
		r.left =0;
	if(r.right>domain)
		r.right=domain;

	for(int i=r.left;i<=r.right;i++)
	{
		if(!contains(i))
		{
			Query::Query_t empty;
			empty.left = i;
			empty.right = i;
			int j=i+1;
			while(j<=r.right && !contains(j))
			{
				empty.right = j;
				j++;
			}
			i = j; //so that
			res.push_back(empty);
		}
	}
	return res;
}

void Database::clear()
{
 for (uint64 i=0; i<domain; i++)
    set(i,false);

}

void Database::sanity_check()
{
int sum = 0;
for (uint64 i=0; i<domain; i++)
{
 if(contains(i) == true)
	++sum;
}

assert(sum==num_keys);
//cout<<"db sanity check succeeded"<<endl;

}



void Database::plot()
{
	if(num_keys<=0) {
		cout<<"---------- DATABASE IZ EMPTY ------"<<endl;
		return;
	}
	cout<<"------- KEY DISTRIBUTION IN COLD STORE ----------"<<endl;
	cout<<"Num keys:"<<num_keys<<endl;

	//int domain = 40000;
	int partitions = 32;//16.0;
	int step = (domain+1)/partitions;
	int nstars = 100;

	for (int i=0; i<partitions; ++i)
	{
		int count = 0;
		for(int j=(i*step);j<((i+1)*step);j++)
		{
			if(j>domain)
				break;
			if(contains(j))
				count++;
		}
		cout<<"[";
		cout.fill('0');
		cout.width(7);
		cout<<(i*step)<<" - ";
		cout.fill('0');
		cout.width(7);
		cout<<(i+1)*step<<"]";
	  	cout << string(count*nstars/num_keys,'*') <<endl;
 	 }

	cout<<"---------------------------------------------"<<endl;



}

void Database::populate(uint64 n, Distribution * dist) //OO FTW
{
	num_keys = n;
	if(dims>1)
	{
		populateMD(n,dist);
		return;
	}

  for (uint64 i=0; i<n; i++) {

    uint64 newK = dist->probe(); // get a key from the distribution
   // cout<<"new key:"<<newK<<endl;
    if (contains(newK)) //if it's already there, we need a different one
      i--;
    else {
      set(newK,true);

      //result[i] = newK;
    }
  }

}

void Database::set(uint64 k,bool val)
{
	db[k]=val;
}


uint64 * Database::addKeys(uint64 n, Distribution * dist)
{

  uint64* result = (uint64*) malloc(n * sizeof(uint64));

  for (uint64 i=0; i<n; i++) {
    uint64 newK = dist->probe(); // get a key from the distribution
    if (contains(newK)) //if it's already there, we need a different one
      i--;
    else {
     set(newK,true);
      result[i] = newK;
    }
  }
  num_keys+=n;
  return result;

}


void Database::removeKeys(uint64 n)
{

	Uniform dist(num_keys);
	  for (uint64 i=0; i<n; i++)
	  {
	    uint64 delK = dist.probe();
	    if(contains(delK))
	    {
	    	set(delK,false);
	    }
	    else
	    {
	    	--i;
	    }
	  }

	 num_keys-=n;
	return;

}

// ????????? what does this even do??
// warning: might run into an endless loop if there are not enough keys in the database
void Database::deleteKeys(uint64 n) {
  // extract the database into a separate array; also keep a marker for which records
  // have already been deleted
  bool* marker = (bool*) malloc(domain * sizeof(bool));
  uint64* dbCopy = (uint64*) malloc(domain * sizeof(uint64));
  uint64 dbSize = 0;
  Query::Query_t q;
  q.left = 0;
  q.right = domain-1;
  uint64 item = getFirstResult(q);
  while (item != noValue()) {
    marker[dbSize] = false;
    dbCopy[dbSize] = item;
    dbSize++;
    item = getNextResult(q, item);
  }  // extract database

  // check how many we can delete
  if (n > dbSize)
    n = dbSize;

  // delete n random items from dbCopy
  Uniform dist(dbSize);
  for (uint64 i=0; i<n; i++) {
    uint64 delK = dist.probe();
    // key already deleted
    if (marker[delK])
      i--;
    else {
      db[dbCopy[delK]] = false;//FIXME used to be true
      marker[delK] = true;
    }
  }  // delete n items

  num_keys-=n;
  // clean up
  free(marker);
  free(dbCopy);
}  // delete


vector< vector<int> > Database::getPoints()
{

	vector< vector<int> > res;

	if(curve ==NULL)
	{
		for(int i=0;i<domain;i++)
		{
			if(contains(i))
			{
				vector<int> pt(1);
				pt[0] = i;
				res.push_back(pt);
			}
		}

		return res;

	}

	for(int i=0;i<=curve->getDomain();i++)
	{
		if(contains(i))
		{
			res.push_back(curve->delinearizeCurveValue(i));
		}
	}

	return res;
}


Database::Database(Database *d,int dimension)
{
	assert(d->curve->type =='c');
	curve = NULL;

	num_keys = 0;
	domain = d->curve->getDomainOfDim(dimension)+1;
	db = (bool*) malloc((domain) * sizeof(bool));
	clear();


	for(int i=0;i<d->curve->getDomain();i++)
	{
		if(!d->contains(i))
			continue;

		vector<int> pt = d->curve->delinearizeCurveValue(i);

		if(!contains(pt[dimension]))
		{
			num_keys++;
			set(pt[dimension],true);
		}

	}
}


bool Database::rangeQuery(vector<int> left,vector<int> right,int cdim)
{
	if(dims == 1)
		return rangeQuery(left[0],right[0]);

	if(left.size() ==1 && dims!=1)
	{
		cout<<"Dual synopsis at work"<<endl;
		assert(cdim!=-1);
		return exists(left[0],right[0],cdim);
	}

	vector<Query::Query_t> qs = curve->linearizeRangeQuery(left,right);
	bool res = false;
	for(int i=0;i<qs.size();i++)
	{
	 bool res2 = rangeQuery(qs[i].left,qs[i].right);
	 if(res2)
	 {
		// vector<int> low =curve->delinearizeCurveValue(qs[i].left);
		// vector<int> high =curve->delinearizeCurveValue(qs[i].right);
		// cout<<"db hit!"<<endl;
		// cout<<low[0]<<" "<<low[1]<<"-"<<high[0]<<" "<<high[1]<<endl;
		 return true;
	 }
	 else res = res | res2;
	}
	return res;
}

bool Database::rangeQuery(uint64 left, uint64 right) {
  // check input parameters
  if ((left < 0) || (left > right) || (right >= domain)) {
    //cout << "WARNING: Range Query with illegal range" << endl;
   // cout<<left<<"-"<<right<<endl;
    //return false;
  }  // undefined range
  if(right>=domain)
	  right =domain -1;
  if(left<0)
	  left=0;
  //cout<<"Q:"<<left<<"-"<<right<<endl;

  // carry out query; return true if hit
  for (uint64 i=left; i<=right; i++)
    if (contains(i))
    {
    	 //vector<int> high =curve->delinearizeCurveValue(i);
    	//		 cout<<"db hit!"<<endl;
    		//	 cout<<"-"<<high[0]<<" "<<high[1]<<endl;
      return true;
    }

  return false;
}  // carry out range query


// execute range queries, thereby actually returning results
// here get first result
// return "null" if there is no result
uint64 Database::getFirstResult(Query::Query_t q) {
  // check input parameters
  if ((q.left < 0) || (q.left > q.right) || (q.right >= domain)) {
    cout << "Warning: Range Query with illegal range" << endl;
    return noValue();
  }  // undefined range

  // carry out query; return true if hit
  for (uint64 i=q.left; i<=q.right; i++)
    if (contains(i))
      return i;

  return noValue();
}  // getFirstResult

// execute range queries, thereby actually returning results
// here get first result that is bigger than p
// return "null" if there is no such result

uint64 Database::getNextResult(Query::Query_t q, uint64 p) {
  // check input parameters
  if ((q.left < 0) || (q.left > q.right) || (q.right >= domain)) {
    cout << "Warning: Range Query with illegal range" << endl;
    return noValue();
  }  // undefined range

  // carry out query; return true if hit
  for (uint64 i=p+1; i<=q.right; i++)
    if (contains(i))
      return i;

  return noValue();
}  // getNextResult

//  prettyPrint:  show the whole database
void Database::prettyPrint() {
  cout << endl << endl << "--------------------------------------------------------------" << endl;
  Query::Query_t q;
  q.left = 0;
  q.right = domain-1;
  uint64 result = getFirstResult(q);
  while (result != noValue()) {
    cout << result << " ";
    result = getNextResult(q, result);
  }  // while all results
  cout << endl << "--------------------------------------------------------------" << endl << endl;;
}  // prettyPrint

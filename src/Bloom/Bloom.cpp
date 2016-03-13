/* ---------------------------------------------------------------------------
 *                               CLASS IMPLEMENTATION
 *
 *   CLASS NAME:        Bloom
 *
 *   FUNCITONS:         Bloom::*
 *
 * ---------------------------------------------------------------------------
 */

#include "../Util.h"
#include "../Query.h"
#include "../Database.h"
#include "../Bloom.h"
#include "../Synopsis.h"
#include "../Synopsis/Statistics.h"


 Bloom::Bloom(uint domain, long noKeys, Database* db, long inSize, int range) {

	size = inSize;
	this->range_covered = range;
	stats = Statistics();
	this->noKeys = noKeys;// actual nr is smaller

	b = vector<bool> (size);

	for (long i = 0; i < size; i++)
		b[i] = false;

	c = NULL;

	// initialize seeds of hash functions
	theta = (double*) malloc(MAXHASHES * sizeof(double));
	theta[0] = THETA0;
	theta[1] = THETA1;
	theta[2] = THETA2;
	theta[3] = THETA3;
	theta[4] = THETA4;
	theta[5] = THETA5;
	theta[6] = THETA6;
	theta[7] = THETA7;

	// initialize the number of hash functions
	// use table of (Google for "bloom filter math"): http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html
	if (inSize < noKeys * 3)
		k = 1;
	else if (inSize < noKeys * 4)
		k = 2;
	else if (inSize < noKeys * 6)
		k = 3;
	else if (inSize < noKeys * 7)
		k = 4;
	else if (inSize < noKeys * 8)
		k = 5;
	else if (inSize < noKeys * 10)
		k = 6;
	else if (inSize < noKeys * 11)
		k = 7;
	else
		k = 8;

	Query::Query_t q;
	q.left = 0;
	q.right = domain - 1;
	uint curr = 0;
	int n = 0;

	while(curr<db->getDomain()) {

		bool value = db->rangeQuery(curr,curr+range_covered-1);

		if (value == true) {

			if(range_covered == 30)
				cout<<"value recorded:"<<curr/range_covered<<endl;
			this->recordNewKey(curr/range_covered);
			n++;
		}

		curr+=range_covered;


	}

	//printf("Expected keys(trivial): %d, actual: %d \n",noKeys,n);

	if(range_covered == 30) {
	curr = 0;
	cout<<"1234!!"<<endl;
	while(curr<db->getDomain()) {

		
		//assert(db->rangeQuery(curr,curr+range_covered-1) == this->pointQuery(curr/range_covered));
		curr+=range_covered;
	}

	}

} // Constructor


bool Bloom::handleQueryRanged(uint left, uint right,bool qR, Database *db)
{

	bool sR = false;

	/* check what this ranged bloom filter does */
	int curr_idx = left/range_covered;
	int curr_offset = left % range_covered;
	int curr = left;
	int pointsLooked = 0;
	while(true) {

		pointsLooked++;
		if(pointQuery(curr_idx) == true) {
			sR = true;
			break;
		}
		curr_idx = curr_idx +1;
		curr = curr + (range_covered - curr_offset);

		curr_offset = 0;
		if(curr>right)
			break;

	}
	/*if(range_covered == 30) {
		cout<<"pts looked:"<<pointsLooked<<endl;
		assert(pointsLooked == 1);
	}*/
	assert(!(!sR && qR));
	stats.update(sR,qR);
	return sR;


}

double Bloom::fpRate(int rangelen)
{

	double pb = fpRate();

	double exponent = pow(1.0-pb,rangelen/range_covered);
	return 1 - exponent;

}

void Bloom::sanityCheck()
{
	int cnt = 0;
	for(int i=0;i<b.size();i++)
		if(b[i] ==0)
			cnt++;
	cout<<"Bloom has "<<cnt<<"empties out of "<<this->size<<endl;
}


bool Bloom::handleQuery(Query::QueryMD_t q,bool qR)
{

	bool sR = rangeQuery(q);
	assert(!(!sR && qR));
	stats.update(sR,qR);
	return sR;


}

bool Bloom::handleQuery(Query::Query_t q,bool qR)
{

	bool sR = rangeQuery(q);
	assert(!(!sR && qR));
	stats.update(sR,qR);
	return sR;


}
double Bloom::fpRate()
{

	//(1 - e^(kn/m))^k
	const double epsilon = 2.71828183;
	double exp = k*noKeys/(0.0 +size);
	//cout<<"exp:"<<exp<<endl;
	exp = pow(epsilon,exp);
	//cout<<"exp:"<<exp<<endl;
	//0.6185 *(m/n)


	double mn = (size+0.0)/ noKeys;
	double res = pow(0.6185,mn);

	return res;

}

Bloom::Bloom(uint domain, long noKeys, Database* db, long inSize) {
  size = inSize;
  stats = Statistics();
  this->range_covered = 1;
  this->noKeys = noKeys;
  //b = (bool*) malloc(size * sizeof(bool));
  b = vector<bool>(size);

  for (long i=0; i<size; i++)
    b[i] = false;

  c = NULL;


  // initialize seeds of hash functions
  theta = (double*) malloc(MAXHASHES * sizeof(double));
  theta[0] = THETA0;
  theta[1] = THETA1;
  theta[2] = THETA2;
  theta[3] = THETA3;
  theta[4] = THETA4;
  theta[5] = THETA5;
  theta[6] = THETA6;
  theta[7] = THETA7;

  // initialize the number of hash functions
  // use table of (Google for "bloom filter math"): http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html
  if (inSize < noKeys * 3)
    k = 1;
  else if (inSize < noKeys * 4)
    k = 2;
  else if (inSize < noKeys * 6)
    k = 3;
  else if (inSize < noKeys * 7)
    k = 4;
  else if (inSize < noKeys * 8)
    k = 5;
  else if (inSize < noKeys * 10)
    k = 6;
  else if (inSize < noKeys * 11)
    k = 7;
  else
    k = 8;

  Query::Query_t q;
  q.left = 0;
  q.right = domain-1;
  uint item = db->getFirstResult(q);
  while (item != db->noValue()) {
    recordNewKey(item);
    item = db->getNextResult(q, item);
  }
}  // Constructor


Bloom::Bloom(uint domain, long noKeys, Database* db, long inSize,Curve * curve) {
  size = inSize;
  this->noKeys = noKeys;
  this->range_covered = 1;
  //b = (bool*) malloc(size * sizeof(bool));
  b = vector<bool>(size);

  stats = Statistics();

  for (long i=0; i<size; i++)
    b[i] = false;

  c = curve;


  // initialize seeds of hash functions
  theta = (double*) malloc(MAXHASHES * sizeof(double));
  theta[0] = THETA0;
  theta[1] = THETA1;
  theta[2] = THETA2;
  theta[3] = THETA3;
  theta[4] = THETA4;
  theta[5] = THETA5;
  theta[6] = THETA6;
  theta[7] = THETA7;

  // initialize the number of hash functions
  // use table of (Google for "bloom filter math"): http://pages.cs.wisc.edu/~cao/papers/summary-cache/node8.html
  if (inSize < noKeys * 3)
    k = 1;
  else if (inSize < noKeys * 4)
    k = 2;
  else if (inSize < noKeys * 6)
    k = 3;
  else if (inSize < noKeys * 7)
    k = 4;
  else if (inSize < noKeys * 8)
    k = 5;
  else if (inSize < noKeys * 10)
    k = 6;
  else if (inSize < noKeys * 11)
    k = 7;
  else
    k = 8;

  Query::Query_t q;
  q.left = 0;
  q.right = domain-1;
  uint item = db->getFirstResult(q);
  while (item != db->noValue()) {
    recordNewKey(item);
    item = db->getNextResult(q, item);
  }
}  // Constructor



Bloom:: ~Bloom() {

  free(theta);
}  // Destructor



bool Bloom::pointQuery(int key)
{

#ifdef TICKTOCK
 	 const uint64_t lookup0 = rdtscp();

#endif

 	// cout<<"bloom point query"<<endl;
    for (int i=0; i<k; i++)
      	if (! b[hash(key, i)])
	{
			#ifdef TICKTOCK
			const uint64_t lookup1 = rdtscp();
			if(lookup1>lookup0)
			{
			 lookupt.push_front(lookup1-lookup0);
			}

			#endif

        	return false;
	}

	#ifdef TICKTOCK
	const uint64_t lookup1 = rdtscp();
	if(lookup1>lookup0)
	{
	 lookupt.push_front(lookup1-lookup0);
	}

	#endif

   return true;
}

/*
bool Bloom::pointQuery(int key)
{
#ifdef TICKTOCK
 	 const uint64_t lookup0 = rdtscp();

#endif

 	int sum =0;
    for (int i=0; i<k; i++)
      	sum+= b[hash(key, i)];

#ifdef TICKTOCK
const uint64_t lookup1 = rdtscp();
if(lookup1>lookup0)
{
 lookupt.push_front(lookup1-lookup0);
}

#endif

   return (sum==k);
}*/

int Bloom::getK()
{
	return k;
}

bool Bloom::rangeQuery(Query::QueryMD_t q)
{


	vector<Query::Query_t> queries = c->linearizeRangeQuery(q.low,q.high);
		bool res = false;

		for(int i=0;i<queries.size();i++)
		{

			res = rangeQuery(queries[i]);
			if(res)
				return true;
		}
		assert(res == false); //if we've reached that point :)
		return res;

}

bool Bloom::pointQuery(vector<int> pt)
{
	return (pointQuery(c->linearizePointQuery(pt)));

}

bool Bloom::rangeQuery(Query::Query_t q) {

#ifdef TICKTOCK
 	 const uint64_t lookup0 = rdtscp();

#endif
 // cout<<"bloom range query"<<endl;
  uint curr = q.left;
  while(curr<=q.right){
    bool res = true;
    for (int i=0; i<k; i++)
    {
      if (! b[hash(curr, i)])
      {
        res = false;
        break;
      }
    }
    if(res == true)
    {
	#ifdef TICKTOCK
		const uint64_t lookup1 = rdtscp();
		 if(lookup1>lookup0)
		 {
			 lookupt.push_front(lookup1-lookup0);
		 }

	#endif
      return true;
    }//if one point in range returns true, entire range returns true
    curr++;
  }
	#ifdef TICKTOCK
		const uint64_t lookup1 = rdtscp();
		 if(lookup1>lookup0)
		 {
			 lookupt.push_front(lookup1-lookup0);
		 }

	#endif

  return false;
}  // range Query


void Bloom::recordNewKey(uint newK) {
  for (int i=0; i<k; i++)
    b[hash(newK, i)] = true;
}  // recordNewKey

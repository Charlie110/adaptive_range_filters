/* ---------------------------------------------------------------------------
 *                               FUNCTION IMPLEMENTATION
 *
 *   CLASS NAME:        Zipf
 *
 *   FUNCTIONS:         Zipf::*
 *
 * ---------------------------------------------------------------------------
 */

using namespace std;

//alright the assumption is this thing returns something from 0 to domain

#include "../Distribution.h"

//domain is [0,domain]

Zipf::Zipf(uint64 inDomain, double inTheta, int seed, bool shuffle) :Distribution(inDomain){

  theta = inTheta;
  // allocate array with the distribution
  distr = (double *) malloc(domain * sizeof(double));
  distr[0] = 1;
  double zeta = 1;
  for (uint64 i=1; i<domain; i++) {
    distr[i] = pow(1.0/(i+1), theta);
    zeta += distr[i];
  }

  distr[0] = distr[0] / zeta;
  for (uint i=1; i<domain; i++) {
    distr[i] = distr[i] / zeta;
    distr[i] += distr[i-1];
  }


	  shuffler = Shuffler(inDomain,seed);
	  this->shuffle = shuffle;


} // Constructor

Zipf::Zipf(uint64 inDomain, double inTheta, bool shuffle) :Distribution(inDomain){

  theta = inTheta;
  // allocate array with the distribution
  cout<<"befoar malloc"<<endl;
  distr = (double *) malloc(domain * sizeof(double));
  if(distr== NULL)
  {
	  cout<<"Zipf malloc error"<<endl;
	  assert(1==0);
  }
  distr[0] = 1;
  double zeta = 1;
  for (uint64 i=1; i<domain; i++) {
    distr[i] = pow(1.0/(i+1), theta);
    zeta += distr[i];
  }

  distr[0] = distr[0] / zeta;
  for (uint i=1; i<domain; i++) {
    distr[i] = distr[i] / zeta;
    distr[i] += distr[i-1];
  }
  if(shuffle)
  {
	  shuffler = Shuffler(inDomain);
	  this->shuffle = true;
  }
  else
	  this->shuffle = false;

} // Constructor


Zipf::~Zipf() {
  free(distr);
}  // Destructor

uint64 Zipf::probe() { // generate a zipfly distributed number from [0.domain] :D
  //cout<<"Probing zipf"<<endl;
//so, mostly, it will be small numbers, but, there will be a few bigger ones thrown in the mix, but sparsely so
  double r = (double(std::rand()) / RAND_MAX) * distr[domain - 1];
  for (uint i=0; i<domain; i++)
    if (r <= distr[i]) // find the first one that is smaller or equal, ie, skip the larger probs
    {
    	if(shuffle)
    		return shuffler.shuffle(i);
    	else
    		return i;
    }

  cout << "There is something wrong here" << endl;
  if(shuffle)
	  return shuffler.shuffle(domain);
  else
	  return domain;
}  // probe

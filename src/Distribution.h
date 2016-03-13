/* ---------------------------------------------------------------------------
 *                               CLASS INTERFACES
 *
 *   CLASS NAME:        Normal, Uniform, Zipf
 *
 *   CLASS KEY :        Distribution
 *
 *   DESCRIPTION:       Implements typical distributions
 *
 * ---------------------------------------------------------------------------
 */

#include <cstdlib>
#include <cmath>
#include "Util.h"
#include "Distribution/Shuffler.h"
#include <random>
#include <functional>
#include <iostream>
#include "Util.h"
/* Make sure that this file is included only once */
#ifndef Distribution_H
#define Distribution_H


//typedef unsigned long long int uint;

class Distribution
{
protected:

  uint64 domain;
  public:
  Distribution(uint64 inDomain);
  virtual ~Distribution();
  virtual uint64 probe()=0;
  uint64 getDomain();

};

class Uniform: public Distribution{

 public:
  Uniform(uint64 inDomain);
  ~Uniform();
  uint64 probe();
};  // Uniform

class Zipf: public Distribution {
 protected:

  double theta;    // skew of Zipf distribution
  double* distr;   // array that captures the distribution

  bool shuffle;

 public:
  Shuffler shuffler;
  Zipf(uint64 inDomain, double inTheta, bool shuffle=false);
  Zipf(uint64 inDomain, double inTheta, int seed, bool shuffle = false);
  ~Zipf();
  uint64 probe();
};

class Normal : public Distribution {
 protected:
       // size of the domain; all keys are in [0 ... domain[
  std::default_random_engine generator;
  std::normal_distribution<double> distribution;
  double mean;
  double stdev;

 public:
  Normal(uint inDomain,double mean, double s);
  ~Normal();
  uint64 probe();
  //void graph();
};

#endif // Distributions_H

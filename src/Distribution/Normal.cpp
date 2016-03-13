/* ---------------------------------------------------------------------------
 *                               FUNCTION IMPLEMENTATION
 *
 *   CLASS NAME:        Normal
 *
 *   FUNCTIONS:         Normal::*
 *
 * ---------------------------------------------------------------------------
 */

#include "../Util.h"
#include "../Distribution.h"


Normal::Normal(uint inDomain,double mean, double s):Distribution(inDomain) {
  domain = inDomain;
  //mean = ceil(inDomain/2.0);
  this->mean = mean;
  this->stdev = s;
  distribution = std::normal_distribution<double>(this->mean,this->stdev);


} // Constructor





Normal::~Normal() {

}  // Destructor


uint64 Normal::probe() {

  //cout <<"probing normal"<<endl;
  double number = 0;
  do
  {
    number = distribution(generator);
   }while(number>domain || number<0);
   //cout<<"numbar: "<<number<<endl;
  return number; //pros to paron

}

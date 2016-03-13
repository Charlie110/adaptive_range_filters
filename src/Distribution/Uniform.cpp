/* ---------------------------------------------------------------------------
 *                               FUNCTION IMPLEMENTATION
 *
 *   CLASS NAME:        Uniform
 *
 *   FUNCTIONS:         Uniform::*
 *
 * ---------------------------------------------------------------------------
 */

#include "../Util.h"
#include "../Distribution.h"

Uniform::Uniform(uint64 inDomain):Distribution(inDomain)
{

} // Constructor


Uniform::~Uniform() {
}  // Destructor

uint64 Uniform::probe() {
  return std::rand() % domain;
}

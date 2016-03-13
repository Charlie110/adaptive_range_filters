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

Distribution::Distribution(uint64 inDomain)
{
 domain = inDomain;
} // Constructor
 uint64 Distribution::getDomain()
{
	return domain;
}

Distribution::~Distribution() {
}  // Destructor

/*uint Distribution::probe() {
  return 0;
} */

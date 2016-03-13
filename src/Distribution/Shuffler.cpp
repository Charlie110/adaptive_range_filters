/*
 * Shuffler.cpp
 *
 *  Created on: Dec 10, 2012
 *      Author: carolinux
 */

#include "Shuffler.h"
#include <random>
#include <algorithm>
#include <iostream>


Shuffler::Shuffler()
{

}

Shuffler::Shuffler(uint64 domain,int seed) {
	// TODO Auto-generated constructor stub

	shuffled = vector<uint64>(domain);

	for(uint64 i=0;i<domain;i++)
	{
		shuffled[i] = i;
	}
	srand(seed);
	std::random_shuffle(shuffled.begin(),shuffled.end());
	/* debug */
	/*
	for(int i=0;i<20;i++)
	{
		std::cout<<i<<"th: "<<shuffled[i]<<std::endl;
	}*/

}

Shuffler::Shuffler(uint64 domain) {
	// TODO Auto-generated constructor stub

	shuffled = vector<uint64>(domain);

	for(uint64 i=0;i<domain;i++)
	{
		shuffled[i] = i;
	}
	std::random_shuffle(shuffled.begin(),shuffled.end());

}

Shuffler::~Shuffler() {
	// TODO Auto-generated destructor stub
}
Shuffler::Shuffler(vector<uint64> s)
{
	shuffled = s;
}

uint64 Shuffler::shuffle(uint64 val)
{
	return shuffled[val];
}

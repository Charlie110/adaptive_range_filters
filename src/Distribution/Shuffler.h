/*
 * Shuffler.h
 *
 *  Created on: Dec 10, 2012
 *      Author: carolinux
 */
#include <vector>
#include "../Util.h"
using namespace std;
#ifndef SHUFFLER_H_
#define SHUFFLER_H_

class Shuffler {

	std::vector<uint64> shuffled;

public:


	vector<uint64> getShuffle()
	{
		return shuffled;
	}

	void setShuffle(vector<uint64> a)
	{

		shuffled = a;
	}



	Shuffler();
	Shuffler(uint64 domain);
	Shuffler(uint64 domain,int seed);
	Shuffler(vector<uint64> s);
	virtual ~Shuffler();
	uint64 shuffle(uint64);


};

#endif /* SHUFFLER_H_ */

/*
 * Experiment3Testbench.cpp
 *
 *  Created on: Oct 8, 2012
 *      Author: carolinux
 */



#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <sstream>

using namespace std;



bool spawn(const char *);

int main()
{
  stringstream ss;

  for
  ss<<"RANGEFILTERSIZE "<<RANGEFILTERSIZE<<endl;
  ss<<"DBSIZE "<<DBSIZE<<endl;



  ss.clear();

  string command ="./Experiment1.out";
  spawn(command.c_str());
  /* get results */


  cout<<"I spawned something"<<endl;


}


bool spawn(const char * command)
{

   FILE *fpipe;
   char line[256];

   if ( !(fpipe = (FILE*)popen(command,"r")) )
   {

	  perror("Problems with pipe");
	  exit(1);
   }

   while ( fgets( line, sizeof line, fpipe))
   {
	 printf("%s", line);
   }
   int status=  pclose(fpipe);
   return (status == 0);
}


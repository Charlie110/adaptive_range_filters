#include "../MultiTrie.h"
#include <sstream>



void MultiTrie::resetTime()
{

	for(int i=0;i<partitions;i++)
		tries[i]->resetTime();
	multiple =0;

	lookupt.clear();
	adaptt.clear();
	truncatet.clear();

	tn = 0;
	tp = 0;
	fp = 0;
	lastMerged = -1;


}



void MultiTrie::print(int idx)
{

	tries[idx]->prettyPrint();

}

int MultiTrie::size()
{
	int sum = 0;
	for(int i=0;i<tries.size();i++)
	{
		sum+=tries[i]->num_bits;
	}
	return sum;

}


void MultiTrie::truncateClock(int target_size)
{
	//FIXME no truncate?

	/* merge redundant info first */
#ifdef MERGE

	int i = 0;
	int merge_idx = (lastMerged+1) % tries.size();

	while(this->size()>target_size && i<tries.size())
	{
		tries[merge_idx]->snapshots = this->snapshots;
		tries[merge_idx]->merge();
		this->snapshots = tries[merge_idx]->snapshots;
		merge_idx++;
		merge_idx = merge_idx % tries.size();
		i++;
	}
	lastMerged = merge_idx -1;
#endif

#ifdef DOTRUNCATE
	int idx = victim_trie;
	bool visited_prev = false;
   // cout<<"------START TRUNCATE------"<<endl;
	list<uint64_t> trunc;
	while(this->size()>target_size)
	{
		tries[idx]->snapshots = this->snapshots;
		int diff = (this->size() - target_size);
		int target_size_curr = tries[idx]->num_bits - diff;
		if(tries[idx]->prev_victim.lvl == -1)
			visited_prev = true; /* start from the beginning */
		/*bool truncated = tries[idx]->evict_victims(-1,0,tries[idx]->prev_victim.lvl,tries[idx]->prev_victim.idx,
				&visited_prev,target_size_curr);*/


		#ifdef TICKTOCK
		 const uint64_t trunc0 = rdtscp();

		#endif
		bool truncated = tries[idx]->evict_top(tries[idx]->prev_victim.lvl,tries[idx]->prev_victim.idx,
						&visited_prev,target_size_curr);

		#ifdef TICKTOCK
			 const uint64_t trunc1 = rdtscp();
			 if(trunc1>trunc0)
			 {
				 /*tries[idx]->*/trunc.push_front(trunc1-trunc0);
			 }

		#endif

		//cout<<"Size:"<<this->size()<<" vs "<<target_size<<endl;
		if(truncated) /* if the truncation was successful */
			/* we stay at the same trie with the victim pointer pointing to the last truncated */
		{


			victim_trie = idx;
			//cout<<"stay at same idx "<<victim_trie<<endl;
			//cout<<tries[idx]->prev_victim.lvl<<","<<tries[idx]->prev_victim.idx<<endl;
			break;
		}
		else /* we maxxed out this trie and we need to move on to the next trie */
		{
			//cout<<"procceed to "<<(idx+1)%tries.size()<<endl;
			visited_prev = true; /* so we start from the beginning of the next trie */
			/* and reset the victim pointer of the current trie */
			tries[idx]->prev_victim.lvl = -1; /* invalidate the previous victim */

		}
		this->snapshots = tries[idx]->snapshots;
		idx = idx +1;
		idx = idx % tries.size();
	}
#endif


#ifdef TICKTOCK
	list<uint64_t>::iterator k;
	uint64_t sum = 0;
	for(k=trunc.begin(); k != trunc.end(); ++k)
	{
		sum+=*k;
	}
	if(!trunc.empty())
		truncatet.push_front(sum);

#endif


}

uint64_t MultiTrie::getSum(int mode)
{

	list<uint64_t> *v = NULL;
	uint64_t sum = 0;

	for(int i=0;i<tries.size();i++)
		{

				if(mode == ADAPT)
				{
					//cout<<" adapt";
						v = &(tries[i]->falsepos);
				}
				if(mode == LOOKUP)
						v = &(tries[i]->lookupt);
				if(mode == TRUNCATE)
				{
						//cout<<" truncate ";
						v =  &(tries[i]->truncatet);
						//assert(v->size()>0);
				}


				list<uint64_t>::iterator it;
				//v->sort(); //sort the list
				//get the medians

             int j = 0;
			 for(it=v->begin(); it!= v->end(); ++it)
			 {


					sum+=*it;
					j++;

			 }




		}
	//cout<<"Summ: "<<sum<<endl;
	return sum;
}

uint64_t MultiTrie::getStddev(int mode,int QUERIES)
{

	list<uint64_t> *v = NULL;
	uint64_t sum = 0;

	for(int i=0;i<tries.size();i++)
		{

				if(mode == ADAPT)
						v = &(tries[i]->falsepos);
				if(mode == LOOKUP)
						v = &(tries[i]->lookupt);
				if(mode == TRUNCATE)
						v =  &(tries[i]->truncatet);


				list<uint64_t>::iterator it;

				for(it=v->begin(); it != v->end(); ++it)
					sum+=*it;



		}

	uint64_t avg = sum/QUERIES;
	long double ssum = 0;
	uint64_t q = 0;
	for(int i=0;i<tries.size();i++)
	{

			if(mode == ADAPT)
					v = &(tries[i]->falsepos);
			if(mode == LOOKUP)
					v = &(tries[i]->lookupt);
			if(mode == TRUNCATE)
					v =  &(tries[i]->truncatet);


			list<uint64_t>::iterator it;

			for(it=v->begin(); it != v->end(); ++it)
			{
				ssum+=(avg-(*it))*(avg-(*it));
				q++;
			}



	}
	/*for(int i=q;i<QUERIES;i++)
		ssum+=avg *avg;*/

	long double lsum = ssum;
	lsum = lsum/QUERIES;
	lsum = sqrt(lsum);
	uint64_t stddev = lsum;

	return stddev;
}


uint64_t MultiTrie::getLookupMedian()
{
	//long double sum = 0.0;
	uint64_t sum =0;
	int q = 0;
	list<uint64_t> *v = NULL;


	vector<uint64_t> data;

	for(int i=0;i<tries.size();i++)
	{

					v = &(tries[i]->lookupt);

			list<uint64_t>::iterator it;

			for(it=v->begin(); it != v->end(); ++it)
				data.push_back(*it);



	}
	sort(data.begin(),data.end());
	cout<<"Exp queries data:"<<data.size()<<endl;


	return data[data.size()*0.5]; //return median :)

}

void MultiTrie::truncateClockPartial(int target_size) /* truncate
each partition separately - this doesn't work out*/
{ //? how? what size will it be? I guess we'll have the sizes equally done for now


	double coeff =1;
	if(target_size != this->bits)
	{
		coeff = (0.0+target_size)/this->bits;

	}
	for(int i=0;i<partitions;i++)
	{

		sizes[i] = sizes[i] * coeff;

	}

	for(int i=0;i<partitions;i++)
	{
		//if(i==0)
		//cout<<"truncating trie to size "<<sizes[i]<<endl;
		//cout<<"size: "<<tries[i]->size()<<endl;
		tries[i]->truncateClock(sizes[i]);
		//cout<<"truncated size: "<<tries[i]->size()<<endl;
	}

	//merge all the partitions




}
void MultiTrie::getStats()
{


}

int MultiTrie::getFp()
{

/*int sum=0;

for(int i=0;i<tries.size();i++)
	sum+=tries[i]->fp;

return sum;*/
return fp;


}

bool MultiTrie::handleQuery(Query::Query_t q,Database * dbase, bool doAdapt)
{
	bool res = false; //this is the result the multitrie gives
	bool realRes = db->rangeQuery(q); //this is the real result
	list<uint64_t> fpp;
	list<uint64_t> lu;


	#ifdef DOTRACE
		stringstream ss;
		ss<<"MultiTrie queried: "<<q.left<<"-"<<q.right;
		tries[0]->snapshots = this->snapshots;
		tries[0]->takeTextSnapshot(ss.str());
		this->snapshots = tries[0]->snapshots;
	#endif


	//cout<<"Original query:"<<q.left<<"-"<<q.right<<endl;

	int tries_queried = 0;
	for(int i=0;i<partitions;i++)
	{
		if(q.left>q.right)
			break;
		if(q.left>=bounds[i].low && q.left<=bounds[i].high)
		{
			/* query me */

			Query::Query_t q2,q3;
			q2.left = q.left - bounds[i].low;
			q2.right = q.right - bounds[i].low;
			if(q2.right>tries[i]->actual_size)
				q2.right = tries[i]->actual_size;

			q3.right = q2.right + bounds[i].low;
			q3.left = q2.left + bounds[i].low;

			assert(bounds[i].low == tries[i]->lowerb);

			#ifdef DOTRACE
						tries[i]->snapshots = this->snapshots;
			#endif

			bool qR = db->rangeQuery(q3);

			bool sR = tries[i]->handleQuery(q2,db,doAdapt,qR);

			lu.push_front(tries[i]->lookupt.front());
			if(doAdapt && (!qR && sR))
				fpp.push_front(tries[i]->falsepos.front());

			//cout<<q3.left<<"-"<<q3.right<<endl;

			#ifdef DOTRACE
						this->snapshots = tries[i]->snapshots;
			#endif

			if(sR)
			{
				res = true;
				if(!doAdapt)
					break;
				//break; //don't, we want to adapt!
			}
			q.left = bounds[i].high+1;
			assert(q.left == tries[i]->lowerb+tries[i]->actual_size+1);


		}


	}




	if(res && !realRes)
	{
		fp++;
	}
	if(res && realRes)
		tp++;
	if(!res && !realRes)
		tn++;
#ifdef TICKTOCK
	list<uint64_t>::iterator i;
	uint64_t sum = 0;
	for(i=fpp.begin(); i != fpp.end(); ++i)
	{
		sum+=*i;
	}
	if(!fpp.empty())
		adaptt.push_front(sum);

	   sum = 0;
		for(i=lu.begin(); i != lu.end(); ++i)
		{
			sum+=*i;
		}
		if(!lu.empty())
			lookupt.push_front(sum);
		assert(!lu.empty());
#endif

	assert(!(!res && realRes));



	return res;
}


bool MultiTrie::handleQueryOld(Query::Query_t q,Database * dbase, bool doAdapt)
{
	bool DO_ADAPT =true;
	bool res = false; //this is the result the multitrie gives
	bool realRes = db->rangeQuery(q); //this is the real result

	Query::Query_t q1 = q;
	int tries_queried = 0;
	for(int i=0;i<partitions;i++)
	{

		//cout<<"Query:"<<i<<" " <<q.left<<"-"<<q.right<<endl;
		if(bounds[i].low>q.right)
			break;
		if(q.left>bounds[i].high)
			continue;

		if(q.left>=bounds[i].low)
		{
			tries_queried++;
			Query::Query_t q_adj,real;
			real.left = q.left;
			if(q.right>bounds[i].high)
				real.right = bounds[i].high;
			else
				real.right = q.right;

			q_adj.left =q.left - bounds[i].low;
			if(q.right>bounds[i].high)
				q_adj.right = bounds[i].high - bounds[i].low;
			else
				q_adj.right = q.right -bounds[i].low;

			assert(q_adj.right<=tries[i]->getMaxVal());

			//cout<<"adjusted Query:" <<i<<" "<<q_adj.left<<"-"<<q_adj.right<<endl;
			//cout<<"querying db:"<<real.left<<"-"<<real.right<<endl;
			bool tempReal = db->rangeQuery(real);
			//assert(real.right == q_adj.right && q_adj.left ==real.left);

			//cout<<"---TRIEMULTI---"<<endl;
			//if(i==9)
			//	cout<<"previous size:"<<tries[i]->Numleaves()<<endl;
			tries[i]->snapshots = this->snapshots;
			bool temp = tries[i]->handleQuery(q_adj,dbase,doAdapt,tempReal);
			this->snapshots = tries[i]->snapshots;

			//if(i==9)
			//	cout<<"next size:"<<tries[i]->Numleaves()<<endl;
			//cout<<"------------"<<endl;


			if(temp)
			{
				//here should we go further? maybe there are more tries that yield a fp
				res = true;
				//break;
			}

			q.left = bounds[i].high + 1;



		}
	}

	if(tries_queried>1)
	{
		multiple++;
		//cout<<"multiple tries: "<<q1.left<<"-"<<q1.right<<endl;
		//getchar();
	}

	if(res && !realRes)
	{
		fp++;
	}
	if(res && realRes)
		tp++;
	if(!res && !realRes)
		tn++;

	assert(!(!res && realRes));



	return res;
}


void MultiTrie::printFps()
{
	cout<<"tries size:"<<tries.size()<<endl;
	for(int i=0;i<tries.size();i++)
		cout<<"Trie part "<<i<<" fps: "<<tries[i]->fp<<" tps: "<<tries[i]->tp<<" tns: "<<tries[i]->tn<<"  size: "<<tries[i]->size()<<" vs (unif) "<<sizes[i]<<" leaves "<<tries[i]->Numleaves()<<endl;
	

	int sum=0;
	int siz = 0;
       for(int i=0;i<tries.size();i++)
       {
		sum+=tries[i]->Numleaves();
       	siz+=tries[i]->num_bits;
       }
        cout<<"---Total Leaves: "<<sum<<" -----"<<endl;
        cout<<"---Total Size: "<<siz<<" -----"<<endl;
        cout<<"Queries spanning multiple tries: "<<multiple<<endl;
	//tries.back()->exportGraphviz("last.txt");

}


void MultiTrie::partition_agnostic() //uniformly partitioned, each partition has the same size
{

		int sSize = bits/partitions;
		for(int i=0;i<partitions;i++)
			sizes.push_back(sSize);
		create_uniform_bounds();


}

void MultiTrie::create_uniform_bounds()
{
/* create the bounds*/
		int step = (domain+1)/partitions;


		for(int i=0;i<partitions;i++)
		{

			interval t;
			t.low = step * i;
			if(i<partitions -1)
				t.high = (step * (i+1)) -1;
			else
				t.high = domain;
			bounds.push_back(t);
		}


}

void MultiTrie::partition_data2() //the ranges are equi-depthy OR most dense regions have moar bits
{

	return; /*

		create_uniform_bounds();
		//for each boundary of a trie, see how many things there are in the cold store
		//adjust size accordingly
		vector<uint64> keys = db->getKeys();

		for(int i=0;i<bounds.size();i++)
		{
			int sSize = getNumInRange(0,bounds[i].low,bounds[i].high,keys);
			//cout<<"values within boundary"<<sSize<<endl;
			double percent = (sSize+0.0)/keys.size();
			//cout<<"percentage:"<<percent<<endl;
			int size = percent * this->bits;
			if(size<MINSIZE)
				size =MINSIZE;

			sizes.push_back(size);

		}*/
}


int MultiTrie::getNumInRange(int startidx,int low,int high,vector<uint> & container) //container is sorted
{
	int sum = 0;
	for(int i=startidx;i<container.size();i++)
	{
		if(container[i]>= low && container[i]<=high)
			sum++;
		if(container[i]>high)
			break;
	}
	return sum;
}

void MultiTrie::partition_data() //the ranges are equi-depthy OR most dense regions have moar bits
{

	int sSize = bits/partitions;
		for(int i=0;i<partitions;i++)
			sizes.push_back(sSize);



		int step = cold_records/partitions;
		vector<uint64> keys = db->getKeys();
		assert(keys.size() == cold_records);

		//sort(keys.begin(),keys.end());

		for(int i=0;i<partitions;i++)
		{

			interval t;
			t.low = keys[step * i];
			if(i<partitions -1)
				t.high = keys[(step * (i+1)) -1];
			else
				t.high = keys.back();
			bounds.push_back(t);
		}

		//assert(1 == 0);
}





void MultiTrie::partition_query() // most queried ranges should have moar bits or they are
{



}

MultiTrie::MultiTrie(int partitions,int PARTITION_MODE,int cold,int domain,int bits,Database * db,int repl_policy,int clock_bits)
{

	int LIMIT = -1;
	bool LEARN_FROM_TP = true;
	Database * DATABASE = NULL;
	//Database * DATABASE = db;
	tn = 0;
	tp = 0;
	fp = 0;
	victim_trie = 0;
	multiple = 0;
	lastMerged = -1;
	unused = 0;
	snapshots = 0;


	this->partitions = partitions;
	this->domain = domain;
	this->bits = bits;
	this->db = db;
	this->cold_records = cold;
	this->clock_bits = clock_bits;
	bounds.reserve(partitions);
	tries.reserve(partitions);
	sizes.reserve(partitions);

       /* initialize sizes */
	/*if(PARTITION_MODE == AGNOSTIC)
	{
		partition_agnostic();
	}
	if(PARTITION_MODE == DATAAWARE)
		partition_data2();*/

	partition_agnostic();
	/*create the tries*/

	for(int i=0;i<partitions;i++)
	{
		/* something fishy is going on here */
		interval t = bounds[i];
		int rangelen = bounds[i].high - bounds[i].low  ;
		int triedom = closestPower2(rangelen+1) - 1;
		//cout<<"domain of trie "<<i<<" with bounds:["<<bounds[i].low<<"-"<<bounds[i].high
		//<<"]="<<rangelen<<"  size in bits: "<<sizes[i]<<endl;

		//cout<<"domian: "<<triedom<<endl;
		unused+= triedom - rangelen;
		Synopsis * syn = NULL;

		if(clock_bits == 0)
		{
			//syn = new Synopsis0bit(triedom,sizes[i],LEARN_FROM_TP,DATABASE,bounds[i].low);
			syn = new Synopsis(triedom,sizes[i],LEARN_FROM_TP,DATABASE,CLOCK,bounds[i].low);


		}
		if(clock_bits == 1)
			syn = new Synopsis(triedom,sizes[i],LEARN_FROM_TP,DATABASE,CLOCK,bounds[i].low);
		if(clock_bits == 2)
			syn = new Synopsis2bit(triedom,sizes[i],LEARN_FROM_TP,DATABASE,bounds[i].low);

		if(clock_bits == -1) /*trie with weights */
		{
			syn = new SynopsisIntClock(triedom,sizes[i],LEARN_FROM_TP,DATABASE,bounds[i].low);
		}
		//Synopsis * syn = new Synopsis(triedom,sizes[i],LEARN_FROM_TP,DATABASE,CLOCK,bounds[i].low);
		syn->setUpperBound(bounds[i].high- bounds[i].low);
		syn->setDatabase(db);
		syn->id= i;
		syn->init();
		if(clock_bits>=0)
			syn->set(clock_bits);
		syn->outfolder="multi-walkthrough";
		tries.push_back(syn);
		//cout<<"Upper bound: "<<syn->actual_size+syn->lowerb<<endl;
		//very important: mark redundant range (due to power of 2) as empty
		//space overhead (yes, confirmed)
		//, but if this is not done, truncating does evil things
		//to our trie :( (because of merging) (possibly, i am not 100% sure
		//but the results are more erratic if you don't mark as empty
#ifdef MARKUNUSED
		vector<Synopsis::state> hist;
		vector<Synopsis::node> marked;
		Query::Query_t empty_part;
		empty_part.left = 1+ bounds[i].high - bounds[i].low;
		empty_part.right = triedom;
		//cout<<"mark empty:"<<empty_part.left<<"-"<<empty_part.right<<endl;
		syn->snapshots = this->snapshots;
		if(empty_part.left<=empty_part.right)
		{
			syn->learn_from_fp(empty_part);
		}
		this->snapshots = syn->snapshots;
#endif

	}

	//cout<<"Unused space:"<<this->unused<<endl;
	/*for(int i=0;i<tries.size();i++)
	      {
	    	  stringstream ss;
	    	  ss<<"start"<<i<<".txt";


	    	  tries[i]->exportGraphViz(ss.str());
	      }*/
}

MultiTrie::~MultiTrie()
{

}

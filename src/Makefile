FLAGS = -O3 

Simulator.out: Simulator \
    Database \
    Distribution \
    Uniform \
    Normal \
    Zipf \
    Synopsis \
    Bloom \
    Query \
    Curve \
    PointGen \
    Util
	g++ -O3 -o Experiment6.out Experiment6.o Database.o Distribution.o Uniform.o Zipf.o  Bloom.o Query.o Synopsis.o SynopsisW.o SynopsisClock.o Synopsis2bit.o SynopsisIntClock.o SynopsisDebug.o Synopsis0bit.o SynopsisDFS.o MultiTrie.o Curve.o Zcurve.o Util.o DatagenMD.o Normal.o SynopsisMD.o Ccurve.o Hcurve.o hilbert.o PointGen.o Shuffler.o Statistics.o FastSynopsis.o

Synopsis: Util.h Database.h Synopsis.h Synopsis/Synopsis.cpp Synopsis/SynopsisW.cpp Synopsis/SynopsisClock.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Synopsis/Synopsis.cpp Synopsis/SynopsisW.cpp Synopsis/SynopsisClock.cpp Synopsis/Synopsis2bit.cpp Synopsis/Synopsis0bit.cpp Synopsis/MultiTrie.cpp Synopsis/SynopsisIntClock.cpp Synopsis/SynopsisDebug.cpp Synopsis/SynopsisDFS.cpp Synopsis/SynopsisMD.cpp Synopsis/Statistics.cpp Synopsis/FastSynopsis.cpp

Curve: curves/Curve.h
	g++ -c  -Wno-sign-compare -O3  -std=c++0x curves/Curve.cpp curves/Zcurve.cpp curves/DatagenMD.cpp curves/Ccurve.cpp curves/Hcurve.cpp hilbert/hilbert.c

PointGen: Database/PointGen.h Database/PointGen.h
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Database/PointGen.cpp 

Util: Util.h Util/Util.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Util/Util.cpp 

Bloom: Util.h Database.h Bloom.h Bloom/Bloom.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Bloom/Bloom.cpp 


Simulator: Experiment6.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x  Experiment6.cpp

Database: Util.h Database.h Distribution.h Database/Database.cpp 
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Database/Database.cpp 

Uniform:  Util.h Distribution.h Distribution/Uniform.cpp Distribution/Distribution.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Distribution/Uniform.cpp Distribution/Distribution.cpp 

Zipf: Util.h Distribution.h Distribution/Zipf.cpp Distribution/Distribution.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Distribution/Zipf.cpp Distribution/Distribution.cpp 
Normal: Util.h Distribution.h Distribution/Normal.cpp Distribution/Distribution.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Distribution/Normal.cpp Distribution/Distribution.cpp 

Distribution: Util.h Distribution.h Distribution/Distribution.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Distribution/Distribution.cpp Distribution/Shuffler.cpp

Query: Util.h Distribution.h Query.h Query/Query.cpp
	g++ -c  -Wno-sign-compare -O3  -std=c++0x Query/Query.cpp 

clean:
	rm -f *.o Simulator.out




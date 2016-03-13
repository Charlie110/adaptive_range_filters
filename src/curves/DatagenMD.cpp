/*
 * DatagenMD.cpp
 *
 *  Created on: Nov 23, 2012
 *      Author: carolinux
 */

#include "DatagenMD.h"

DatagenMD::DatagenMD() {
	// TODO Auto-generated constructor stub

}

DatagenMD::~DatagenMD() {
	// TODO Auto-generated destructor stub
}








	void DatagenMD::InitStatistics(int Dimensions)
	// ==============
	// initialisiert ZÃ¤hlvariablen der Statistik
	{
	Statistics_SumX = new double[Dimensions];
	Statistics_SumXsquared = new double[Dimensions];
	Statistics_SumProduct = new double[Dimensions*Dimensions];

	Statistics_Count = 0;
	for (int d=0; d<Dimensions; d++) {
	Statistics_SumX[d]=0.0;
	Statistics_SumXsquared[d]=0.0;
	for (int dd=0; dd<Dimensions; dd++) Statistics_SumProduct[d*Dimensions+dd] = 0.0;
	}
	}


	void DatagenMD::EnterStatistics(int Dimensions,double* x)
	// ===============
	// registiriert den Vektor "x" fur die Statistik
	{
	Statistics_Count++;
	for (int d=0; d<Dimensions; d++) {
	Statistics_SumX[d] += x[d];
	Statistics_SumXsquared[d] += x[d]*x[d];
	for (int dd=0; dd<Dimensions; dd++) Statistics_SumProduct[d*Dimensions+dd] += x[d]*x[dd];
	}
	}


	void DatagenMD::OutputStatistics(int Dimensions)
	// ================
	// gibt die Statistik aus
	{
	for (int d=0; d<Dimensions; d++) {
	double E = Statistics_SumX[d] / Statistics_Count;
	double V = Statistics_SumXsquared[d]/Statistics_Count - E*E;
	double s = sqrt(V);
	printf("E[X%d]=%5.2f Var[X%d]=%5.2f s[X%d]=%5.2f\n",d+1,E,d+1,V,d+1,s);
	}
	printf("\nCorrelation Matrix:\n");
	for (int d=0; d<Dimensions; d++) {
	for (int dd=0; dd<Dimensions; dd++) {
	double Kov = (Statistics_SumProduct[d*Dimensions+dd]/Statistics_Count) -
	(Statistics_SumX[d]/Statistics_Count) * (Statistics_SumX[dd]/Statistics_Count);
	double Cor = Kov /
	sqrt(Statistics_SumXsquared[d]/Statistics_Count - sqr(Statistics_SumX[d] / Statistics_Count)) /
	sqrt(Statistics_SumXsquared[dd]/Statistics_Count - sqr(Statistics_SumX[dd] / Statistics_Count));
	printf(" %5.2f",Cor);
	}
	printf("\n");
	}
	printf("\n");
	}


	double DatagenMD::RandomEqual(double min,double max)
	// ===========
	// liefert eine im Intervall [min,max[ gleichverteilte Zufallszahl
	{
	double x = (double)rand()/MAXINT;
	return x*(max-min)+min;
	}


	double DatagenMD::RandomPeak(double min,double max,int dim)
	// ==========
	// liefert eine Zufallsvariable im Intervall [min,max[
	// als Summe von "dim" gleichverteilten Zufallszahlen
	{
	double sum = 0.0;
	for (int d=0; d<dim; d++) sum += RandomEqual(0,1);
	sum /= dim;
	return sum*(max-min)+min;
	}


	double DatagenMD::RandomNormal(double med,double var)
	// ============
	// liefert eine normalverteilte Zufallsvariable mit Erwartungswert med
	// im Intervall ]med-var,med+var[
	{
	return RandomPeak(med-var,med+var,12);
	}


	vector<vector<double> >  DatagenMD::GenerateDataEqually(FILE* f,int Count,int Dimensions)
	// ===================
	// generiert in der Datei "f" "Count" gleichverteilte Dateien
	{
	InitStatistics(Dimensions);
	vector<vector<double> > res;
	for (int i=0; i<Count; i++)
	{
	double x[Dimensions];
	res.push_back(vector<double>(Dimensions));
	for (int d=0; d<Dimensions; d++)
	{
	x[d] = RandomEqual(0,1);
	if(f!=NULL)
						fprintf(f,"%8.6f ",x[d]);
					res[i][d] = x[d];
	}
	EnterStatistics(Dimensions,x);
	if(f!=NULL)
		fprintf(f,"\n");
	}
	OutputStatistics(Dimensions);
		return res;
	}


	vector<vector<double> >  DatagenMD::GenerateDataCorrelated(FILE* f,int Count,int Dimensions)
	// ======================
	// generiert in der Datei "f" "Count" korrelierte DatensÃ¤tze
	{
	InitStatistics(Dimensions);
	double x[Dimensions];
	vector<vector<double> > res;
	for (int i=0; i<Count; i++) {
		res.push_back(vector<double>(Dimensions));
	again:
	double v = RandomPeak(0,1,Dimensions);
	for (int d=0; d<Dimensions; d++) x[d] = v;
	double l = v<=0.5 ? v:1.0-v;
	for (int d=0; d<Dimensions; d++) {
	double h = RandomNormal(0,l);
	x[d] += h;
	x[(d+1)%Dimensions] -= h;
	}
	for (int d=0; d<Dimensions; d++) if (x[d]<0 || x[d]>=1) goto again;
	for (int d=0; d<Dimensions; d++)
		{
		if(f!=NULL)
					fprintf(f,"%8.6f ",x[d]);
				res[i][d] = x[d];

		}
	EnterStatistics(Dimensions,x);
	if(f!=NULL)
		fprintf(f,"\n");
	}
	OutputStatistics(Dimensions);
	return res;
	}


	vector<vector<double> > DatagenMD::GenerateDataAnticorrelated(FILE* f,int Count,int Dimensions)
	// ==========================
	// generiert in der Datei "f" "Count" antikorrelierte DatensÃ¤tze
	{
	InitStatistics(Dimensions);
	double x[Dimensions];
	vector<vector<double> > res;
	for (int i=0; i<Count; i++) {
		res.push_back(vector<double>(Dimensions));
	again:
	double v = RandomNormal(0.5,0.25);
	for (int d=0; d<Dimensions; d++) x[d] = v;
	double l = v<=0.5 ? v:1.0-v;
	for (int d=0; d<Dimensions; d++) {
	double h = RandomEqual(-l,l);
	x[d] += h;
	x[(d+1)%Dimensions] -= h;
	}
	for (int d=0; d<Dimensions; d++) if (x[d]<0 || x[d]>=1) goto again;
	for (int d=0; d<Dimensions; d++)
	{
		if(f!=NULL)
			fprintf(f,"%8.6f ",x[d]);
		res[i][d] = x[d];
	}
	EnterStatistics(Dimensions,x);
	if(f!=NULL)
		fprintf(f,"\n");
	}
	//OutputStatistics(Dimensions);

	cout<<"------lalalal------------"<<endl;
	return res;
	}


	void DatagenMD::GenerateData(int Dimensions,char Distribution,int Count,char* FileName)
	// ============
	// generates fiel
	{
	if (Count <= 0) {
	printf("invalid number of points.\n");
	return;
	}
	if (Dimensions < 2) {
	printf("invalid number of dimensions.\n");
	return;
	}
	switch (Distribution) {
	case 'E':
	case 'e': Distribution = 'E'; break;
	case 'C':
	case 'c': Distribution = 'C'; break;
	case 'A':
	case 'a': Distribution = 'A'; break;
	default: printf("invalid correlation status.\n"); return;
	}

	FILE* f = NULL;
	if(FileName!=NULL)
		f = fopen(FileName,"w");

	if (f == NULL) {
		printf("Can't open file \"%s\".\n",FileName);
	//return;
	}
	fprintf(f,"%d %d\n",Count,Dimensions);
	switch (Distribution) {
	case 'E': GenerateDataEqually(f,Count,Dimensions); break;
	case 'C': GenerateDataCorrelated(f,Count,Dimensions); break;
	case 'A': GenerateDataAnticorrelated(f,Count,Dimensions); break;
	}
	if(f!=NULL)
	{
		fclose(f);
		printf("%d Points generated, saved in file \"%s\".\n",Count,FileName);
	}
	}

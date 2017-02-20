#ifndef _GENALG_H
#define _GENALG_H

#include "RainbowIslands\Commons_RainbowIslands.h"
#include "RainbowIslands\Constants_RainbowIslands.h"
#include <vector>
#include <algorithm>
using namespace std;

class GenAlg
{
public:
	GenAlg(int popSize, double mutRate, double crossRate, int numWeights);

	vector<Genome> Epoch(vector<Genome> &old_pop);

	vector<Genome> GetChromos() const { return mPop; }
	void SetChromos(vector<Genome> startPop) { mPop = startPop; }
	double AverageFitness() const { return mTotalFitness / mPopSize; }
	double BestFitness() const { return mBestFitness; }

private:
	//entire population of chromosomes
	vector<Genome> mPop;

	//size of population
	int mPopSize;

	//amount of weights per chromosome
	int mChromoLength;

	//total fitness of population
	double mTotalFitness;

	//best in this population
	double mBestFitness;

	double mAverageFitness;

	double mWorstFitness;

	//tracking the current fittest
	int mFittestGenome;

	//prob chance bits in a chromosome will mutate, 0.05 to 0.3 usually
	double mMutationRate;

	//prob chance of chromosome crossing over bits, try 0.7
	double mCrossoverRate;

	//counter for generations
	int mGeneration;

	void Crossover(const vector<double> &mum,
		const vector<double> &dad,
		vector<double> &baby1,
		vector<double> &baby2);

	void Mutate(vector<double> &chromo);

	Genome GetChromoRoulette();

	//introducing elitism
	void GrabNBest(int nBest, const int numCopies, vector<Genome> &population);

	void CalculateBestWorstAvTot();

	void Reset();
};

#endif // !_GENALG_H

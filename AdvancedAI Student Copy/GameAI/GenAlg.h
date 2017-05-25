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
	GenAlg(int popSize, int numWeights);

	vector<Genome> Epoch(vector<Genome> &old_pop);

	vector<Genome> GetChromos() const { return mPop; }
	void SetChromos(vector<Genome> startPop) { mPop = startPop; }
	double AverageFitness() const { return mTotalFitness / mPopSize; }
	double BestFitness() const { return mBestFitness; }

private:
	vector<Genome> mPop;
	int mPopSize;
	int mChromoLength;

	double mTotalFitness;
	double mBestFitness;
	double mAverageFitness;
	double mWorstFitness;

	int mFittestGenome;

	int mGeneration;

	void Crossover(const vector<double> &mum,
		const vector<double> &dad,
		vector<double> &baby1,
		vector<double> &baby2);

	void Mutate(vector<double> &chromo);

	Genome GetChromoRoulette();
	void GrabNBest(int nBest, const int numCopies, vector<Genome> &population);
	void CalculateBestWorstAvTot();
	void Reset();
	void InitPopWithRandomChromos();
};

#endif // !_GENALG_H

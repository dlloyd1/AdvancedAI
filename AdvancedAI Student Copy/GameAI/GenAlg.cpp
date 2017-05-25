#include "GenAlg.h"

/*Credit Matt Buckland, AI Techniques for Game Programming*/

GenAlg::GenAlg(int popSize, int numWeights)
{
	mPopSize = popSize;
	mChromoLength = numWeights;

	mTotalFitness = 0;
	mGeneration = 0;
	mFittestGenome = 0;
	mBestFitness = 0;
	mWorstFitness = 99999999;
	mAverageFitness = 0;

	InitPopWithRandomChromos();
}

void GenAlg::InitPopWithRandomChromos()
{
	//init pop with chromosomes of random weights and all fitness initiatilised to 0
	for (int i = 0; i < mPopSize; ++i)
	{
		mPop.push_back(Genome());
		for (int j = 0; j < mChromoLength; ++j)
		{
			//float random = (-1.0f + 1) * ((((float)rand()) / (float)RAND_MAX)) + -1.0f;
			float random = RandomNumber(0.0f, 1.0f);
			mPop[i].mWeights.push_back(random);
		}
	}
}

//run a population of chromosomes through one cycle
//returns a new population
vector<Genome> GenAlg::Epoch(vector<Genome>& old_pop)
{
	mPop = old_pop;
	Reset();

	sort(mPop.begin(), mPop.end());
	CalculateBestWorstAvTot();

	vector<Genome> newPop;
	if (!(kNumCopiesElite * kNumElite % 2))
	{
		GrabNBest(kNumElite, kNumCopiesElite, newPop);
	}

	//GA Loop
	while (newPop.size() < mPopSize)
	{
		Genome mum = GetChromoRoulette();
		Genome dad = GetChromoRoulette();

		vector<double> baby1;
		vector<double> baby2;

		Crossover(mum.mWeights, dad.mWeights, baby1, baby2);

		Mutate(baby1);
		Mutate(baby2);

		newPop.push_back(Genome(baby1, 0));
		newPop.push_back(Genome(baby2, 0));

	}
	mPop = newPop;

	return mPop;
}

void GenAlg::Crossover(const vector<double>& mum, const vector<double>& dad, vector<double>& baby1, vector<double>& baby2)
{
	float random = RandomNumber(0.0f, 1.0f);

	if (random > kRainbowCrossoverRate || (mum == dad))
	{
		baby1 = mum;
		baby2 = dad;
		return;
	}

	int randomInt = RandomInt(0, mChromoLength - 1);
	int crossoverPoint = randomInt;

	for (int i = 0; i < crossoverPoint; ++i)
	{
		baby1.push_back(mum[i]);
		baby2.push_back(dad[i]);
	}
	for (int j = crossoverPoint; j < mum.size(); ++j)
	{
		baby1.push_back(dad[j]);
		baby2.push_back(mum[j]);
	}
}

void GenAlg::Mutate(vector<double>& chromo)
{
	for (int i = 0; i < chromo.size(); ++i)
	{
		float random = RandomNumber(0.0f, 1.0f);
		if (random < kRainbowMutationRate)
		{
			float random = RandomNumber(0.0f, 1.0f);
			chromo[i] += random * kMaxPerturbation;
		}
	}
}

Genome GenAlg::GetChromoRoulette()
{
	float random = RandomNumber(0.0f, 1.0f);
	double slice = (double)random * mTotalFitness;

	Genome chosenGenome;
	double fitnessSoFar = 0;

	for (int i = 0; i < mPopSize; ++i)
	{
		fitnessSoFar += mPop[i].mFitness;
		if(fitnessSoFar >= slice)
		{
			chosenGenome = mPop[i];
			break;
		}
	}
	return chosenGenome;
}

void GenAlg::GrabNBest(int nBest, const int numCopies, vector<Genome>& population)
{
	while (nBest--)
	{
		for (int i = 0; i < numCopies; ++i)
		{
			population.push_back(mPop[(mPopSize - 1) - nBest]);
		}
	}
}

void GenAlg::CalculateBestWorstAvTot()
{
	mTotalFitness = 0;

	double highestSoFar = 0;
	double lowestSoFar = 99999999;

	for (int i = 0; i < mPopSize; ++i)
	{
		if (mPop[i].mFitness > highestSoFar)
		{
			highestSoFar = mPop[i].mFitness;
			mFittestGenome = i;
			mBestFitness = highestSoFar;
		}

		if (mPop[i].mFitness < lowestSoFar)
		{
			lowestSoFar = mPop[i].mFitness;
			mWorstFitness = lowestSoFar;
		}

		mTotalFitness += mPop[i].mFitness;
	}
	mAverageFitness = mTotalFitness / mPopSize;
}

void GenAlg::Reset()
{
	mTotalFitness = 0;
	mBestFitness = 0;
	mWorstFitness = 99999999;
	mAverageFitness = 0;
}

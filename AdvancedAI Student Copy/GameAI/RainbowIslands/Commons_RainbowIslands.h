//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------
#pragma once

#include <vector>
#include <iostream>
#include <random>

using namespace std;
//
//random_device rd;
//mt19937 rng(rd);

static int kNumCopiesElite = 1;
static int kNumElite = 4;

static float RandomNumber(float Min, float Max)
{

	//uniform_int_distribution<int> gen((int)Min, (int)Max);

	//return (float)gen(rng);

	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

static int RandomInt(int Min, int Max)
{
	/*uniform_int_distribution<int> gen(Min, Max);
	return gen(rng);*/

	return (rand() / RAND_MAX) * (Max - Min) + Min;
}

enum FACING
{
	FACING_LEFT = 0,
	FACING_RIGHT
};

enum BADDIE_TYPE
{
	BADDIE_NORMAL = 0,
	BADDIE_ANGRY
};

enum CHARACTER_STATE
{
	CHARACTERSTATE_NONE,
	CHARACTERSTATE_WALK,
	CHARACTERSTATE_JUMP,
	CHARACTERSTATE_PLAYER_DEATH
};

enum FRUIT_TYPE
{
	FRUIT_EGGPLANT,
	FRUIT_CHOCOLATE,
	FRUIT_EGG,
	FRUIT_BURGER,
	FRUIT_ICECREAM,
	FRUIT_MELON,
	FRUIT_CAKE,
	FRUIT_APPLE,
	FRUIT_FLOWER,
	FRUIT_CORN,

	FRUIT_MAX
};



struct Neuron
{
	int mNumInputs;
	vector<double> mWeights;

	Neuron(int numInputs)
	{	
		//one extra to account for bias
		mNumInputs = numInputs + 1;

		for (int i = 0; i < mNumInputs; ++i)
		{
			//setup weights with initial random vals between -1 and 1
			//Extra weight to represent threshold/bias
			//float random = (float((rand()) / float(RAND_MAX)) * (1.0f - -1.0f)) + -1.0f;
			float random = RandomNumber(-1.0f, 1.0f);
			mWeights.push_back((double)random);
		}
	}
};

struct NeuronLayer
{
	int mNumNeurons;

	vector<Neuron> mNeurons;

	NeuronLayer(int numNeurons, int numInputsPerNeuron)
	{
		mNumNeurons = numNeurons;
		for (int i = 0; i < numNeurons; ++i)
		{
			mNeurons.push_back(Neuron(numInputsPerNeuron));
		}
	}
};

struct Genome
{
	vector<double> mWeights;
	double mFitness;

	Genome() :mFitness(0) {}
	Genome(vector<double> w, double f) : mWeights(w), mFitness(f) {}

	//overloaded less than for sorting
	friend bool operator < (const Genome& lhs, const Genome& rhs)
	{
		return (lhs.mFitness < rhs.mFitness);
	}
};

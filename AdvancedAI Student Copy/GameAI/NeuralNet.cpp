#include "NeuralNet.h"

NeuralNet::NeuralNet()
{
	mNumInputs = kNumNNInputs;
	mNumOutputs = kNumNNOutputs;
	mNumHiddenLayers = kNumNNHiddenLayers;
	mNeuronsPerHiddenLayer = kNumNNNeuronsPerHiddenLayer;

	CreateNet();
	GenerateRandomWeights();
}

void NeuralNet::CreateNet()
{
	//create layers of network
	if (mNumHiddenLayers > 0)
	{
		//hidden layer 1
		mLayers.push_back(NeuronLayer(mNeuronsPerHiddenLayer, mNumInputs));

		for (int i = 0; i < mNumHiddenLayers - 1; ++i)
		{
			mLayers.push_back(NeuronLayer(mNeuronsPerHiddenLayer, mNeuronsPerHiddenLayer));
		}

		//output layer
		mLayers.push_back(NeuronLayer(mNumOutputs, mNeuronsPerHiddenLayer));
	}
	else
	{
		//just output layer
		mLayers.push_back(NeuronLayer(mNumOutputs, mNumInputs));
	}
}

vector<double> NeuralNet::GetWeights() const
{
	vector<double> allWeights;

	//for each neuron in each layer, store it's weights
	for (int i = 0; i < mNumHiddenLayers; ++i)
	{
		for (int j = 0; j < mLayers[i].mNumNeurons; ++j)
		{
			for (int k = 0; k < mLayers[i].mNeurons[j].mNumInputs; ++k)
			{
				allWeights.push_back(mLayers[i].mNeurons[j].mWeights[k]);
			}
		}
	}
	return allWeights;
}

int NeuralNet::GetNumberOfWeights() const
{
	int count = 0;
	for (int i = 0; i < mNumHiddenLayers; ++i)
	{
		for (int j = 0; j < mLayers[i].mNumNeurons; ++j)
		{
			for (int k = 0; k < mLayers[i].mNeurons[j].mNumInputs; ++k)
			{
				count++;
			}
		}
	}
	return count;
}

void NeuralNet::PutWeights(vector<double>& weights)
{
	int weight = 0;

	for (int i = 0; i < mNumHiddenLayers; ++i)
	{
		for (int j = 0; j < mLayers[i].mNumNeurons; ++j)
		{
			for (int k = 0; k < mLayers[i].mNeurons[j].mNumInputs; ++k)
			{
				mLayers[i].mNeurons[j].mWeights[k] = weights[weight++];
			}
		}
	}
	return;
}


inline double NeuralNet::Sigmoid(double activation, double response)
{
	return (1 / (1 + exp(-activation / response)));
}

vector<double> NeuralNet::Update(vector<double>& inputs)
{
	//resultant outputs from each layer
	vector<double> outputs;

	int weight = 0;
	
	if (inputs.size() != mNumInputs)
	{
		return outputs;
	}

	//for each layer
	for (int i = 0; i < mNumHiddenLayers + 1; ++i)
	{
		if (i > 0)
		{
			inputs = outputs;
		}
		outputs.clear();
		weight = 0;
		//for each neuron sum the inputs*respective weights. Throw the total at the sigmoid func for output
		for (int j = 0; j < mLayers[i].mNumNeurons; ++j)
		{
			double netInput = 0;
			int numInputs = mLayers[i].mNeurons[j].mNumInputs;
			//for each weight
			for (int k = 0; k < mNumInputs - 1; ++k)
			{
				//weight * its respective input
				netInput += mLayers[i].mNeurons[j].mWeights[k] * inputs[weight++];
			}
			//add in the bias
			netInput += mLayers[i].mNeurons[j].mWeights[numInputs - 1] * kBias;
			//store outputs from each layer as we gen them
			//combined activation is first filtered through sigmoid func
			outputs.push_back(Sigmoid(netInput, kActivationResponse));
			weight = 0;
		}
	}
	return outputs;
}

void NeuralNet::GenerateRandomWeights()
{
	int numWeights = GetNumberOfWeights();
	vector<double> newRandomWeights;
	for (int i = 0; i < numWeights; ++i)
	{
		newRandomWeights.push_back((double)RandomNumber(-1.0f, 1.0f));
	}
	PutWeights(newRandomWeights);
}

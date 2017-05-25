#include "NeuralNet.h"

/*Credit Matt Buckland, AI Techniques for Game Programming*/

NeuralNet::NeuralNet()
{
	CreateNet();
	GenerateRandomWeights();
}

void NeuralNet::CreateNet()
{
	//create layers of network
	if (kNumNNHiddenLayers > 0)
	{
		//hidden layer 1
		mLayers.push_back(NeuronLayer(kNumNNNeuronsPerHiddenLayer, kNumNNInputs));

		for (int i = 0; i < kNumNNHiddenLayers - 1; ++i)
		{
			mLayers.push_back(NeuronLayer(kNumNNNeuronsPerHiddenLayer, kNumNNNeuronsPerHiddenLayer));
		}

		//output layer
		mLayers.push_back(NeuronLayer(kNumNNOutputs, kNumNNNeuronsPerHiddenLayer));
	}
	else
	{
		//just output layer
		mLayers.push_back(NeuronLayer(kNumNNOutputs, kNumNNInputs));
	}
}

vector<double> NeuralNet::GetWeights() const
{
	vector<double> allWeights;

	//for each neuron in each layer, store it's weights
	for (int i = 0; i < kNumNNHiddenLayers + 1; ++i)
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

vector<double> NeuralNet::GetHiddenLayerWeights() const
{
	vector<double> hiddenLayerWeights;

	for (int j = 0; j < mLayers[0].mNumNeurons; ++j)
	{
		for (int k = 0; k < mLayers[0].mNeurons[j].mNumInputs; ++k)
		{
			hiddenLayerWeights.push_back(mLayers[0].mNeurons[j].mWeights[k]);
		}
	}

	return hiddenLayerWeights;
}

void NeuralNet::PutHiddenLayerWeights(vector<double>& weights)
{
	int weight = 0;

	for (int j = 0; j < mLayers[0].mNumNeurons; ++j)
	{
		for (int k = 0; k < mLayers[0].mNeurons[j].mNumInputs; ++k)
		{
			mLayers[0].mNeurons[j].mWeights[k] = weights[weight++];
		}
	}
}

vector<double> NeuralNet::GetOutputLayerWeights() const
{
	vector<double> outputLayerWeights;

	for (int j = 0; j < mLayers[1].mNumNeurons; ++j)
	{
		for (int k = 0; k < mLayers[1].mNeurons[j].mNumInputs; ++k)
		{
			outputLayerWeights.push_back(mLayers[1].mNeurons[j].mWeights[k]);
		}
	}
	return outputLayerWeights;
}

void NeuralNet::PutOutputLayerWeights(vector<double>& weights)
{}

int NeuralNet::GetNumberOfWeights() const
{
	int count = 0;
	for (int i = 0; i < kNumNNHiddenLayers + 1; ++i)
	{
		for (int j = 0; j < mLayers[i].mNumNeurons; ++j)
		{
			for (int k = 0; k < mLayers[i].mNeurons[j].mNumInputs; k++)
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

	for (int i = 0; i < kNumNNHiddenLayers + 1; ++i)
	{
		for (int j = 0; j < mLayers[i].mNumNeurons; ++j)
		{
			for (int k = 0; k < mLayers[i].mNeurons[j].mNumInputs; k++)
			{
				mLayers[i].mNeurons[j].mWeights[k] = weights[weight];
				weight++;
			}
		}
	}
}

vector<Neuron> NeuralNet::GetHiddenLayerNeurons() const
{
	vector<Neuron> relevantNeurons;

	for (int j = 0; j < mLayers[0].mNumNeurons; ++j)
	{
		relevantNeurons.push_back(mLayers[0].mNeurons[j]);
	}

	return relevantNeurons;
}

vector<double> NeuralNet::GetHiddenLayerNeuronActivations() const
{
	vector<double> activations;

	for(int j = 0; j < mLayers[0].mNumNeurons; ++j)
	{
		activations.push_back(mLayers[0].mNeurons[j].mActivation);
	}

	return activations;
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
	
	if (inputs.size() != kNumNNInputs)
	{
		return outputs;
	}

	//for each layer
	for (int i = 0; i < kNumNNHiddenLayers + 1; i++)
	{
		if (i > 0)
		{
			inputs = outputs;
		}
		outputs.clear();
		weight = 0;
		//for each neuron sum the inputs*respective weights. Throw the total at the sigmoid func for output
		for (int j = 0; j < mLayers[i].mNumNeurons; j++)
		{
			double netInput = 0;
			int numInputs = mLayers[i].mNeurons[j].mNumInputs;
			//for each weight
			for (int k = 0; k < numInputs - 1; ++k)
			{
				//weight * its respective input
				netInput += mLayers[i].mNeurons[j].mWeights[k] * inputs[weight++];
			}
			//add in the bias
			netInput += mLayers[i].mNeurons[j].mWeights[numInputs - 1] * kBias;
			//store outputs from each layer as we gen them
			//combined activation is first filtered through sigmoid func
			//mLayers[i].mNeurons[j].mActivation = Sigmoid(netInput, kActivationResponse);

			outputs.push_back(Sigmoid(netInput, kActivationResponse)); //output for every neuron
			weight = 0;
		}
	}
	return outputs; //last layer is the outputs interested in
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

bool NeuralNet::NetworkTrainingEpoch(double score, vector<double> inputs, vector<double> outputs, double maxScore)
{
	vector<double>::iterator curWeight;
	vector<Neuron>::iterator curNrnOut, curNrnHid;

	mErrorSum = 0;


	if (outputs.size() == 0)
	{
		return false;
	}

	//for each output neuron, calc the error and adjust weights
	for (int op = 0; op > kNumNNOutputs; ++op)
	{
		//calc error
		double err = maxScore - score;

		//update error total
		mErrorSum += err;

		mLayers[1].mNeurons[op].error = err;

		curWeight = mLayers[1].mNeurons[op].mWeights.begin();
		curNrnHid = mLayers[0].mNeurons.begin();

		//for each weight up to but not including the bias
		while (curWeight != mLayers[1].mNeurons[op].mWeights.end() - 1)
		{
			*curWeight += err * kLearningRate * curNrnHid->mActivation;
			++curWeight;
			++curNrnHid;
		}

		*curWeight += err * kLearningRate * kBias;
	}
	curNrnHid = mLayers[0].mNeurons.begin();

	int n = 0;

	//for each hidden layer nueron calc the error signal and then adjust weights accordingly
	while (curNrnHid != mLayers[0].mNeurons.end())
	{
		double err = 0;
		curNrnOut = mLayers[1].mNeurons.begin();

		//to calc the error for this neuron we need to iterate through
		//all the neurons in the output layer it is connected to and sum the error * weights
		while (curNrnOut != mLayers[1].mNeurons.end())
		{
			err += curNrnOut->error * curNrnOut->mWeights[n];
			++curNrnOut;
		}

		//now calc the error
		err *= curNrnHid->mActivation * (1 - curNrnHid->mActivation);

		//for each weight in this neuron calc the new weight based on the error signal and the learning rate
		for (int w = 0; w < inputs.size(); ++w) //@TODO: local param inputs instead?
		{
			//calc the new weight based on the backprop rules
			curNrnHid->mWeights[w] += err * kLearningRate * inputs[w];
		}
		
		curNrnHid->mWeights[kNumNNInputs] += err * kLearningRate * kBias;
		++curNrnHid;
		++n;
		
	}//next input vector
	return true;
}

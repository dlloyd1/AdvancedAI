#ifndef _NEURALNET_H
#define _NEURALNET_H

#include "RainbowIslands\Commons_RainbowIslands.h"
#include "RainbowIslands\Constants_RainbowIslands.h"
#include <vector>
#include "Commons.h"

using namespace std;

class NeuralNet
{

public:
	NeuralNet();
	void CreateNet();


	vector<double> GetWeights()const;
	int GetNumberOfWeights()const;

	//replaces existing weights with new ones, as part of the GA
	void PutWeights(vector<double> &weights);

	//sigmoid response curve
	inline double Sigmoid(double activation, double response);

	//calculate the outputs from a set of inputs
	vector<double> Update(vector<double> &inputs);

	
	vector<double> GetHiddenLayerWeights() const;
	void PutHiddenLayerWeights(vector<double>& weights);

	vector<Neuron> GetHiddenLayerNeurons() const;
	vector<double> GetHiddenLayerNeuronActivations() const;

	void GenerateRandomWeights();
	vector<double> GetOutputLayerWeights() const;
	void PutOutputLayerWeights(vector<double>& weights);

	bool NetworkTrainingEpoch(double score, vector<double> inputs, vector<double> outputs, double maxScore);


private:

	vector<NeuronLayer> mLayers;
	double mErrorSum;
	bool mTrained;
	int mNumEpochs;
};



#endif // _NEURALNET_H


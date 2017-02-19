//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef _CHARACTERBUB_H
#define _CHARACTERBUB_H

#include "Character.h"
#include <iostream>
#include "..\NeuralNet.h"

using namespace::std;

class CharacterBub : public Character
{

//--------------------------------------------------------------------------------------------------
public:
	CharacterBub(SDL_Renderer* renderer, string imagePath, LevelMap* map, Vector2D startPosition, NeuralNet neuralNetwork);
	~CharacterBub();

	void AlternateCharacterUpdate(float deltaTime, SDL_Event e);  //Required as it is very similar to original, but with RAINBOW tweaks.
	void Update(float deltaTime, SDL_Event e);
	void Render();

	void MoveLeft(float deltaTime, int yCentreOnImage);
	void MoveRight(float deltaTime, int yCentreOnImage);
	void Jump();

	void SetState(CHARACTER_STATE newState);

	bool OnARainbow()						{ return mOnRainbow; }
	void SetOnARainbow(bool yesNo)			{ mOnRainbow = yesNo; }

	void AddPoints()						{ mPoints += 10; }
	int  GetPoints()						{ return mPoints; }

	void SetTimeAlive(float val) { mTimeAlive = val; }
	float GetTimeAlive() { return mTimeAlive; }

	int	GetRainbowsAllowed();

	void PutWeights(vector<double> &w) { mItsBrain.PutWeights(w); }
	int GetNumberOfWeights()const { return mItsBrain.GetNumberOfWeights(); }

	NeuralNet GetBrain() { return mItsBrain; }

//--------------------------------------------------------------------------------------------------
private:
	int		mStartFrame;
	int		mEndFrame;
	int		mCurrentFrame;
	float	mFrameDelay;

	bool	mOnRainbow;
	int		mPoints;

	NeuralNet mItsBrain;
	double mFitness;

	vector<double> mInputs;
	vector<double> mOutputs;

	float mTimeAlive;

};
#endif //_CHARACTERBUB_H
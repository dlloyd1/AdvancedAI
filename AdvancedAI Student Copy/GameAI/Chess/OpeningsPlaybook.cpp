#include "OpeningsPlaybook.h"
#include "ChessCommons.h"
#include "../Commons.h"
#include <iostream>
#include <string>

using namespace std;

OpeningsPlaybook* OpeningsPlaybook::mInstance = NULL;

OpeningsPlaybook::OpeningsPlaybook()
{
	mChosenOpeningIndex = 0;
	mCurrentMoveNumber = 0;

	populateSpanishOpening();
	populateFrenchDefenseOpening();
	populateKingsGambitOpening();

	//store all three openings, to choose a random one
	mAllOpenings.push_back(mSpanishOpening);
	mAllOpenings.push_back(mFrenchDefense);
	mAllOpenings.push_back(mKingsGambit);
}


OpeningsPlaybook::~OpeningsPlaybook()
{
	mInstance = NULL;
}

OpeningsPlaybook* OpeningsPlaybook::Instance()
{
	if (!mInstance)
	{
		mInstance = new OpeningsPlaybook;
	}

	return mInstance;
}

void OpeningsPlaybook::IncrementCurrentMoveNumber()
{
	mCurrentMoveNumber++;
}

int OpeningsPlaybook::GetCurrentMoveNumber()
{
	return mCurrentMoveNumber;
}

void OpeningsPlaybook::SelectAnOpening()
{
	mChosenOpeningIndex = rand() % 3;
	mSelectedOpening = mAllOpenings.at(mChosenOpeningIndex);
}

void OpeningsPlaybook::populateSpanishOpening()
{
	Move black1(Vector2D(4, 1), Vector2D(4, 3));
	mSpanishOpening.push_back(black1);
	Move black2(Vector2D(1, 0), Vector2D(2, 2));
	mSpanishOpening.push_back(black2);
	Move black3(Vector2D(0, 1), Vector2D(0, 2));
	mSpanishOpening.push_back(black3);
}

void OpeningsPlaybook::populateFrenchDefenseOpening()
{
	Move black1(Vector2D(4, 1), Vector2D(4, 2));
	mFrenchDefense.push_back(black1);
	Move black2(Vector2D(3, 1), Vector2D(3, 3));
	mFrenchDefense.push_back(black2);
	Move black3(Vector2D(2, 1), Vector2D(2, 3));
	mFrenchDefense.push_back(black3);
}


void OpeningsPlaybook::populateKingsGambitOpening()
{
	Move black1(Vector2D(4, 1), Vector2D(4, 3));
	mKingsGambit.push_back(black1);
	Move black2(Vector2D(3, 1), Vector2D(3, 3));
	mKingsGambit.push_back(black2);
	Move black3(Vector2D(2, 0), Vector2D(6, 4));
	mKingsGambit.push_back(black3);
}


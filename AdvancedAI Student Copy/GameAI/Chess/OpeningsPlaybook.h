//include guard
#ifndef _OPENINGS_PLAYBOOK_H 
#define _OPENINGS_PLAYBOOK_H

#include "ChessCommons.h"
#include "../Commons.h"
#include <vector>
using namespace std;

//--------------------------------------------------------------------------------------------------

class OpeningsPlaybook
{
	//--------------------------------------------------------------------------------------------------
public:
	~OpeningsPlaybook();

	static OpeningsPlaybook* Instance();

	void SelectAnOpening();
	void IncrementCurrentMoveNumber();
	int GetCurrentMoveNumber();

	vector<Move> mSelectedOpening;

	//--------------------------------------------------------------------------------------------------
private:
	OpeningsPlaybook();

	//--------------------------------------------------------------------------------------------------
private:
	static OpeningsPlaybook* mInstance;
	
	vector<vector<Move>> mAllOpenings;

	//randomly chosen index of moves to attempt from playbook.
	int mChosenOpeningIndex;
	int mCurrentMoveNumber;

	vector<Move> mSpanishOpening;
	vector<Move> mFrenchDefense;
	vector<Move> mKingsGambit;


	void populateSpanishOpening();
	void populateFrenchDefenseOpening();
	void populateKingsGambitOpening();
};

//--------------------------------------------------------------------------------------------------

#endif //_OPENINGS_PLAYBOOK_H


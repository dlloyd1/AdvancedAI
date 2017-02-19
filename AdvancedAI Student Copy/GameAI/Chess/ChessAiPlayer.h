#ifndef _CHESSAIPLAYER_H
#define _CHESSAIPLAYER_H

#pragma once
#include "ChessPlayer.h"

class ChessAiPlayer :
	public ChessPlayer
{
public:
	ChessAiPlayer(SDL_Renderer* renderer, COLOUR colour, Board* board, vector<Vector2D>* highlights, Vector2D* selectedPiecePosition, Move* lastMove, int* searchDepth);
	~ChessAiPlayer();

	GAMESTATE PreTurn();
	bool TakeATurn(SDL_Event e);
	void EndTurn();
protected:
	void MakeAMove(Move moveToTake);
	Board CreateBoardStateFromMove(Board currentBoard, Move theMove);
	void OutputMove(Move move);
	void GetAttacksForPiece(Board boardToTest, vector<Move>* movesForThisPiece, vector<Move>* attackMoves, COLOUR enemyColour);
	int GetValOfBestAttack(Board boardToTest, vector<Move>* attackMoves);
	void OutputBoardState(Board board);

	int Maximum(Board board, int depth, int parentLow);
	int Minimum(Board board, int depth, int parentHigh);
	int ScoreBoard(Board board);
	int ScoreBoardForPlayer(Board board, int xPos, int yPos, COLOUR colourToCheckFor);
	int GetBoardPosWeight(int xPos, int yPos);
	void Castle();
	void SetCastlingAvailability();
	bool RookIsOnOpenFile(Board board, int rookXPos, int rookYPos);
	int ScorePawnPromotionPotential(int pawnXPos, int pawnYPos, COLOUR enemyColour);
	
	Board mNewBoard;
	bool mInOpeningMoves;
	bool mInEndGame;
	int* mSearchDepth;
	Move mBestMoveFound;

	//boardScore vars
	int mPawnVal = 1;
	int mKnightVal = 5;
	int mBishopVal = 5;
	int mRookVal = 10;
	int mQueenVal = 25;
	int mKingVal = 0;
	int mOpenFileValue = 15;
	int mSumPiecesOurs = 0;
	int mSumPiecesTheirs = 0;
	int mPlayerScoreTotal = 0;

	//determining if in endgame
	int mActivePieces = 0;
	
	//pawn promo score contributions
	int mDistZeroRowVal = 10;
	int mDistOneRowVal = 8;
	int mDistTwoRowVal = 6;
	int mDistThreeRowVal = 4;
	int mDistFourRowVal = 2;
	int mDistFiveRowVal = 1;
	int mDistSixRowVal = 1;
	int mPawnDistanceWeight = 1;

	vector<Move>* mPlayerMoves;
	vector<Move>* mPlayerAttackMoves;

	//kingside only
	bool mCanCastle = false;
	bool mHasCastled = false;
};

#endif //_CHESSAIPLAYER_
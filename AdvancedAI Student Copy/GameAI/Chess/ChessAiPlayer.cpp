#include "ChessAiPlayer.h"
#include "ChessMoveManager.h"
#include <chrono>
#include <ctime>
#include <string>

ChessAiPlayer::ChessAiPlayer(SDL_Renderer* renderer, COLOUR colour, Board* board, vector<Vector2D>* highlights, Vector2D* selectedPiecePosition, Move* lastMove, int* searchDepth)
	: ChessPlayer(renderer, colour, board, highlights, selectedPiecePosition, lastMove)
{
	mInOpeningMoves = true;
	mInEndGame = false;
	mSearchDepth = searchDepth;
	mNewBoard = Board();
	mBestMoveFound = Move();
	mPlayerMoves = new vector<Move>();
}

ChessAiPlayer::~ChessAiPlayer()
{
	delete mPlayerMoves;
	ChessPlayer::~ChessPlayer();
}

GAMESTATE ChessAiPlayer::PreTurn() // CHECKMATE | CHECK | STALEMATE | NORMAL
{
	return ChessPlayer::PreTurn(); 
}

int ChessAiPlayer::Maximum(Board board, int depth, int parentLow)
{
	if (depth == 0)
	{
		board.score = ScoreBoard(board);
		return board.score;
	}
	
	int alpha = -MaxInt;
	vector<Move> moves;
	GetAllMoveOptions(board, COLOUR_BLACK, &moves);

	for each (Move move in moves)
	{
		if (alpha > parentLow)
		{
			return alpha;
		}
		else
		{
			Board newBoard = CreateBoardStateFromMove(board, move);
			alpha = max(alpha, Minimum(newBoard, depth - 1, alpha));
			if (alpha > parentLow)
			{
				parentLow = alpha;

				if (depth == *mSearchDepth)
				{
					mBestMoveFound = move;
				}
			}
		}
	}
	return alpha;
}

int ChessAiPlayer::Minimum(Board board, int depth, int parentHigh)
{
	if (depth == 0)
	{
		board.score = ScoreBoard(board);
		return board.score;
	}

	int alpha = MaxInt;
	vector<Move> moves;
	GetAllMoveOptions(board, COLOUR_WHITE, &moves);

	for each (Move move in moves)
	{
		if (alpha < parentHigh)
		{
			return alpha;
		}
		else
		{
			Board newBoard = CreateBoardStateFromMove(board, move);
			alpha = min(alpha, Maximum(newBoard, depth - 1, alpha));
			if (alpha < parentHigh)
			{
				parentHigh = alpha;
			}
		}
	}
	return alpha;
}

bool ChessAiPlayer::TakeATurn(SDL_Event e) 
{
	//readability
	cout << endl;

	if (mInOpeningMoves)
	{
		if (OpeningsPlaybook::Instance()->GetCurrentMoveNumber() == 0)
		{
			//select an opening on first move, and play it out.
			OpeningsPlaybook::Instance()->SelectAnOpening();
		}
		
		//only continue with opening moves playbook if we're not in check
		if (!ChessPlayer::CheckForCheck(*mChessBoard, COLOUR_BLACK))
		{
			Move moveToTake = OpeningsPlaybook::Instance()->mSelectedOpening.at(OpeningsPlaybook::Instance()->GetCurrentMoveNumber());
			Board newBoard = CreateBoardStateFromMove(*mChessBoard, moveToTake);
			MakeAMove(moveToTake);
			OpeningsPlaybook::Instance()->IncrementCurrentMoveNumber();

			if (OpeningsPlaybook::Instance()->GetCurrentMoveNumber() > 2)
			{
				mInOpeningMoves = false;
			}

			*mChessBoard = newBoard;
			return true;
		}
		else //currently in check
		{
			//so call the turn again recursively, applying minimax instead of opening playbook.
			mInOpeningMoves = false;
			TakeATurn(SDL_Event());
		}
	}
	else
	{
		//if we can castle, do that instead
		if (mCanCastle && !mHasCastled)
		{
			Castle();
			return true;
		}
		else
		{
			int startParent = -MaxInt;

			Board startBoard = *mChessBoard;

			std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

			//decide on a move using minimax search
			Maximum(startBoard, *mSearchDepth, startParent);
			
			//Move Timing
			std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			std::time_t end_time = std::chrono::system_clock::to_time_t(end);
			cout << "Elapsed time : " << elapsed_seconds.count() << "s\n";

			MakeAMove(mBestMoveFound);

			//does board state allow for a castle next move?
			if (!mHasCastled)
			{
				SetCastlingAvailability();
			}

			if (mInEndGame)
			{
				mPawnDistanceWeight = 5;
			}

			return true;
		}
	}
}

void ChessAiPlayer::MakeAMove(Move moveToTake)
{
	mChessBoard->currentLayout[moveToTake.from_X][moveToTake.from_Y].hasMoved = true;
	mChessBoard->currentLayout[moveToTake.to_X][moveToTake.to_Y] = mChessBoard->currentLayout[moveToTake.from_X][moveToTake.from_Y];
	mChessBoard->currentLayout[moveToTake.from_X][moveToTake.from_Y] = BoardPiece();

	//Store the last move to output at start of turn.
	mLastMove->from_X = moveToTake.from_X;
	mLastMove->from_Y = moveToTake.from_Y;
	mLastMove->to_X = moveToTake.to_X;
	mLastMove->to_Y = moveToTake.to_Y;

	//Record the move.
	MoveManager::Instance()->StoreMove(Vector2D(moveToTake.from_X, moveToTake.from_Y), Vector2D(moveToTake.to_X, moveToTake.to_Y));

	//pawn promo
	//Check if we need to promote a pawn.
	if (mChessBoard->currentLayout[moveToTake.to_X][moveToTake.to_Y].piece == PIECE_PAWN &&
		(moveToTake.to_Y == 0 || moveToTake.to_Y == 7))
	{
		mChessBoard->currentLayout[moveToTake.to_X][moveToTake.to_Y].piece = PIECE_QUEEN;
	}
}

int ChessAiPlayer::ScoreBoard(Board board)
{
	mSumPiecesOurs = 0;
	mSumPiecesTheirs = 0;

	mActivePieces = 0;

	for (int x = 0; x < kBoardDimensions; x++)
	{
		for (int y = 0; y < kBoardDimensions; y++)
		{
			if (board.currentLayout[x][y].piece != PIECE_NONE && board.currentLayout[x][y].colour == COLOUR_BLACK)
			{
				mSumPiecesOurs += ScoreBoardForPlayer(board, x, y, COLOUR_BLACK);
			}
			else if (board.currentLayout[x][y].piece != PIECE_NONE && board.currentLayout[x][y].colour == COLOUR_WHITE)
			{
				mSumPiecesTheirs += ScoreBoardForPlayer(board, x, y, COLOUR_WHITE);
			}
		}
	}

	if (mActivePieces < 4)
	{
		mInEndGame = true;		
	}
	return mSumPiecesOurs - mSumPiecesTheirs;
}

int ChessAiPlayer::ScoreBoardForPlayer(Board board, int xPos, int yPos, COLOUR colourToCheckFor)
{
	int tempPieceCount = 0;
	mPlayerScoreTotal = 0;
	COLOUR oppositeColour = (colourToCheckFor == COLOUR_WHITE) ? COLOUR_BLACK : COLOUR_WHITE;

	if (board.currentLayout[xPos][yPos].piece != PIECE_NONE && board.currentLayout[xPos][yPos].colour == colourToCheckFor)
	{
		mPlayerMoves->clear();
		switch (board.currentLayout[xPos][yPos].piece)
		{
		case PIECE_KING:
			mPlayerScoreTotal += mKingVal * kPieceWeight;
			tempPieceCount++;
			break;
		case PIECE_QUEEN:
			mPlayerScoreTotal += mQueenVal * kPieceWeight;
			mPlayerScoreTotal += GetBoardPosWeight(xPos, yPos) * kPositionWeight;
			tempPieceCount++;
			break;
		case PIECE_ROOK:
			mPlayerScoreTotal += mRookVal * kPieceWeight;
			if (RookIsOnOpenFile(board, xPos, yPos))
			{
				mPlayerScoreTotal += mOpenFileValue * kPositionWeight;
			}
			tempPieceCount++;
			break;
		case PIECE_BISHOP:
			mPlayerScoreTotal += mBishopVal * kPieceWeight;
			tempPieceCount++;
			break;
		case PIECE_KNIGHT:
			mPlayerScoreTotal += mKnightVal * kPieceWeight;
			mPlayerScoreTotal += GetBoardPosWeight(xPos, yPos) * kPositionWeight;
			tempPieceCount++;
			break;
		case PIECE_PAWN:
			mPlayerScoreTotal += mPawnVal * kPieceWeight;
			mPlayerScoreTotal += ScorePawnPromotionPotential(xPos, yPos, oppositeColour) * kPositionWeight;
			tempPieceCount++;
			break;
		}
		if (colourToCheckFor == COLOUR_BLACK)
		{
			mActivePieces += tempPieceCount;
		}
		return mPlayerScoreTotal;
	}
}

//Closer to enemy backline our pawn is, better board score
int ChessAiPlayer::ScorePawnPromotionPotential(int pawnXPos, int pawnYPos, COLOUR enemyColour)
{
	int enemyBacklineYValue = (enemyColour == COLOUR_WHITE) ? 7 : 0;

	int yDistFromBackline = abs(enemyBacklineYValue - pawnYPos);

	switch (yDistFromBackline)
	{
	case 0:
		return mDistZeroRowVal * mPawnDistanceWeight;
	case 1:
		return mDistOneRowVal * mPawnDistanceWeight;
	case 2:
		return mDistTwoRowVal * mPawnDistanceWeight;
	case 3:
		return mDistThreeRowVal * mPawnDistanceWeight;
	case 4:
		return mDistFourRowVal * mPawnDistanceWeight;
	case 5:
		return mDistFiveRowVal * mPawnDistanceWeight;
	case 6:
		return mDistSixRowVal * mPawnDistanceWeight;
	default:
		return 0;
	}
}

//Each chess piece contributes a weight to the fitness sum based on position
int ChessAiPlayer::GetBoardPosWeight(int xPos, int yPos)
{
	//Square brackets denote positional fitness weight (xW short for xPosWeight)
	//--------------------------------------------------------------------------
	// |0,0[1]|1,0[1]|2,0[1]|3,0[1]|4,0[1]|5,0[1]|6,0[1]|7,0[1]| yPosWeight of 8
	// |-------------------------------------------------------|
	// |0,1[1]|1,1[2]|2,1[2]|3,1[2]|4,1[2]|5,1[2]|6,1[2]|7,1[1]| yPosWeight of 7
	// |-------------------------------------------------------|
	// |0,2[1]|1,2[2]|2,2[3]|3,2[3]|4,2[3]|5,2[3]|6,2[2]|7,2[1]| yPosWeight of 6
	// |-------------------------------------------------------|
	// |0,3[1]|1,3[2]|2,3[3]|3,3[4]|4,3[4]|5,3[3]|6,3[2]|7,3[1]| yPosWeight of 5
	// |-------------------------------------------------------|
	// |0,4[1]|1,4[2]|2,4[3]|3,4[4]|4,4[4]|5,4[3]|6,4[2]|7,4[1]| yPosWeight of 4
	// |-------------------------------------------------------|
	// |0,5[1]|1,5[2]|2,5[3]|3,5[3]|4,5[3]|5,5[3]|6,5[2]|7,5[1]| yPosWeight of 3
	// |-------------------------------------------------------|
	// |0,6[1]|1,6[2]|2,6[2]|3,6[2]|4,6[2]|5,6[2]|6,6[2]|7,6[1]| yPosWeight of 2
	// |-------------------------------------------------------|
	// |0,7[1]|1,7[1]|2,7[1]|3,7[1]|4,7[1]|5,7[1]|6,7[1]|7,7[1]| yPosWeight of 1
	//  xW=8   xW=7   xW=6   xW=5   xW=4   xW=3   xW=2   xW=1
	//--------------------------------------------------------------------------	

	int xPosWeight = kBoardDimensions - xPos;
	int yPosWeight = kBoardDimensions - yPos;

	if ((yPosWeight == 8 || yPosWeight == 1) || (xPosWeight == 8 || xPosWeight == 1))
	{
		return 10;
	}
	else if ((yPosWeight < 8 && yPosWeight > 1) && (xPosWeight < 8 && xPosWeight > 1))
	{
		return 20;
	}
	else if ((yPosWeight < 7 && yPosWeight > 2) && (xPosWeight < 7 && xPosWeight > 2))
	{
		return 30;
	}
	else if ((yPosWeight < 6 && yPosWeight > 3) && (xPosWeight < 6 && xPosWeight > 3))
	{
		return 40;
	}	
}

bool ChessAiPlayer::RookIsOnOpenFile(Board board, int rookXPos, int rookYPos)
{
	bool pieceFoundOnFile = false;
	int tempYPos = rookYPos;
	//check all squares going up the file
	while (tempYPos <= 7)
	{
		tempYPos++; //incremement immediately, because no need to check the pos the rook is currently on
		if (board.currentLayout[rookXPos][tempYPos].piece != PIECE_NONE)
		{
			pieceFoundOnFile = true;
			break; //file is not open
		}
	}

	tempYPos = rookYPos;
	//then check all squares going down the file
	while (tempYPos >= 0) //down to 0
	{
		tempYPos--; 
		if (board.currentLayout[rookXPos][tempYPos].piece != PIECE_NONE)
		{
			pieceFoundOnFile = true;
			break; //file is not open
		}
	}
	return true;
}

void ChessAiPlayer::Castle()
{
	//move king into position
	mChessBoard->currentLayout[4][0].hasMoved = true;
	mChessBoard->currentLayout[6][0] = mChessBoard->currentLayout[4][0];
	mChessBoard->currentLayout[4][0] = BoardPiece();

	//then call makeAMove on the rook normally
	MakeAMove(Move(7, 0, 5, 0));

	mHasCastled = true;
}

void ChessAiPlayer::SetCastlingAvailability()
{
	if (mChessBoard->currentLayout[4][0].piece == PIECE_KING &&
		mChessBoard->currentLayout[4][0].hasMoved == false && //isAKing and has not moved yet this game
		mChessBoard->currentLayout[7][0].piece == PIECE_ROOK &&
		mChessBoard->currentLayout[7][0].hasMoved == false && //isARook and has not moved yet this game
		mChessBoard->currentLayout[5][0].piece == PIECE_NONE &&
		mChessBoard->currentLayout[6][0].piece == PIECE_NONE) //no pieces between them
	{
		mCanCastle = true;
	}
	else
	{
		mCanCastle = false;
	}
}

Board ChessAiPlayer::CreateBoardStateFromMove(Board currentBoard, Move theMove)
{
	Board tempBoard = currentBoard;
	//apply move to update the board
	tempBoard.currentLayout[theMove.to_X][theMove.to_Y] = tempBoard.currentLayout[theMove.from_X][theMove.from_Y];
	tempBoard.currentLayout[theMove.from_X][theMove.from_Y] = BoardPiece();

	return tempBoard;
}


//Populates attackMoves paramater with all attacking moves found from MovesForThisPiece var
void ChessAiPlayer::GetAttacksForPiece(Board boardToTest, vector<Move>* movesForThisPiece, vector<Move>* attackMoves, COLOUR enemyColour)
{
	for each (Move moveToTake in *movesForThisPiece)
	{
		//the piece in position at the destination of the move
		BoardPiece pieceToCheck = boardToTest.currentLayout[moveToTake.to_X][moveToTake.to_Y];

		//if this location on the board is populated with an enemy piece
		if (pieceToCheck.piece != PIECE_NONE && pieceToCheck.colour == enemyColour)
		{
			//store it
			attackMoves->push_back(moveToTake);
		}
	}
}

int ChessAiPlayer::GetValOfBestAttack(Board boardToTest, vector<Move>* attackMoves)
{
	int valueOfBestAttack = 0;
	for each (Move move in *attackMoves)
	{
		switch (boardToTest.currentLayout[move.to_X][move.to_Y].piece)
		{
		case PIECE_KING:
			if (2 * mQueenVal > valueOfBestAttack)
			{
				valueOfBestAttack = 2 * mQueenVal;
			}
			break;
		case PIECE_QUEEN:
			if (mQueenVal > valueOfBestAttack)
			{
				valueOfBestAttack = mQueenVal;
			}
			break;
		case PIECE_BISHOP:
			if (mBishopVal > valueOfBestAttack)
			{
				valueOfBestAttack = mBishopVal;
			}
			break;
		case PIECE_KNIGHT:
			if (mKnightVal > valueOfBestAttack)
			{
				valueOfBestAttack = mKnightVal;
			}
			break;
		case PIECE_ROOK:
			if (mRookVal > valueOfBestAttack)
			{
				valueOfBestAttack = mRookVal;
			}
			break;
		case PIECE_PAWN:
			if (mPawnVal > valueOfBestAttack)
			{
				valueOfBestAttack = mPawnVal;
			}
			break;
		}
	}
	return valueOfBestAttack;
}

void ChessAiPlayer::OutputMove(Move move)
{
	cout << "Move is: (" << move.from_X << ", " << move.from_Y << ") -> ("
		<< move.to_X << ", " << move.to_Y << ")" << endl;
}

void ChessAiPlayer::OutputBoardState(Board board)
{
	for (int i = 0; i < kBoardDimensions; i++)
	{
		for (int j = 0; j < kBoardDimensions; j++)
		{
			switch (board.currentLayout[j][i].piece)
			{
			case PIECE_NONE:
				cout << "0";
				break;
			case PIECE_PAWN:
				cout << "P";
				break;
			case PIECE_ROOK:
				cout << "R";
				break;
			case PIECE_KNIGHT:
				cout << "N";
				break;
			case PIECE_BISHOP:
				cout << "B";
				break;
			case PIECE_QUEEN:
				cout << "Q";
				break;
			case PIECE_KING:
				cout << "K";
				break;
			}
		}
		cout << endl;
	}
	cout << endl;
}

void ChessAiPlayer::EndTurn()
{
	ChessPlayer::EndTurn();
}
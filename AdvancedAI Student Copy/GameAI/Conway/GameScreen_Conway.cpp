//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#include "GameScreen_Conway.h"
#include <stdlib.h>
#include "../Texture2D.h"
#include "ConwayConstants.h"
#include "../TinyXML/tinyxml.h"

//--------------------------------------------------------------------------------------------------

GameScreen_Conway::GameScreen_Conway(SDL_Renderer* renderer) : GameScreen(renderer)
{
	mMap = new int*[kConwayScreenWidth/kConwayTileDimensions];
	for(int i =0; i < kConwayScreenWidth/kConwayTileDimensions; i++)
	{
		mMap[i] = new int[kConwayScreenHeight/kConwayTileDimensions];
	}
	mNextMapState = new int*[kConwayScreenWidth / kConwayTileDimensions];
	for (int j = 0; j < kConwayScreenWidth / kConwayTileDimensions; j++)
	{
		mNextMapState[j] = new int[kConwayScreenHeight / kConwayTileDimensions];
	}

	//Get all required textures.
	mWhiteTile = new Texture2D(renderer);
	mWhiteTile->LoadFromFile("Images/Conway/WhiteTile.png");
	mBlackTile = new Texture2D(renderer);
	mBlackTile->LoadFromFile("Images/Conway/BlackTile.png");

	//Start map has 50% active cells.
	CreateRandomMap(50);

	//Start game in frozen state.
	mPause = true;
}

//--------------------------------------------------------------------------------------------------

GameScreen_Conway::~GameScreen_Conway()
{
	delete mWhiteTile;
	mWhiteTile = NULL;

	delete mBlackTile;
	mBlackTile = NULL;

	for(int i =0; i < kConwayScreenWidth/kConwayTileDimensions; i++)
	{
		delete mMap[i];
		mMap[i] = NULL;
	}
	delete mMap;
	mMap = NULL;

	for (int j = 0; j < kConwayScreenWidth / kConwayTileDimensions; j++)
	{
		delete mNextMapState[j];
		mNextMapState[j] = NULL;
	}
	delete mNextMapState;
	mNextMapState = NULL;

}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::Render()
{
	for(int x = 0; x < kConwayScreenWidth/kConwayTileDimensions; x++)
	{
		for(int y = 0; y < kConwayScreenHeight/kConwayTileDimensions; y++)
		{
			switch(mMap[x][y])
			{
				case 0:
					mBlackTile->Render(Vector2D(x*kConwayTileDimensions,y*kConwayTileDimensions));
				break;
				
				case 1:
					mWhiteTile->Render(Vector2D(x*kConwayTileDimensions,y*kConwayTileDimensions));
				break;
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::Update(float deltaTime, SDL_Event e)
{
	//Additional input outside of player.
	switch(e.type)
	{
		//Deal with mouse click input.
		case SDL_KEYUP:
			switch(e.key.keysym.sym)
			{
				case SDLK_SPACE:
					mPause = !mPause;
				break;

				case SDLK_r:
					mPause = true;
					CreateRandomMap(50);
				break;

				case SDLK_l:
					mPause = true;
					LoadMap("Conway/ConwaySeed.xml");
				break;
			}
		break;
		
		//500x500 pixel window
		//but 250x250 grid
		//sdl coordinates in pixels
		//map the coordinates to the x,y of mMap
		case SDL_MOUSEBUTTONDOWN:
			if (mPause == true)
			{
				switch (e.button.button)
				{
				case SDL_BUTTON_LEFT:
					mMap[e.button.x / 2][e.button.y / 2] = 0;
					break;
				case SDL_BUTTON_RIGHT:
					mMap[e.button.x / 2][e.button.y / 2] = 1;
					break;
				}
			}
		break;

		default:
		break;
	}
	
	if (!mPause)
	{
		UpdateMap();
		SetNextMapState();
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::UpdateMap()
{
	//Rules
	//1. Any living cell with less than 2 live neighbour dies.
	//2. Any living cell with 2 or 3 live cells lives on to next generation.
	//3. Any live cell with more than 3 live neighbours dies.
	//4. Any dead cell with exactly 3 live neighbours becomes a living cell.
	ZeroMap(mNextMapState);

	for (int x = 0; x < kConwayScreenWidth / kConwayTileDimensions; x++)
	{
		for (int y = 0; y < kConwayScreenHeight / kConwayTileDimensions; y++)
		{
			int livingNeighbours = GetLivingNeighbourCount(x, y);
			if (mMap[x][y] == 1)
			{
				//Cell is live
				if (livingNeighbours < 2 || livingNeighbours > 3)
				{
					//loneliness or overcrowding
					mNextMapState[x][y] = 0;
				}
				else if (livingNeighbours == 2 || livingNeighbours == 3)
				{
					//live on to next generation
					mNextMapState[x][y] = 1;
				}
			}
			else
			{
				//Cell is dead
				if (livingNeighbours == 3)
				{
					mNextMapState[x][y] = 1;
				}
			}
		}
	}
}

void GameScreen_Conway::SetNextMapState()
{
	for (int i = 0; i < kConwayScreenWidth / kConwayTileDimensions; i++)
	{
		for (int j = 0; j < kConwayScreenWidth / kConwayTileDimensions; j++)
		{
			mMap[i][j] = mNextMapState[i][j];
		}
	}
}

void GameScreen_Conway::ZeroMap(int** &mMap)
{
	for (int i = 0; i < kConwayScreenWidth / kConwayTileDimensions; i++)
	{
		for (int j = 0; j < kConwayScreenHeight / kConwayTileDimensions; j++)
		{
			mMap[i][j] = 0;
		}
	}
}

int GameScreen_Conway::GetLivingNeighbourCount(int mapX, int mapY)
{
	int count = 0;
	int mapWidth = kConwayScreenWidth / kConwayTileDimensions;
	int mapHeight = kConwayScreenHeight / kConwayTileDimensions;

	/*
	//better algorithm, edge echecks are broken
	int neighbours[8][2] =
	{
		{ -1, -1 },//topleft - x and y not 0
		{ -1, 0 },//left - x not 0
		{ -1, 1 },//bottomleft - x not 0, y not height-1
		{ 0, -1 },//top - y not 0
		{ 0, 1 },//bottom - y not height-1
		{ 1, -1 },//topright - x not width-1, y not 0
		{ 1, 0 },//right - x not width-1
		{ 1, 1 }//bottomright - x not width-1, y not height-1
	};
	
	
	for (auto& offset : neighbours)
	{
		if (mapX != 0 && mapY != 0 && mapX != mapWidth - 1 && mapY != mapHeight - 1)
		{
			if (mMap[mapX + offset[1]][mapY + offset[0]] == 1)
			{
				count++;
			}
		}
	}
	return count;
	*/
	
	//right
	if (mapX != (mapWidth - 1))//Edge check
	{
		if (mMap[mapX + 1][mapY] == 1)
		{
			count++;
		}
	}
	//bottom right
	if (mapX != (mapWidth - 1) && mapY != (mapHeight - 1))//Edge check
	{
		if (mMap[mapX + 1][mapY + 1] == 1)
		{
			count++;
		}
	}
	//bottom
	if (mapY != mapHeight - 1)//Edge check
	{
		if (mMap[mapX][mapY + 1] == 1)
		{
			count++;
		}
	}
	//bottom left
	if (mapX != 0 && mapY != (mapHeight - 1))//Edge check
	{
		if (mMap[mapX - 1][mapY + 1] == 1)
		{
			count++;
		}
	}
	//left
	if (mapX != 0)//Edge check
	{
		if (mMap[mapX - 1][mapY] == 1)
		{
			count++;
		}
	}
	//top left
	if (mapX != 0 && mapY != 0)//Edge check
	{
		if (mMap[mapX - 1][mapY - 1] == 1)
		{
			count++;
		}
	}
	//top
	if (mapY != 0)//Edge check
	{
		if (mMap[mapX][mapY - 1] == 1)
		{
			count++;
		}
	}
	//top right
	if (mapX != (mapWidth - 1) && mapY != 0)//Edge check
	{
		if (mMap[mapX + 1][mapY - 1] == 1)
		{
			count++;
		}
	}

	return count;
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::CreateRandomMap(int percentageInactive)
{
	int randomNumber = 0;

	for(int x = 0; x < kConwayScreenWidth/kConwayTileDimensions; x++)
	{
		for(int y = 0; y < kConwayScreenHeight/kConwayTileDimensions; y++)
		{
			randomNumber = (rand() % 100)+1;

			if(randomNumber > percentageInactive)
				mMap[x][y] = 0;
			else
				mMap[x][y] = 1;
		}
	}
}

//--------------------------------------------------------------------------------------------------

void GameScreen_Conway::LoadMap(std::string path)
{
	//Get the whole xml document.
	TiXmlDocument doc;
	if(!doc.LoadFile(path))
	{
		cerr << doc.ErrorDesc() << endl;
	}

	//Now get the root element.
	TiXmlElement* root = doc.FirstChildElement();
	if(!root)
	{
		cerr << "Failed to load file: No root element." << endl;
		doc.Clear();
	}
	else
	{
		//Jump to the first 'objectgroup' element.
		for(TiXmlElement* groupElement = root->FirstChildElement("objectgroup"); groupElement != NULL; groupElement = groupElement->NextSiblingElement())
		{
			string name = groupElement->Attribute("name");
			if(name == "Seed")
			{
				int x = 0;
				int y = 0;

				//Jump to the first 'object' element - within 'objectgroup'
				for(TiXmlElement* objectElement = groupElement->FirstChildElement("object"); objectElement != NULL; objectElement = objectElement->NextSiblingElement())
				{
					string name = objectElement->Attribute("name");
					if(name == "TileTypes")
					{
						//Reset x position of map to 0 at start of each element.
						x = 0;

						//Split up the comma delimited connections.
						stringstream ss(objectElement->Attribute("value"));
						int i;
						while(ss >> i)
						{
							mMap[x][y] = i;

							if(ss.peek() == ',')
								ss.ignore();

							//Increment the x position.
							x++;
						}

						//Move down to the next y position in the map.
						y++;
					}
				}
			}
		}
	}
}
//------------------------------------------------------------------------
//  Author: Paul Roberts 2016
//------------------------------------------------------------------------

#include "LunarLander.h"
#include "../Texture2D.h"
#include "LunarConstants.h"
#include <SDL.h>
#include <iostream>
#include "../C2DMatrix.h"
#include "../Collisions.h"

using namespace::std;

//--------------------------------------------------------------------------------------------------

LunarLander::LunarLander(SDL_Renderer* renderer, Vector2D startPosition, Rect2D platform)
{
	mRenderer		 = renderer;
	mLandingPlatform = platform;

	mAliveTexture = new Texture2D(renderer);
	mAliveTexture->LoadFromFile("Images/Lunar/Lander.png");
	mDeadTexture = new Texture2D(renderer);
	mDeadTexture->LoadFromFile("Images/Lunar/LanderRed.png");
	mLandedTexture = new Texture2D(renderer);
	mLandedTexture->LoadFromFile("Images/Lunar/LanderGreen.png");
	mThrustTexture = new Texture2D(renderer);
	mThrustTexture->LoadFromFile("Images/Lunar/Thrust.png");

	mStartPosition = startPosition;
	Reset();
}

//--------------------------------------------------------------------------------------------------

LunarLander::~LunarLander()
{
	delete mAliveTexture;
	mAliveTexture = NULL;

	delete mDeadTexture;
	mDeadTexture = NULL;

	delete mLandedTexture;
	mLandedTexture = NULL;
}

//--------------------------------------------------------------------------------------------------

void LunarLander::Update(float deltaTime, SDL_Event e)
{
	if(mAlive && !mLanded)
	{
		mSurvivalTime   += deltaTime;
		mVelocity.y		+= kGravity;
		mPosition		+= mVelocity*deltaTime;

		//Visual effects for thrusting.
		if(mThrusting)
		{
			mThrustingTime += deltaTime;

			if (mThrustingTime > kThrustDuration)
			{
				mThrusting = false;
			}
		}

		//Has the lander hit the screen edges or the landing platform?
		if (HasCollidedWithScreenEdge())
		{
			mAlive = false;
		}
		else if(HasCollidedWithPlatform())
		{
			if (HasTouchedDownSafely())
			{
				mLanded = true;
			}
			else
			{
				mAlive = false;
			}
		}
	}
}

double LunarLander::TestFitness()
{
	double distFromPlatformX = fabs(mLandingPlatform.x - mPosition.x);
	double distFromPlatformY = fabs(mLandingPlatform.y - mPosition.y);
	double disFitSquared = ((kLunarScreenWidth - distFromPlatformX) * (kLunarScreenWidth - distFromPlatformX)) + ((kLunarScreenHeight - distFromPlatformY) * (kLunarScreenHeight - distFromPlatformY));
	double disFit = sqrt(disFitSquared);
	double rotFit = 1 / fabs(GetRotationAngle()+1);
	double airTime = GetSurvivalTime() / (GetSpeed() + 1);

	double fitness = (kDistWeight * disFit) + (kRotWeight * rotFit) + (kAirTimeWeight * airTime);

	return fitness;
}

//--------------------------------------------------------------------------------------------------

void LunarLander::Render()
{
	if (mAlive)
	{
		if (mLanded)
		{
			mLandedTexture->Render(mPosition, SDL_FLIP_NONE, mRotationAngle);
		}
		else
		{
			mAliveTexture->Render(mPosition, SDL_FLIP_NONE, mRotationAngle);

			//Add the thrust image if thrusting.
			if (mThrusting)
			{
				mThrustTexture->Render(mPosition, SDL_FLIP_NONE, mRotationAngle);
			}
		}
	}
	else
	{
		mDeadTexture->Render(mPosition, SDL_FLIP_NONE, mRotationAngle);
	}
}

//--------------------------------------------------------------------------------------------------

Vector2D LunarLander::GetCentralPosition()
{
	Vector2D centre = Vector2D(mPosition.x+(mAliveTexture->GetWidth()*0.5f), mPosition.y+(mAliveTexture->GetHeight()*0.5f));
	return centre;
}

//--------------------------------------------------------------------------------------------------

Rect2D LunarLander::GetAdjustedBoundingBox()
{
	//Return texture size - Override for more precise collision.
	//This assumes there is only one image on the texture.
	return Rect2D(mPosition.x, mPosition.y, (double)mAliveTexture->GetWidth(), (double)mAliveTexture->GetHeight());
}

//--------------------------------------------------------------------------------------------------

void LunarLander::RotateByRadian(double radian, float deltaTime)
{
	double degrees = RadsToDegs(radian);
	mRotationAngle += degrees;
	if (mRotationAngle > 360.0f)
	{
		mRotationAngle -= 360.0f;
	}
	else if (mRotationAngle < 0.0f)
	{
		mRotationAngle += 360.0f;
	}
  
	//Usee a rotation matrix to rotate the player's heading
	C2DMatrix RotationMatrix;
  
	//Calculate the direction of rotation.
	RotationMatrix.Rotate(radian);	

	//Get the new fire direction.
	RotationMatrix.TransformVector2Ds(mHeading);
}

//--------------------------------------------------------------------------------------------------

void LunarLander::TurnLeft(float deltaTime)
{
	if(mAlive && !mLanded)
	{
		RotateByRadian(-0.2f, deltaTime);
	}
}

//--------------------------------------------------------------------------------------------------

void LunarLander::TurnRight(float deltaTime)
{
	if(mAlive && !mLanded)
	{
		RotateByRadian(0.2f, deltaTime);
	}
}

//--------------------------------------------------------------------------------------------------

void LunarLander::Thrust()
{
	if(mAlive && !mLanded)
	{
		//Add to the velocity the thrust value * new heading.
		mVelocity		+= mHeading * kThrust;

		//Visual effects.
		mThrusting		= true;
		mThrustingTime	= 0.0f;
	}
}

//--------------------------------------------------------------------------------------------------

void LunarLander::Reset()
{
	mLanded			 = false;
	mAlive			 = true;
	mPosition		 = mStartPosition;
	mVelocity		 = Vector2D();
	mHeading		 = Vector2D(0.0f, -1.0f);
	mRotationAngle	 = 0.0f;
	mSurvivalTime	 = 0.0;
	mThrusting		 = false;
	mThrustingTime	 = 0.0f;
}

//--------------------------------------------------------------------------------------------------

bool LunarLander::HasCollidedWithScreenEdge()
{
	if( mPosition.x < 0.0f ||
		mPosition.x + mAliveTexture->GetWidth() > kLunarScreenWidth ||
		mPosition.y < 0.0f ||
		mPosition.y + mAliveTexture->GetHeight() > kLunarScreenHeight )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------------------------------

bool LunarLander::HasCollidedWithPlatform()
{
	if( Collisions::Instance()->PointInBox(mPosition, mLandingPlatform) ||
		Collisions::Instance()->PointInBox(Vector2D(mPosition.x + mAliveTexture->GetWidth(), mPosition.y), mLandingPlatform) ||
		Collisions::Instance()->PointInBox(Vector2D(mPosition.x, mPosition.y+mAliveTexture->GetHeight()), mLandingPlatform) ||
		Collisions::Instance()->PointInBox(Vector2D(mPosition.x + mAliveTexture->GetWidth(), mPosition.y+mAliveTexture->GetHeight()), mLandingPlatform) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------------------------------

bool LunarLander::HasTouchedDownSafely()
{
	if( mVelocity.Length() < kLandingSpeed &&																												//Correct descent speed.
		(mRotationAngle == 0.0f || mRotationAngle == 360.0f) &&																								//No rotation.
		mPosition.y-mAliveTexture->GetHeight() < mLandingPlatform.y &&																						//Above the platform.
		Collisions::Instance()->PointInBox(Vector2D(mPosition.x, mPosition.y+mAliveTexture->GetHeight()), mLandingPlatform) &&								//Both sides of the lander are on the platform.
		Collisions::Instance()->PointInBox(Vector2D(mPosition.x + mAliveTexture->GetWidth(), mPosition.y+mAliveTexture->GetHeight()), mLandingPlatform) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------------------------------

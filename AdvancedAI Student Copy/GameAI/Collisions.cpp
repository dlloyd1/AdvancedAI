//------------------------------------------------------------------------
//  Author: Paul Roberts 2015
//------------------------------------------------------------------------

#include "Collisions.h"
#include "Commons.h"
#include <iostream>

//Initialise the instance to null.
Collisions* Collisions::mInstance = NULL;

//--------------------------------------------------------------------------------------------------

Collisions::Collisions()
{
}

//--------------------------------------------------------------------------------------------------

Collisions::~Collisions()
{
	mInstance = NULL;
}

//--------------------------------------------------------------------------------------------------

Collisions* Collisions::Instance()
{
	if (!mInstance)
	{
		mInstance = new Collisions;
	}

	return mInstance;
}

//--------------------------------------------------------------------------------------------------

bool Collisions::PointInBox(Vector2D point, Rect2D rect2)
{
	if (point.x > rect2.x &&
		point.x < (rect2.x + rect2.width) &&
		point.y > rect2.y &&
		point.y < (rect2.y + rect2.height))
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------

bool Collisions::Circle(Vector2D circle1_Pos, float circle1_Radius, Vector2D circle2_Pos, float circle2_Radius)
{
	Vector2D vec = Vector2D((circle1_Pos.x - circle2_Pos.x), (circle1_Pos.y - circle2_Pos.y));
	double distance = sqrt((vec.x*vec.x) + (vec.y*vec.y));

	double combinedDistance = (circle1_Radius + circle2_Radius);
	return distance < combinedDistance;
}

//--------------------------------------------------------------------------------------------------

bool Collisions::Box(Rect2D rect1, Rect2D rect2)
{
	if (rect1.x + (rect1.width / 2) > rect2.x &&
		rect1.x + (rect1.width / 2) < rect2.x + rect2.width &&
		rect1.y + (rect1.height / 2) > rect2.y &&
		rect1.y + (rect1.height / 2) < rect2.y + rect2.height)
	{
		return true;
	}

	return false;
}
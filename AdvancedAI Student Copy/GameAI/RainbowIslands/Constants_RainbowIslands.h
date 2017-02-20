//------------------------------------------------------------------------
//  Author: Paul Roberts 2017
//------------------------------------------------------------------------

#ifndef RAINBOW_CONSTANTS_H
#define RAINBOW_CONSTANTS_H

#define RAD2DEG	57.2957795f

//Screen dimensions.
const int kRainbowIslandsScreenWidth	= 320;
const int kRainbowIslandsScreenHeight   = 624;// 320;

//Background map dimensions.
const int MAP_WIDTH						= 20;
const int MAP_HEIGHT					= 39;

const int TILE_WIDTH					= 16;
const int TILE_HEIGHT					= 16;

//Generic
const float MOVE_SPEED					= 0.16f;
const float GRAVITY						= 0.1f;
const float DEAD_TIME					= 0.5f;

//Characters
const float ANIMATION_DELAY				= 100.0f;
const float RAINBOW_DELAY				= 10.0f;
const float RAINBOW_TIME				= 2000.0f;

//Caterpillar
const float CALM_SPEED					= 0.05f;
const float ANGRY_SPEED					= 0.1f;

//Jumping.
const float JUMP_FORCE_INITIAL			= 0.26f;
const float JUMP_FORCE_DECREMENT		= 0.008f;
const float JUMP_FORCE_INITIAL_FRUIT	= 0.29f;

//Level Time.
const int LEVEL_TIME = 10000;/*60000;*///60 seconds
const int TRIGGER_ANGRY_TIME			= 10000;//10 seconds


//Background tile map sprite sheet.
const int kTileSpriteSheetWidth			= 5;
const int kTileSpriteSheetHeight		= 5;

//0.05 - 0.3
const double kRainbowMutationRate = 0.05;

const double kRainbowCrossoverRate = 0.7;

const int kNumNNInputs = 3;
const int kNumNNOutputs = 4;
const int kNumNNHiddenLayers = 1;
const int kNumNNNeuronsPerHiddenLayer = 6;
const double kMaxPerturbation = 0.3;
const int kActivationResponse = 1;
const int kBias = -1;

const int kRunsPerGeneration = 10;


#endif //RAINBOW_CONSTANTS_H
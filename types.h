#pragma once

#include "olcPixelGameEngine.h"
#include "olcPGEX_TransformedView.h"
#include "olcPGEX_Gamepad.h"
#include "olcPGEX_MiniAudio.h"

#include "box2d/box2d.h"

#include <ranges>

// ================================================================================================

using vec2 = b2Vec2; // olc::vf2d;
struct posSize { vec2 Pos{ 0.0f, 0.0f }; vec2 Size{ 1.0f, 1.0f }; };
struct rectCorners { vec2 TopLeft{ 0.0f, 0.0f }; vec2 BottomRight{ 1.0f, 1.0f }; };
struct lowHigh { float Low{ 0 }; float High{ 0 }; };

// ================================================================================================

// Screen
const vec2 SCREEN_SIZE{ 1280.0f, 720.0f };
const float SCREEN_WIDTH{ SCREEN_SIZE.x };
const float SCREEN_HEIGHT{ SCREEN_SIZE.y };

const vec2 TILE_SIZE{ 16.0f, 16.0f };
const float TILE_WIDTH{ TILE_SIZE.x };
const float TILE_HEIGHT{ TILE_SIZE.y };

// Coordinates
const vec2 ZERO{ 0.0f, 0.0f };
const vec2 UP{ 0.0f, -1.0f };
const vec2 RIGHT{ 1.0f, 0.0f };

// Limits
const float MAX_FLOAT{ (std::numeric_limits<float>::max)() };
const float MIN_FLOAT{ (std::numeric_limits<float>::min)() };

// Constants
constexpr float PI{ 3.14159265359f }; 
const float MIPS_CHANGE_FACTOR{ 1.0f };

// ================================================================================================

inline auto vec2_to_vf2d = [](vec2 Pos)
	{
		return olc::vf2d{ Pos.x, Pos.y };
	};

inline auto vf2d_to_vec2 = [](olc::vf2d Pos)
	{
		return vec2{ Pos.x, Pos.y };
	};

// ================================================================================================

inline auto rectCorners_to_posSize = [](rectCorners RectCorners)
	{
		vec2 RectSize = RectCorners.BottomRight - RectCorners.TopLeft;
		return posSize{ RectCorners.TopLeft + 0.5f * RectSize, RectSize };
	};

inline auto posSize_to_rectCorners = [](posSize PosSize)
	{
		return rectCorners{ PosSize.Pos - 0.5f * PosSize.Size, PosSize.Pos + 0.5f * PosSize.Size };
	};

// ================================================================================================

inline int CyclicalMod(int a, int b)
{
	return ((a % b) + b) % b;
}

// ================================================================================================

enum class appStateId { SPLASH, MENU, GAME };
enum class gameStateId { START_NEW_GAME, START_NEW_LEVEL, PLAYING, CLEARED_TUTORIAL, LEVEL_JUST_CLEARED, SCORING_LEVEL, LEVEL_SCORED, GAME_OVER, WIN  };
enum class easingId { LINEAR, EASE_IN, EASE_OUT, EASE_IN_OUT, BOUNCE, EASE_OUT_ELASTIC};

enum class entityCategoryId
{
	GROUND = 1 << 0,
	PLAYER = 1 << 1,
	GIBBON = 1 << 2,
	PROJECTILE = 1 << 3,
	STAGE = 1 << 4,
	PIN = 1 << 5,
	GRAVITY_FIELD = 1 << 6
};

// ================================================================================================

const int COUNT_INPUT_ID{ 12 };

enum class inputID
{
	DPAD_UP, DPAD_LEFT, DPAD_DOWN, DPAD_RIGHT,
	FACE_UP, FACE_LEFT, FACE_DOWN, FACE_RIGHT,
	SHOULDER_LEFT, SHOULDER_RIGHT, START, SELECT
};

std::vector<olc::Key> Player1Keys{ olc::Key::UP, olc::Key::LEFT, olc::Key::DOWN, olc::Key::RIGHT, olc::Key::SPACE, olc::Key::SPACE, olc::Key::SPACE, olc::Key::SPACE, olc::Key::NONE, olc::Key::NONE, olc::Key::P, olc::Key::NONE };

class playerInputs
{
public:
	bool bIsGamePadConnected{ false };
	bool bIsPlayerSubscribed{ false };

	std::array<bool, COUNT_INPUT_ID> Pressed{ false };
	std::array<bool, COUNT_INPUT_ID> Held{ false };
	std::array<bool, COUNT_INPUT_ID> HeldLastFrame{ false };

	bool IsPressed(inputID InputID) { return Pressed[(int)InputID]; }
	bool IsHeld(inputID InputID) { return Held[(int)InputID]; }
	bool IsPressedOrHeld(inputID InputID) { return IsPressed(InputID) || IsHeld(InputID); }

	void ResetInputs()
	{
		for (int i = 0; i < COUNT_INPUT_ID; ++i)
		{
			Pressed[i] = false;
			Held[i] = false;
		}
	}
};

// ================================================================================================

std::string EasingToString(easingId EasingID)
{
	switch (EasingID)
	{
	case easingId::LINEAR: return "LINEAR";
	case easingId::EASE_IN: return "EASE_IN";
	case easingId::EASE_OUT: return "EASE_OUT";
	case easingId::EASE_IN_OUT: return "EASE_IN_OUT";
	case easingId::BOUNCE: return "BOUNCE";
	case easingId::EASE_OUT_ELASTIC: return "EASE_OUT_ELASTIC";
	}

	// Should never get here:
	throw std::invalid_argument("Unimplemented Enum Item"); // **
	return "LINEAR";
}

easingId StringToEasing(std::string sID)
{
	if (sID == "LINEAR") return easingId::LINEAR;
	if (sID == "EASE_IN") return easingId::EASE_IN;
	if (sID == "EASE_OUT") return easingId::EASE_OUT;
	if (sID == "EASE_IN_OUT") return easingId::EASE_IN_OUT;
	if (sID == "BOUNCE") return easingId::BOUNCE;
	if (sID == "EASE_OUT_ELASTIC") return easingId::EASE_OUT_ELASTIC;

	// Should never get here:
	throw std::invalid_argument("Unimplemented Enum Item"); // **
	return easingId::LINEAR;
}

// ================================================================================================

float Easing(easingId EasingId, float PercentageNow)
{
	switch (EasingId) // ** Plenty of scope for more easing flexibility
	{
		// Ref: https://easings.net/
		// Formulae: https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c
	case easingId::LINEAR:
		// Do nothing
		break;
	case easingId::EASE_IN: // ** QUBIC
		PercentageNow = PercentageNow * PercentageNow * PercentageNow;
		break;
	case easingId::EASE_OUT:
		PercentageNow = 1.0f - PercentageNow;
		PercentageNow = 1.0f - PercentageNow * PercentageNow * PercentageNow;
		break;
	case easingId::EASE_IN_OUT: // ** QUBIC
		if (PercentageNow < 0.5f)
		{   // Ease-in phase:
			PercentageNow = 4.0f * PercentageNow * PercentageNow * PercentageNow;
		}
		else
		{   // Ease-out phase
			PercentageNow = ((2.0f * PercentageNow) - 2.0f);
			PercentageNow = 0.5f * PercentageNow * PercentageNow * PercentageNow + 1.0f;
		}
		break;
	case easingId::BOUNCE:
		// ** To Do
		break;
	case easingId::EASE_OUT_ELASTIC:
		// Elastic ease-out: exponential decay + sine wobble
		const float c4 = (2.0f * PI) / 3.0f;

		if (PercentageNow == 0.0f || PercentageNow == 1.0f)
		{
			// Perfect start/end
		}
		else
		{
			PercentageNow =
				powf(2.0f, -10.0f * PercentageNow) *
				sinf((PercentageNow * 10.0f - 0.75f) * c4) + 1.0f;
		}
	break;

	};

	return PercentageNow;
}

// ================================================================================================

vec2 Angle_to_Vector(float AngleInRad) // Relative to right being zero
{
	return vec2{ cos(AngleInRad), sin(AngleInRad)};
}

// ================================================================================================

struct debugLine { vec2 Pos1{ 0.0f, 0.0f }; vec2 Pos2{ 1.0f, 1.0f }; olc::Pixel Tint; };

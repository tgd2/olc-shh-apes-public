#pragma once

#include "types.h"
#include "state.h"

extern state State;

namespace ObjectRecipes
{
	// ==================================================

	// Advanced gfx mapping:

	std::unordered_map<std::string, advancedGfx> AdvancedGfxMap
	{
		{"./gfx/apes/baby-gorilla-base-96x96.png", advancedGfx{{
			mipLevel{ vec2{24.0f, 24.0f}, "./gfx/apes/frames/baby-gorilla-24x24.png" },
			mipLevel{ vec2{48.0f, 48.0f}, "./gfx/apes/frames/baby-gorilla-48x48.png" },
			mipLevel{ vec2{96.0f, 96.0f}, "./gfx/apes/frames/baby-gorilla-96x96.png" },
			mipLevel{ vec2{192.0f, 192.0f}, "./gfx/apes/frames/baby-gorilla-192x192.png" },
			mipLevel{ vec2{384.0f, 384.0f}, "./gfx/apes/frames/baby-gorilla-384x384.png" }
			}}},
		{"./gfx/apes/baby-gibbon-base-96x96.png", advancedGfx{{
			mipLevel{ vec2{24.0f, 24.0f}, "./gfx/apes/frames/baby-gibbon-24x24.png" },
			mipLevel{ vec2{48.0f, 48.0f}, "./gfx/apes/frames/baby-gibbon-48x48.png" },
			mipLevel{ vec2{96.0f, 96.0f}, "./gfx/apes/frames/baby-gibbon-96x96.png" },
			mipLevel{ vec2{192.0f, 192.0f}, "./gfx/apes/frames/baby-gibbon-192x192.png" },
			mipLevel{ vec2{384.0f, 384.0f}, "./gfx/apes/frames/baby-gibbon-384x384.png" }
			}}},
		{"./gfx/apes/gibbon-base-96x128.png", advancedGfx{{
			mipLevel{ vec2{24.0f, 32.0f}, "./gfx/apes/frames/gibbon-24x32.png" },
			mipLevel{ vec2{48.0f, 64.0f}, "./gfx/apes/frames/gibbon-48x64.png" },
			mipLevel{ vec2{96.0f, 128.0f}, "./gfx/apes/frames/gibbon-96x128.png" },
			mipLevel{ vec2{192.0f, 256.0f}, "./gfx/apes/frames/gibbon-192x256.png" },
			mipLevel{ vec2{384.0f, 512.0f}, "./gfx/apes/frames/gibbon-384x512.png" }
			}}},
		{"./gfx/bananas/single-banana-base-64x64.png", advancedGfx{{
			mipLevel{ vec2{16.0f, 16.0f}, "./gfx/bananas/frames/bananas-16x16.png" },
			mipLevel{ vec2{32.0f, 32.0f}, "./gfx/bananas/frames/bananas-32x32.png" },
			mipLevel{ vec2{64.0f, 64.0f}, "./gfx/bananas/frames/bananas-64x64.png" },
			mipLevel{ vec2{128.0f, 128.0f}, "./gfx/bananas/frames/bananas-128x128.png" },
			mipLevel{ vec2{256.0f, 256.0f}, "./gfx/bananas/frames/bananas-256x256.png" }
			}}},
		{"./gfx/particles/particles-base-32x32.png", advancedGfx{{
			mipLevel{ vec2{8.0f, 8.0f}, "./gfx/particles/frames/particles-8x8.png" },
			mipLevel{ vec2{16.0f, 16.0f}, "./gfx/particles/frames/particles-16x16.png" },
			mipLevel{ vec2{32.0f, 32.0f}, "./gfx/particles/frames/particles-32x32.png" },
			mipLevel{ vec2{64.0f, 64.0f}, "./gfx/particles/frames/particles-64x64.png" },
			mipLevel{ vec2{128.0f, 128.0f}, "./gfx/particles/frames/particles-128x128.png" },
			}}},
	};

	// ==================================================

	// Other object value lookups:

	std::vector<vec2> GravityFieldDirections{ UP, RIGHT, -UP, -RIGHT };

	// ==================================================

	// Object templates:

	objectRecipe BananaTemplate
	{
		.Id = -1,
		.Name = "BANANA",
		.Class = "PROJECTILE",
		.Filename = "./gfx/bananas/single-banana-base-64x64.png",
		.Size = {2.0f, 2.0f},
	};

	// ==================================================

	// Particle templates:

	particleRecipe ParticleTemplate_BananaReticle
	{
		.ParticleCount = 1,
		.ParticleTag = "SINGLE_BANANA_WHITE",
		.WorldSize = {1.75f, 1.75f}
	};

	particleRecipe ParticleTemplate_RandomGlyphBurst
	{
		.ParticleCount = 15,
		.ParticleTag = "RANDOM_GLYPH",
		.VelocityDampening = 0.1f,
		.Speed = {30.0f, 35.0f},
		.MovementAngle = { -2.0f * PI, 2.0f * PI},
		.RotationStartingAngle = { -2.0f * PI, 2.0f * PI},
		.RotationSpeed = {-4.0f * PI, 4.0f * PI},
		.RotationDampening = 0.1f,
		.WorldSize = {2.0f, 2.0f},
		.TimeBeforeDeletion = {0.3f, 0.45f}
	};

	particleRecipe ParticleTemplate_SmallDotsAndCirclesBurst
	{
		.ParticleCount = 12,
		.ParticleTag = "CIRCLES_AND_DOTS",
		.VelocityDampening = 0.1f,
		.Speed = {18.0f, 22.5f},
		.MovementAngle = { -2.0f * PI, 2.0f * PI},
		.TintTag = "RANDOM",
		.WorldSize = {1.75f, 1.75f},
		.TimeBeforeDeletion = {0.15f, 0.25f},
	};

	particleRecipe ParticleTemplate_StarBurst
	{
		.ParticleCount = 25,
		.ParticleTag = "STAR",
		.VelocityDampening = 0.1f,
		.Speed = {12.0f, 20.0f},
		.MovementAngle = { -2.0f * PI, 2.0f * PI},
		.RotationStartingAngle = { -2.0f * PI, 2.0f * PI},
		.RotationSpeed = {-4.0f * PI, 4.0f * PI},
		.RotationDampening = 0.2f,
		.TintTag = "RANDOM",
		.WorldSize = {5.0f, 5.0f},
		.TimeBeforeDeletion = {0.1f, 0.55f}
	};

	// ==================================================

	// Animation lambdas:

	auto NextFrame_BabyGorilla = [](objectState& Object)
		{
			/*
			// Frames:
			0: Idle
			1: Jump
			2: Head scratch
			3: Head in hands
			4: Crossed arms 1
			5: Crossed arms 2
			6: Waving
			7: Pointing
			8: Waving arm
			9: Pointing head
			10: Pointing right arm
			11-13: Pointing left arm (1, 2 and 3 bananas)
			*/

			if (Object.AnimationTag == "WAVING")
			{
				Object.AnimationFrameIndex = 6;
				Object.TargetFrameTime = 1.45f;
			}
			else if (Object.AnimationTag == "AIMING")
			{
				Object.AnimationFrameIndex = 7;
				Object.TargetFrameTime = 0.9f;
			}
			else if (Object.AnimationTag == "WAITING")
			{
				Object.AnimationFrameIndex = 0;
				Object.TargetFrameTime = 1.0f;
			}
			else if (Object.AnimationTag == "THINKING")
			{
				Object.AnimationFrameIndex = 2;
				Object.TargetFrameTime = 1.0f;
			}
			else if (Object.AnimationTag == "GOOD_NEWS")
			{
				Object.AnimationFrameIndex = Object.AnimationFrameIndex == 1 ? 0 : 1;
				Object.TargetFrameTime = 0.4f;
			}
			else if (Object.AnimationTag == "BAD_NEWS")
			{
				if (Object.AnimationFrameIndex != 3 && Object.AnimationFrameIndex != 4)
				{
					Object.AnimationFrameIndex = 3;
					Object.TargetFrameTime = 1.5f;
				}
				else
				{
					Object.AnimationFrameIndex = 4;
					Object.TargetFrameTime = 4.0f;
				}
			}
			else if (Object.AnimationTag == "VERY_BAD_NEWS")
			{
				if (Object.AnimationFrameIndex != 3 && Object.AnimationFrameIndex != 5)
				{
					Object.AnimationFrameIndex = 3;
					Object.TargetFrameTime = 1.5f;
				}
				else
				{
					Object.AnimationFrameIndex = 5;
					Object.TargetFrameTime = 4.0f;
				}
			}
			else // ** Assume IDLE
			{
				if (Object.AnimationFrameIndex != 0)
				{
					Object.AnimationFrameIndex = 0;
					Object.TargetFrameTime = 0.95f;
				}
				else
				{
					Object.AnimationFrameIndex = 2;
					Object.TargetFrameTime = 0.85f;
				}
			}
		};


	auto NextFrame_Gibbon = [](objectState& Object)
		{
			/*
			// Frames:
			0: Cround
			1: Jump
			2: Fed 1
			3: Fed 2
			4-6: Just caught banana (1, 2 and 3 bananas)
			7-9: Eating banana (1, 2 and 3 bananas)
			*/

			/*
			CALLING
			CAUGHT_BANANA
			*/

			if (Object.AnimationTag == "CAUGHT_BANANA")
			{
				Object.TargetFrameTime = 1.25f;
				if (Object.AnimationFrameIndex < 2)
				{
					Object.AnimationFrameIndex = 4;
				}
				else if(Object.AnimationFrameIndex >= 4 && Object.AnimationFrameIndex < 7)
				{
					Object.AnimationFrameIndex = 7;
				}
				else if (Object.AnimationFrameIndex >= 7)
				{
					Object.GibbonBananasDiscarded = Object.GibbonBananasEaten;
					Object.GibbonBananasInHand = 0;
					if (Object.GibbonCurrentAppetite > 0)
					{	
						Object.AnimationTag = "CALLING"; // Still hungry
						Object.bEndFrameEarly = true;
					}
					else
					{	
						Object.AnimationFrameIndex = 2; // Full
					}
				}
				else if (Object.AnimationFrameIndex == 2)
				{
					Object.AnimationFrameIndex = 3;
				}
			}
			else // ** Assume CALLING
			{
				Object.AnimationFrameIndex = Object.AnimationFrameIndex == 1 ? 0 : 1;
				Object.TargetFrameTime = 0.3f;
			}
		};

}

#pragma once

#include "types.h"
#include "state.h"

#include "objectRecipes.h"

extern state State;

namespace Particles
{
	std::unordered_map<std::string, olc::vi2d> ParticleTagMap
	{
		{ "SINGLE_BANANA", { 0, 0 }},
		{ "DOUBLE_BANANA", { 1, 0 }},
		{ "TRIPLE_BANANA", { 2, 0 }},

		{ "SINGLE_BANANA_WHITE", { 3, 0 }},
		{ "DOUBLE_BANANA_WHITE", { 4, 0 }},
		{ "TRIPLE_BANANA_WHITE", { 5, 0 }},

		{ "BANANA_PARTICLES", { 0, 1 }}, // ** Will need to randomise

		{ "STAR", { 0, 2 }},
		{ "CIRCLE", { 1, 2 }},
		{ "DOT", { 2, 2 }},
		{ "TRIANGLE", { 3, 2 }},
		{ "HEART", { 4, 2 }}
	};

	void AddParticleFromRecipe(particleRecipe NewRecipe) // Only call from within particles
	{
		for (int i = 0; i < NewRecipe.ParticleCount; ++i)
		{
			float Speed{ std::lerp(NewRecipe.Speed.Low, NewRecipe.Speed.High, float(rand()) / float(RAND_MAX)) };
			float MovementAngle{ std::lerp(NewRecipe.MovementAngle.Low, NewRecipe.MovementAngle.High, float(rand()) / float(RAND_MAX)) };
			float RotationStartingAngle{ std::lerp(NewRecipe.RotationStartingAngle.Low, NewRecipe.RotationStartingAngle.High, float(rand()) / float(RAND_MAX)) };
			float RotationSpeed{ std::lerp(NewRecipe.RotationSpeed.Low, NewRecipe.RotationSpeed.High, float(rand()) / float(RAND_MAX)) };
			float TimeRemaining{ std::lerp(NewRecipe.TimeBeforeDeletion.Low, NewRecipe.TimeBeforeDeletion.High, float(rand()) / float(RAND_MAX)) };

			olc::vi2d Frame{};
			if (ParticleTagMap.contains(NewRecipe.ParticleTag))
			{
				Frame = ParticleTagMap.at(NewRecipe.ParticleTag);
			}
			else
			{
				if (NewRecipe.ParticleTag == "RANDOM_GLYPH")
				{
					Frame = ParticleTagMap.at("STAR");
					Frame.x += (rand() % 5); // **
				}
				if (NewRecipe.ParticleTag == "CIRCLES_AND_DOTS")
				{
					Frame = ParticleTagMap.at("CIRCLE");
					Frame.x += (rand() % 1); // **
				}
				else
				{
					Frame = ParticleTagMap.at("DOT");
				}
			}

			olc::Pixel Tint{ NewRecipe.Tint };
			if(NewRecipe.TintTag == "RANDOM") Tint = olc::Pixel(196 + rand() % 48, 196 + rand() % 48, 32 + rand() % 64);

			particleState NewParticle
			{
				.Position = NewRecipe.SourcePosition,
				.Velocity = NewRecipe.SourceVelocity + Speed * Angle_to_Vector(MovementAngle),
				.VelocityDampening = NewRecipe.VelocityDampening,

				.RotationAngle = RotationStartingAngle,
				.RotationSpeed = RotationSpeed,
				.RotationDampening = NewRecipe.RotationDampening,

				.Frame = Frame,
				.Tint = Tint,
				.WorldSize = NewRecipe.WorldSize,
				.TimeRemaining = TimeRemaining
			};

			State.Game.Level.Particles.push_back(NewParticle);
		}
	};

	void UpdateParticles(float ElapsedTime)
	{
		// Update existing particles:
		for (auto& Particle : State.Game.Level.Particles)
		{
			Particle.Position = Particle.Position + ElapsedTime * Particle.Velocity;
			Particle.Velocity *= (1.0f - ElapsedTime * Particle.VelocityDampening);
			Particle.RotationAngle += ElapsedTime * Particle.RotationSpeed;
			Particle.RotationSpeed *= (1.0f - ElapsedTime * Particle.RotationDampening);

			Particle.TimeRemaining -= ElapsedTime;
		}

		// Remove old particles:
		std::erase_if(State.Game.Level.Particles, [](const auto& p) { return p.TimeRemaining < 0.0f; });

		// Check for new particles:
		for (auto& NewParticleRecipe : State.Game.Level.NewParticleRecipes)
		{
			AddParticleFromRecipe(NewParticleRecipe);
		}
		State.Game.Level.NewParticleRecipes.clear();
	}

	void DrawParticles()
	{
		float MIPS_CHANGE_FACTOR{ 1.0f };
		{
			for (auto& Particle : State.Game.Level.Particles)
			{
				// ** If this is slow, can compromise by not recalculating mip map per particle

				const olc::vf2d WorldSize{ vec2_to_vf2d(Particle.WorldSize) };
				const olc::vf2d Size{ WorldSize * vec2_to_vf2d(TILE_SIZE) };
	
				advancedGfx& AdvancedGfx{ ObjectRecipes::AdvancedGfxMap.at("./gfx/particles/particles-base-32x32.png") }; // **
	
				static bool bDrawingFirstParticle{ true };
				if (bDrawingFirstParticle)
				{
					for (auto& MipMap : AdvancedGfx.MipMap)
					{
						std::string GfxFilename{ "./assets/" + MipMap.Filename };
						MipMap.Gfx = Gfx::Load(GfxFilename, true, true);
					}
					bDrawingFirstParticle = false;
				}
	
				olc::vf2d SizeOnScreen{ State.LevelView.ScaleToScreen(WorldSize) };
				mipLevel MipLevelToUse{};
				MipLevelToUse = AdvancedGfx.MipMap.back();
				for (auto& MipLevel : AdvancedGfx.MipMap)
				{
					if (MIPS_CHANGE_FACTOR * SizeOnScreen.x < MipLevel.FrameSize.x) // ** Only consider x dimension
					{
						MipLevelToUse = MipLevel;
						break;
					}
				}

				olc::vf2d DrawPosition{ vec2_to_vf2d(Particle.Position) };
				olc::vf2d SourcePosition{ (float)Particle.Frame.x * MipLevelToUse.FrameSize.x, (float)Particle.Frame.y * MipLevelToUse.FrameSize.y };
				olc::vf2d SourceSize{ vec2_to_vf2d(MipLevelToUse.FrameSize) };
				olc::vf2d CentreInFrame{ 0.5f * SourceSize };
				olc::vf2d Scale{ Size.x / SourceSize.x, Size.y / SourceSize.y };
				float RotationInRadians = Particle.RotationAngle;
				olc::Pixel Tint{ Particle.Tint };

				State.LevelView.DrawPartialRotatedDecal(DrawPosition, MipLevelToUse.Gfx->Decal(), RotationInRadians,
					CentreInFrame, SourcePosition, SourceSize, Scale, Tint);
			}
		}
	}
};


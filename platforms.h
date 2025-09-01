#pragma once

#include "types.h"
#include "state.h"

#include "bodyHelpers.h"

#include "sfx.h"

extern state State;

namespace Platforms
{
	void CreatePlatform(const platformRecipe& Recipe, std::vector<platformState>& PlatformVector)
	{
		PlatformVector.emplace_back(platformState{});
		platformState& NewPlatform{ PlatformVector.back() };

		NewPlatform.Name = Recipe.Name;
		NewPlatform.Class = Recipe.Class;

		vec2 WorldPosition{ 0.0f, 0.0f }; // ** To do: ideally don't use origin as position
		for (const auto& v : Recipe.Vertices) NewPlatform.Vertices.push_back(v);

		NewPlatform.BodyUserData = new bodyUserData{};
		NewPlatform.BodyUserData->ObjectClass = NewPlatform.Class;
		NewPlatform.Body = BodyHelpers::CreateBody(b2_staticBody, WorldPosition, ZERO, 0.0f, true, NewPlatform.BodyUserData, State.World);

		if (Recipe.Class == "GROUND") // **
		{
			b2ShapeDef ShapeDef{ BodyHelpers::GetGroundShapeDef(entityCategoryId::GROUND) };
			BodyHelpers::AddPolygonToBody({ 0.0f, 0.0f }, Recipe.Vertices, 0.0f, ShapeDef, NewPlatform.Body);
		}
		else
		{
			std::cout << "Invalid platform class " << Recipe.Class << '\n';
		}

		olc::Pixel JUNGLE_GREEN{ 0x0a, 0x48, 0x17, 0xff };
		olc::Pixel JUNGLE_BROWN2{ 0x66, 0x39, 0x31, 0xff };

		olc::Pixel JUNGLE_BROWN_TOPPER{ 0x4d, 0x2d, 0x27 };
		olc::Pixel JUNGLE_BROWN{ 0x3b, 0x21, 0x1c };

		if (Recipe.bIsGroundTopper)
		{
			NewPlatform.Tint = JUNGLE_BROWN_TOPPER;
		}
		else
		{
			NewPlatform.Tint = JUNGLE_BROWN2;
		}
	}

	void UpdatePlatform1(platformState& Platform, float ElapsedTime)
	{
	}

	void ProcessCollisions(platformState& Platform)
	{
	}

	void UpdatePlatform2(platformState& Platform, float ElapsedTime) // Post physics
	{
	}

	void DrawPlatform(const platformState& Platform)
	{
		std::vector<olc::vf2d> Vertices;
		for (auto& v : Platform.Vertices) Vertices.push_back(vec2_to_vf2d(Platform.WorldPosition + v));
		State.LevelView.DrawPolygonDecal(nullptr, Vertices, Vertices, Platform.Tint);
	}
}

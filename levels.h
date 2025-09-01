#pragma once

#include "types.h"
#include "state.h"

#include "collisions.h"
#include "objects.h"
#include "platforms.h"
#include "particles.h"

extern state State;

namespace Levels
{
	// ============================================================================

	void DeleteMarkedObjects()
	{
		auto EraseMarkedObjects = [&](std::vector<objectState>& Objects)
			{
				std::erase_if(Objects, [](objectState& a)
					{
						if (a.bDestroyOnNextUpdate)
						{
							if (b2Body_IsValid(a.Body))b2DestroyBody(a.Body);
							if (a.BodyUserData) delete(a.BodyUserData);
						};
						return a.bDestroyOnNextUpdate;
					});
			};

		EraseMarkedObjects(State.Game.Level.BackgroundObjects);
		EraseMarkedObjects(State.Game.Level.Objects);
		EraseMarkedObjects(State.Game.Level.ForegroundObjects);
	}

	void DeleteAllObjectsAndPlatforms()
	{
		auto EraseAllObjects = [&](std::vector<objectState>& Objects)
			{
				for (auto& Object : Objects)
				{
					if (b2Body_IsValid(Object.Body))b2DestroyBody(Object.Body);
					if (Object.BodyUserData) delete(Object.BodyUserData);
				}
			};

		auto EraseAllPlatforms = [&](std::vector<platformState>& Platforms)
			{
				for (auto& Platform : Platforms)
				{
					if (b2Body_IsValid(Platform.Body))b2DestroyBody(Platform.Body);
					if (Platform.BodyUserData) delete(Platform.BodyUserData);
				}
			};

		EraseAllObjects(State.Game.Level.BackgroundObjects);
		EraseAllPlatforms(State.Game.Level.Platforms);
		EraseAllObjects(State.Game.Level.Objects);
		EraseAllObjects(State.Game.Level.ForegroundObjects);
	}

	// ============================================================================

	void CreateLevelFromRecipe()
	{
		DeleteAllObjectsAndPlatforms();

		State.Game.Level = state::gameState::levelState{};
		levelRecipe& StageRecipe = State.Game.StageRecipe;
		levelRecipe& LevelRecipe = State.Game.LevelRecipe;

		const vec2 LEVEL_BOUND_MARGIN_IN_TILES{ 3.0f, 3.0f }; // Allow a maring, in case of screen shake etc.
		State.Game.Level.LevelCorners = { -0.5f * LevelRecipe.Size + LEVEL_BOUND_MARGIN_IN_TILES, 0.5f * LevelRecipe.Size - LEVEL_BOUND_MARGIN_IN_TILES };

		// Background:
		for (auto& ObjectRecipe : StageRecipe.BackgroundObjectRecipes) Objects::CreateObject(ObjectRecipe, State.Game.Level.BackgroundObjects);
		for (auto& ObjectRecipe : LevelRecipe.BackgroundObjectRecipes) Objects::CreateObject(ObjectRecipe, State.Game.Level.BackgroundObjects);

		// Game:
		for (auto& PlatformRecipe : LevelRecipe.PlatformRecipes) Platforms::CreatePlatform(PlatformRecipe, State.Game.Level.Platforms);
		for (auto& ObjectRecipe : LevelRecipe.ObjectRecipes)
		{
			Objects::CreateObject(ObjectRecipe, State.Game.Level.Objects);
			if (ObjectRecipe.Name == "INSTRUCTIONS-3") State.Game.Level.PositionInstructions3 = ObjectRecipe.Position;
		}

		// Foreground:
		for (auto& ObjectRecipe : StageRecipe.ForegroundObjectRecipes) Objects::CreateObject(ObjectRecipe, State.Game.Level.ForegroundObjects);
		for (auto& ObjectRecipe : LevelRecipe.ForegroundObjectRecipes) Objects::CreateObject(ObjectRecipe, State.Game.Level.ForegroundObjects);

		State.Camera.StartWorldPositionSize = LevelRecipe.CameraStartWorldPositionSize;
	}

	void CreateAnyNewObjects()
	{
		for (auto& NewObjectRecipe : State.Game.Level.NewObjectRecipes) Objects::CreateObject(NewObjectRecipe, State.Game.Level.Objects);
		State.Game.Level.NewObjectRecipes.clear();
	}

	// ============================================================================

	auto GetAllOjectsOfClass(std::string Class)
	{
		auto OjectsOfSelectedClass{ std::ranges::filter_view(State.Game.Level.Objects, [Class](auto& o) { return o.Class == Class; }) };
		return OjectsOfSelectedClass;
	}

	void UpdateActiveGibbonCount()
	{
		int ActiveGibbonCount = 0;
		for (auto& Object : State.Game.Level.Objects) // ** Replace with std count algorithm ??
		{
			if (Object.Class == "GIBBON" && Object.GibbonCurrentAppetite > 0) ++ActiveGibbonCount;
		}
		State.Game.Level.ActiveGibbonCount = ActiveGibbonCount;
	}

	// ============================================================================

	void UpdateObjectsAndPlatforms(float ElapsedTime)
	{
		for (auto& Object : State.Game.Level.BackgroundObjects) Objects::UpdateObject1(Object, ElapsedTime);
		for (auto& Platform : State.Game.Level.Platforms) Platforms::UpdatePlatform1(Platform, ElapsedTime);
		for (auto& Object : State.Game.Level.Objects) Objects::UpdateObject1(Object, ElapsedTime);
		for (auto& Object : State.Game.Level.ForegroundObjects) Objects::UpdateObject1(Object, ElapsedTime);
		DeleteMarkedObjects();
		CreateAnyNewObjects();

		// Update physics and process collisions
		b2World_Step(State.World, ElapsedTime, 4);
		Collisions::ProcessCollisions();
		for (auto& Object : State.Game.Level.BackgroundObjects) Objects::ProcessCollisions(Object);
		for (auto& Platform : State.Game.Level.Platforms) Platforms::ProcessCollisions(Platform);
		for (auto& Object : State.Game.Level.Objects) Objects::ProcessCollisions(Object);
		for (auto& Object : State.Game.Level.ForegroundObjects) Objects::ProcessCollisions(Object);
		DeleteMarkedObjects();
		if (State.Game.Level.bUpdateActiveGibbonCountOnNextFrame)
		{
			UpdateActiveGibbonCount();
			State.Game.Level.bUpdateActiveGibbonCountOnNextFrame = false;
		}

		for (auto& Object : State.Game.Level.BackgroundObjects) Objects::UpdateObject2(Object, ElapsedTime);
		for (auto& Platform : State.Game.Level.Platforms) Platforms::UpdatePlatform2(Platform, ElapsedTime);
		for (auto& Object : State.Game.Level.Objects) Objects::UpdateObject2(Object, ElapsedTime);
		for (auto& Object : State.Game.Level.ForegroundObjects) Objects::UpdateObject2(Object, ElapsedTime);
		Particles::UpdateParticles(ElapsedTime);
	}

	void DrawObjectsAndPlatforms()
	{
		for (auto& Object : State.Game.Level.BackgroundObjects) Objects::DrawObject(Object);
		for (auto& Object : State.Game.Level.Objects) Objects::DrawObject(Object);
		Particles::DrawParticles();
		for (auto& Platform : State.Game.Level.Platforms) Platforms::DrawPlatform(Platform);
		for (auto& Object : State.Game.Level.ForegroundObjects) Objects::DrawObject(Object);
	}
}


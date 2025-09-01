#pragma once

#include "types.h"
#include "state.h"

#include "bodyHelpers.h"
#include "objectRecipes.h"
#include "geometry.h"

#include "gfx.h"
#include "sfx.h"

extern state State;

namespace Objects
{
	constexpr float GRAVITY{ 17.5f };
	constexpr float GRAVITY_EXTRA_FORCE_MULTIPLIER{ 2.5f };
	constexpr float GRAVITY_FIELD_MULTIPLIER{ 3.5f };

	const float AIM_ROTATION_BASE_SPEED{ 0.85f * PI };
	const float AIM_ROTATION_MULTIPLIER_WHEN_PREPARING_TO_THROW{ 0.1f };
	const float AIM_COOLDOWN_BASE_TIME{ 1.5f };
	const float AIM_BAD_NEWS_MULTIPLIER{ 2.0f };
	const float AIM_WOBBLE_BASE_STRENGTH{ 0.03f };
	const float AIM_WOBBLE_FREQUENCY{ 30.0f };

	const float TIME_TO_REACH_MAXIMUM_STRENGTH_THROW{ 1.2f };
	const float TIME_LIMIT_AT_MAXIMUM_STRENGTH_THROW{ 0.75f };
	const float PROJECTILE_STARTING_DISTANCE_FROM_PLAYER{ 1.0f };
	const float PROJECTILE_BASE_MIN_SPEED{ 8.0f };
	const float PROJECTILE_BASE_MAX_SPEED{ 48.0f };

	// ============================================================================

	objectState& CreateObject(const objectRecipe& Recipe, std::vector<objectState>& ObjectVector)
	{
		ObjectVector.emplace_back(objectState{});
		objectState& NewObject{ ObjectVector.back() };

		NewObject.Name = Recipe.Name;
		NewObject.Class = Recipe.Class;

		bool bClamped = { Recipe.Class != "GRAVITY_FIELD" }; // ** Jam hacky ... last hour of the jam

		if (ObjectRecipes::AdvancedGfxMap.contains(Recipe.Filename))
		{
			NewObject.AdvancedGfx = ObjectRecipes::AdvancedGfxMap.at(Recipe.Filename);
			for (auto& MipMap : NewObject.AdvancedGfx.MipMap)
			{
				std::string GfxFilename{ "./assets/" + MipMap.Filename };
				MipMap.Gfx = Gfx::Load(GfxFilename, true, bClamped);
			}
		}
		else
		{
			std::string GfxFilename{ "./assets/" + Recipe.Filename };
			std::shared_ptr<olc::Renderable> NewGfx{ Gfx::Load(GfxFilename, true, bClamped) };
			vec2 NewGfxSize = vec2{ (float)NewGfx->Sprite()->width, (float)NewGfx->Sprite()->height };
			NewObject.AdvancedGfx.MipMap.push_back({ NewGfxSize, GfxFilename, NewGfx });
		}

		NewObject.BodyUserData = new bodyUserData{};
		NewObject.BodyUserData->ObjectClass = NewObject.Class;
		NewObject.BodyUserData->Value = Recipe.Value;

		NewObject.WorldPosition = Recipe.Position;
		NewObject.WorldVelocity = Recipe.Velocity;
		NewObject.WorldSize = Recipe.Size;
		NewObject.WorldRotation = Recipe.Rotation;
		NewObject.HFlip = Recipe.bHorizontalFlip;

		NewObject.SourceCentre = ZERO; // ** To do: Set based on POI ??
		NewObject.ParallaxFactor = Recipe.ParallaxFactor;

		b2ShapeDef ShapeDef{};
		if (Recipe.Class == "PLAYER") // **
		{
			NewObject.Body = BodyHelpers::CreateBody(b2_kinematicBody, Recipe.Position, Recipe.Velocity, Recipe.Rotation, true, NewObject.BodyUserData, State.World);
			ShapeDef = BodyHelpers::GetPlayerShapeDef();
		}
		else if (Recipe.Class == "GIBBON") // **
		{
			NewObject.bIsBaby = (Recipe.Filename == "./gfx/apes/baby-gibbon-base-96x96.png");
			NewObject.GibbonTotalAppetite = std::min(Recipe.Value + State.Game.TimesLevelsWrappedAround, 5); 
			NewObject.GibbonCurrentAppetite = NewObject.GibbonTotalAppetite;
			NewObject.BodyUserData->Value = NewObject.GibbonTotalAppetite;
			NewObject.Body = BodyHelpers::CreateBody(b2_kinematicBody, Recipe.Position, Recipe.Velocity, Recipe.Rotation, true, NewObject.BodyUserData, State.World);
			ShapeDef = BodyHelpers::GetGibbonShapeDef();
		}
		else if (Recipe.Class == "PROJECTILE") // **
		{
			NewObject.ProjectileStrength = std::clamp(Recipe.Value, 1, 3); // **
			NewObject.AnimationFrameIndex = NewObject.ProjectileStrength - 1;
			NewObject.Body = BodyHelpers::CreateBody(b2_dynamicBody, Recipe.Position, Recipe.Velocity, Recipe.Rotation, false, NewObject.BodyUserData, State.World);
			ShapeDef = BodyHelpers::GetProjectileShapeDef();

			State.Camera.BodiesToTrack.push_back(NewObject.Body);
		}
		else if (Recipe.Class == "PIN") // **
		{
			NewObject.PinTag = "NORMAL";
			if (Recipe.Filename == "./gfx/level-objects/glass-pin-base-64x64.png") NewObject.PinTag = "GLASS";
			if (Recipe.Filename == "./gfx/level-objects/double-glass-pin-base-64x64.png") NewObject.PinTag = "DOUBLE_GLASS"; // ** TO DO
			if (Recipe.Filename == "./gfx/level-objects/bumper-pin-base-128x128.png") NewObject.PinTag = "BUMPER"; // ** TO DO
			NewObject.Body = BodyHelpers::CreateBody(b2_kinematicBody, Recipe.Position, Recipe.Velocity, Recipe.Rotation, false, NewObject.BodyUserData, State.World);
			ShapeDef = BodyHelpers::GetPinShapeDef();
		}
		else if (Recipe.Class == "GRAVITY_FIELD") // **
		{
			NewObject.GravityFieldDirectionIndex = Recipe.Value;
			NewObject.Body = BodyHelpers::CreateBody(b2_kinematicBody, Recipe.Position, Recipe.Velocity, Recipe.Rotation, true, NewObject.BodyUserData, State.World);
			ShapeDef = BodyHelpers::GetGravityFieldShapeDef();
		}
		else if (Recipe.Class == "STAGE") // **
		{
		}
		else
		{
//			std::cout << "Invalid object class " << Recipe.Class << '\n';
		}

		if (b2Body_IsValid(NewObject.Body))
		{
			if (State.Game.LevelRecipe.ColliderRecipes.contains(Recipe.Filename))
			{
				colliderRecipe& ColliderRecipe{ State.Game.LevelRecipe.ColliderRecipes.at(Recipe.Filename) };

				for (auto& Polygon : ColliderRecipe.Polygons)
				{
					std::vector<vec2> Vertices{};
					for (auto& Vertex : Polygon.Vertices)
					{
						Vertices.push_back({ Vertex.x * NewObject.WorldSize.x, Vertex.y * NewObject.WorldSize.y });
					}
					BodyHelpers::AddPolygonToBody({ 0.0f, 0.0f }, Vertices, 0.0f, ShapeDef, NewObject.Body);
				}

				for (auto& Circle : ColliderRecipe.Circles)
				{
					vec2 ScaledPosition{ Circle.Position.x * NewObject.WorldSize.x, Circle.Position.y * NewObject.WorldSize.y };
					float ScaledRadius{ Circle.Radius * NewObject.WorldSize.x };
					BodyHelpers::AddCircleToBody(ScaledPosition, ScaledRadius, ShapeDef, NewObject.Body);
				}
			}
			else
			{
				b2Vec2 TL{ -0.5f * Recipe.Size };
				rectCorners RectCorners{ TL, -TL };
				BodyHelpers::AddBoxToBody(RectCorners, ShapeDef, NewObject.Body);
			}
		}

		if (Recipe.MovementRecipes.size() > 0)
		{
			NewObject.StartingWorldPosition = NewObject.WorldPosition;

			float CumulativeTime{ 0.0f };
			vec2 CumulativeRelativePosition{ 0.0f, 0.0f };
			float CumulativeRotation{ 0.0f };
			for (auto& m : Recipe.MovementRecipes)
			{
				objectState::movementLeg NewLeg{};
				NewLeg.StartTime = CumulativeTime;
				NewLeg.LegTime = m.Time;
				NewLeg.StartPosition = CumulativeRelativePosition;
				NewLeg.Velocity = 1.0f / m.Time * m.Move;
				NewLeg.StartRotation = CumulativeRotation;
				NewLeg.RotationSpeed = 1.0f / m.Time * m.Rotation;
				NewLeg.EasingId = m.EasingId;

				NewObject.KineticMovementLegs.push_back(NewLeg);

				CumulativeTime += NewLeg.LegTime;
				CumulativeRelativePosition += m.Move;
				CumulativeRotation += m.Rotation;
			}
		}

		return NewObject;
	}

	// ============================================================================

	void SetAnimationTag(objectState& Object, std::string NewTag)
		{
			Object.AnimationTag = NewTag;
			Object.bEndFrameEarly = true;
		};

	void DisableAllCollisionsForObject(objectState& Object)
	{
		if (b2Body_IsValid(Object.Body))
		{
			b2ShapeId ShapeIds[10]; // ** Jam hacky .. shortcut
			int returnCount = b2Body_GetShapes(Object.Body, ShapeIds, 10); // ** Jam hacky .. shortcut
			for (int i = 0; i < returnCount; ++i)
			{
				b2ShapeId& ShapeId = ShapeIds[i];
				b2Shape_SetFilter(ShapeId, BodyHelpers::GetPlayerShapeDef().filter); // ** Jam hacky .. shortcut
			}
		}
	};

	// ============================================================================

	void UpdateObject1(objectState& Object, float ElapsedTime)
	{
		// Class specific updates:
		if (Object.Class == "PLAYER") // **
		{
			playerInputs& Inputs = State.PlayerInputs.at(Object.ControllerIndex);

			if (State.Game.GameStateId != gameStateId::PLAYING && State.Game.GameStateId != gameStateId::CLEARED_TUTORIAL)
			{
				Object.bPreparingThrow = false;
				Object.TimeStartedPreparingToThrow = 0.0;
				Object.bAimCooldownRemaining = 0.0f;
				Object.CurrentAimWobble = 0.0f;
			}
			if (State.Game.CurrentLevelIndex <= 3 && State.Game.CurrentLevelIndex != 2)
			{
				if (State.Game.CurrentLevelIndex == 0)
				{
					if (Object.AnimationTag != "WAVING") Objects::SetAnimationTag(Object, "WAVING");
				}
				else if (State.Game.CurrentLevelIndex == 1)
				{
					if (Object.AnimationTag != "VERY_BAD_NEWS") Objects::SetAnimationTag(Object, "VERY_BAD_NEWS");
					Object.AnimationFrameIndex = 5;
				}
				else
				{
					if (Object.AnimationTag != "THINKING") Objects::SetAnimationTag(Object, "THINKING");
				}
			}
			else if (State.Camera.CameraStateTag == "TRACK_BODIES" && Object.bAimCooldownRemaining <= 0.0f)
			{
				State.ShowAppMessage("Press SPACE to Skip", 2.0f);
				if (Inputs.IsPressedOrHeld(inputID::FACE_DOWN))
				{
					Object.bHasButtonAirGapBeforeThrowing = false;
					State.Camera.CameraStateTag = "RETURN_TO_START";
				}
			}
			else if (!Object.bHasButtonAirGapBeforeThrowing)
			{
				State.HideAppMessage();
				if (!Inputs.IsPressedOrHeld(inputID::FACE_DOWN)) Object.bHasButtonAirGapBeforeThrowing = true;
			}
			else if(Object.bPreparingThrow || Object.bAimCooldownRemaining <= 0.0f)
			{
				State.HideAppMessage();

				vec2 Move = ZERO;
				if (Inputs.IsPressedOrHeld(inputID::DPAD_LEFT)) Move.x -= 1.0f;
				if (Inputs.IsPressedOrHeld(inputID::DPAD_RIGHT)) Move.x += 1.0f;
				if (Object.bPreparingThrow) Move.x *= AIM_ROTATION_MULTIPLIER_WHEN_PREPARING_TO_THROW;

				Object.CurrentAimAngle += Move.x * AIM_ROTATION_BASE_SPEED * Object.AimRotationSpeedMultiple * ElapsedTime;
				if (Object.bLoopAimBounds)
				{
					float AimBoundsLength{ Object.AimBoundsMinMax.second - Object.AimBoundsMinMax.first };
					while (Object.CurrentAimAngle < Object.AimBoundsMinMax.first) Object.CurrentAimAngle += AimBoundsLength;
					while (Object.CurrentAimAngle > Object.AimBoundsMinMax.second) Object.CurrentAimAngle -= AimBoundsLength;
				}
				else
				{
					Object.CurrentAimAngle = std::clamp(Object.CurrentAimAngle, Object.AimBoundsMinMax.first, Object.AimBoundsMinMax.second);
				}

				Object.CurrentAimWobble = 0.0f; // Reset each frame
				Object.ThrowStengthAsProportionMaximum = 0.0f; // Reset each frame

				if (Object.AnimationTag != "AIMING") Objects::SetAnimationTag(Object, "AIMING");

				if (!Object.bPreparingThrow && Inputs.IsPressedOrHeld(inputID::FACE_DOWN))
				{
					Object.bPreparingThrow = true;
					Object.TimeStartedPreparingToThrow = State.CumulativeTime;
				}

				if (Object.bPreparingThrow)
				{
					double TimePreparingToThrow{ State.CumulativeTime - Object.TimeStartedPreparingToThrow };

					if (TimePreparingToThrow > TIME_LIMIT_AT_MAXIMUM_STRENGTH_THROW + TIME_TO_REACH_MAXIMUM_STRENGTH_THROW)
					{
						Object.bPreparingThrow = false;

						Object.bAimCooldownRemaining = AIM_COOLDOWN_BASE_TIME * AIM_BAD_NEWS_MULTIPLIER;
						Objects::SetAnimationTag(Object, "BAD_NEWS");
					}
					else
					{
						Object.ThrowStengthAsProportionMaximum = Easing(easingId::EASE_OUT, std::clamp((float)(TimePreparingToThrow / TIME_TO_REACH_MAXIMUM_STRENGTH_THROW), 0.0f, 1.0f));

						if (TimePreparingToThrow > TIME_TO_REACH_MAXIMUM_STRENGTH_THROW + Object.TimeAtMaxStrengthBeforeWobble)
						{
							float TimeWobbling{ (float)std::max(0.0, TimePreparingToThrow - (TIME_TO_REACH_MAXIMUM_STRENGTH_THROW + Object.TimeAtMaxStrengthBeforeWobble)) };
							float WobbleStrength{ AIM_WOBBLE_BASE_STRENGTH * (1.0f + TimeWobbling)};
							Object.CurrentAimWobble = WobbleStrength * sinf(TimeWobbling * AIM_WOBBLE_FREQUENCY); // **
						}

						if (!Inputs.IsPressedOrHeld(inputID::FACE_DOWN))
						{
							Object.bPreparingThrow = false;

							objectRecipe NewBananaRecipe{ ObjectRecipes::BananaTemplate };
							vec2 LaunchVector{ Angle_to_Vector(Object.CurrentAimAngle + Object.CurrentAimWobble - 0.5f * PI) }; // Adjusting as: Aim is zero == up; Vector is zero == left
							NewBananaRecipe.Position = Object.WorldPosition + PROJECTILE_STARTING_DISTANCE_FROM_PLAYER * LaunchVector;
							NewBananaRecipe.Velocity = std::lerp(PROJECTILE_BASE_MIN_SPEED, PROJECTILE_BASE_MAX_SPEED * Object.ThrowSpeedMultiple, Object.ThrowStengthAsProportionMaximum)
								* LaunchVector;
							State.Game.Level.NewObjectRecipes.push_back(NewBananaRecipe);

							State.Camera.CameraStateTag = "TRACK_BODIES";
							State.Camera.CameraSpeedTag = "URGENT";
							State.Camera.BodiesToTrack.clear();
							State.Camera.BodiesToTrack.push_back(Object.Body);

							Object.bAimCooldownRemaining = AIM_COOLDOWN_BASE_TIME * Object.AimCooldownMultiple;
							Objects::SetAnimationTag(Object, "THINKING");

							Sfx::PlaySfx(SfxID::THROW, 0.6f);
						}
					}
				}

			}
			else
			{
				Object.bAimCooldownRemaining -= ElapsedTime;
				Object.CurrentAimWobble = 0.0f; // **
				Object.bHasButtonAirGapBeforeThrowing = false;
			}

			if (State.Game.GameStateId == gameStateId::CLEARED_TUTORIAL)
			{
				if (Object.AnimationTag != "WAVING") Objects::SetAnimationTag(Object, "WAVING"); // ** NOT ANIMATING - NOT SURE WHY
			}
		}
		else if (Object.Class == "GIBBON") // **
		{
		}
		else if (Object.Class == "PROJECTILE") // **
		{
			vec2 ObjectVelocity{ b2Body_GetLinearVelocity(Object.Body) };
			vec2 GravityForce{ -GRAVITY * b2Body_GetMass(Object.Body) * UP };
			if (Object.bThisProjectileJustThrown)
			{
				if (ObjectVelocity.y > 0.0f) Object.bThisProjectileJustThrown = false;
				if (Object.CountGravityFieldOverlaps > 0) Object.bThisProjectileJustThrown = false;
			}
			else
			{
				if (Object.CountGravityFieldOverlaps > 0)
				{
					vec2 GravityFieldDirection{ ObjectRecipes::GravityFieldDirections.at(Object.GravityFieldDirectionIndex) };
					GravityForce = GRAVITY_FIELD_MULTIPLIER * GRAVITY * b2Body_GetMass(Object.Body) * GravityFieldDirection;
				}
				else
				{
					GravityForce *= GRAVITY_EXTRA_FORCE_MULTIPLIER;
				}
			}
			
//			if (Object has a player controller attached to it ... responds to after touch ...)
			{
				// ** Jam hacky .. out of time to implement
			}

			b2Body_ApplyForceToCenter(Object.Body, GravityForce, true);
		}
		else if (Object.Class == "STAGE") // **
		{
		}
		else if (Object.Class == "PIN") // **
		{
		}
		else if (Object.Class == "GRAVITY_FIELD") // **
		{
		}
		else
		{
//			std::cout << "Invalid object class " << Object.Class << '\n';
		}

		// Apply kinetic movement:
		if (Object.KineticMovementLegs.size() > 0)
		{
			Object.KineticMovementTimer += ElapsedTime;
			objectState::movementLeg* CurrentLeg = &Object.KineticMovementLegs[Object.CurrentMovementLegIndex];

			auto CalculateMovementPositionAndRotation = [](objectState::movementLeg* CurrentLeg, float MovementTimer)
				{
					float PercentageNow{ MovementTimer / CurrentLeg->LegTime };
					PercentageNow = std::clamp(PercentageNow, 0.0f, 1.0f);
					PercentageNow = Easing(CurrentLeg->EasingId, PercentageNow);
					vec2 MovementPosition{ CurrentLeg->StartPosition + PercentageNow * CurrentLeg->LegTime * CurrentLeg->Velocity };
					float MovementRotation{ CurrentLeg->StartRotation + PercentageNow * CurrentLeg->LegTime * CurrentLeg->RotationSpeed };
					return std::pair<vec2, float>{MovementPosition, MovementRotation};
				};

			if (Object.KineticMovementTimer >= CurrentLeg->LegTime)
			{
				Object.KineticMovementTimer -= CurrentLeg->LegTime;
				Object.CurrentMovementLegIndex = (Object.CurrentMovementLegIndex + 1) % Object.KineticMovementLegs.size();
				CurrentLeg = &Object.KineticMovementLegs[Object.CurrentMovementLegIndex];

				auto MovementPositionAndRotation{ CalculateMovementPositionAndRotation(CurrentLeg, Object.KineticMovementTimer) };
				vec2 MovementTargetPosition{ Object.StartingWorldPosition + MovementPositionAndRotation.first };
				b2Rot MovementTargetRotation{ b2MakeRot(MovementPositionAndRotation.second) };
				b2Body_SetTransform(Object.Body, MovementTargetPosition, MovementTargetRotation);
			}

			auto CurrentTargetPositionAndRotation{ CalculateMovementPositionAndRotation(CurrentLeg, Object.KineticMovementTimer) };
			auto NextTargetPositionAndRotation{ CalculateMovementPositionAndRotation(CurrentLeg, Object.KineticMovementTimer + ElapsedTime) };
			vec2 TargetVelocity{ 1.0f / ElapsedTime * (NextTargetPositionAndRotation.first - CurrentTargetPositionAndRotation.first) };
			float TargetRotationSpeed{ 1.0f / ElapsedTime * (NextTargetPositionAndRotation.second - CurrentTargetPositionAndRotation.second) };
			b2Body_SetLinearVelocity(Object.Body, TargetVelocity);
			b2Body_SetAngularVelocity(Object.Body, TargetRotationSpeed);
		}
	}

	// ============================================================================

	void ProcessCollisions(objectState& Object)
	{
		if (Object.Class == "PLAYER") // **
		{
		}
		else if (Object.Class == "GIBBON") // **
		{
			for (auto& CollisionMessage : Object.BodyUserData->CollisionMessages)
			{
				if (CollisionMessage.second.OtherObjectClass == "PROJECTILE" && Object.GibbonCurrentAppetite > 0)
				{
					Object.GibbonBananasEaten = std::min(Object.GibbonTotalAppetite, Object.GibbonBananasEaten + CollisionMessage.second.OtherObjectValue);
					Object.GibbonCurrentAppetite = std::max(0, Object.GibbonCurrentAppetite - CollisionMessage.second.OtherObjectValue);
					int NewBananasToEat{ CollisionMessage.second.OtherObjectValue };
					while (NewBananasToEat > 0)
					{
						--NewBananasToEat;
						++Object.GibbonBananasInHand;
						State.Game.Level.CurrentVolume = std::max(0.0f, State.Game.Level.CurrentVolume - State.Game.Level.VolumeDecreasePerHitGibbon);

						State.Game.Level.NewParticleRecipes.emplace_back(particleRecipe{ ObjectRecipes::ParticleTemplate_StarBurst });
						vec2 ParticleSourceOffset{ Object.bIsBaby ? vec2{ -1.5f, -0.75f } : vec2{ -1.75, 0.25 } }; //**
						State.Game.Level.NewParticleRecipes.back().SourcePosition = { Object.WorldPosition + ParticleSourceOffset };

						Sfx::PlaySfx(SfxID::GIBBON_HAPPY, 0.5f);
					}
					while (Object.GibbonBananasInHand > 3)
					{
						Object.GibbonBananasInHand = std::max(0, Object.GibbonBananasInHand - 1);
						Object.GibbonBananasDiscarded = std::min(Object.GibbonTotalAppetite, Object.GibbonBananasDiscarded + 1); // **
					}

					if (Object.GibbonCurrentAppetite == 0)DisableAllCollisionsForObject(Object);

					Object.BodyUserData->Value = Object.GibbonCurrentAppetite;
					State.Game.Level.bUpdateActiveGibbonCountOnNextFrame = true;

					if (Object.AnimationTag != "CAUGHT_BANANA")
					{
						State.Camera.BodiesToTrack.push_back(Object.Body);
						Object.AnimationTag = "CAUGHT_BANANA";
						Object.bEndFrameEarly = true;
					}
				}
			}
		}
		else if (Object.Class == "PROJECTILE") // **
		{
			for (auto& CollisionMessage : Object.BodyUserData->CollisionMessages)
			{
				if (CollisionMessage.second.OtherObjectClass == "GIBBON" && CollisionMessage.second.OtherObjectValue > 0)
				{
					Object.bDestroyOnNextUpdate = true;
				}
				else if (CollisionMessage.second.OtherObjectClass == "PIN" || CollisionMessage.second.OtherObjectClass == "GROUND")
				{
					if (b2Length(Object.WorldVelocity) > 7.0f) // Note this is still "previous velocity"
					{
						State.Game.Level.NewParticleRecipes.emplace_back(particleRecipe{ ObjectRecipes::ParticleTemplate_SmallDotsAndCirclesBurst }); // **
						State.Game.Level.NewParticleRecipes.back().SourcePosition = { Object.WorldPosition }; // **

						Sfx::PlaySfx(SfxID::BANANA_BOUNCE, std::min(1.0f, (b2Length(Object.WorldVelocity) - 7.0f) * 0.075f));
					}
				}
				else if (CollisionMessage.second.OtherObjectClass == "GRAVITY_FIELD")
				{
					Object.GravityFieldDirectionIndex = CollisionMessage.second.OtherObjectValue;
					Object.CountGravityFieldOverlaps += CollisionMessage.second.OtherObjectValue2;
				}
			}
		}
		else if (Object.Class == "PIN") // **
		{
			for (auto& CollisionMessage : Object.BodyUserData->CollisionMessages)
			{
				if (CollisionMessage.second.OtherObjectClass == "PROJECTILE")
				{
					if (Object.PinTag == "GLASS")
					{
						// Trigger glass smash
						State.Game.Level.NewParticleRecipes.emplace_back(particleRecipe{ ObjectRecipes::ParticleTemplate_RandomGlyphBurst }); // **
						State.Game.Level.NewParticleRecipes.back().SourcePosition = { Object.WorldPosition }; // **
						DisableAllCollisionsForObject(Object);
						Object.PinTag = "SMASHED";
					}
					else if (Object.PinTag == "DOUBLE_GLASS")
					{
						// Trigger glass smash
						State.Game.Level.NewParticleRecipes.emplace_back(particleRecipe{ ObjectRecipes::ParticleTemplate_RandomGlyphBurst }); // **
						State.Game.Level.NewParticleRecipes.back().SourcePosition = { Object.WorldPosition }; // **
						Object.PinTag = "GLASS";
					}
					else if (Object.PinTag == "BUMPER")
					{
					}
				}
			}
		}
		else if (Object.Class == "GRAVITY_FIELD") // **
		{
		}
		else if (Object.Class == "STAGE") // **
		{
		}
		else
		{
//			std::cout << "Invalid object class " << Object.Class << '\n';
		}
		Object.BodyUserData->CollisionMessages.clear();
	}

	// ============================================================================

	void UpdateObject2(objectState& Object, float ElapsedTime) // Post physics
	{
		b2BodyId& Body{ Object.Body };

		if (b2Body_IsValid(Body))
		{
			Object.WorldPosition = b2Body_GetPosition(Body);
			Object.WorldVelocity = b2Body_GetLinearVelocity(Body);
			b2Rot BodyRotation = b2Body_GetRotation(Body);
			Object.WorldRotation = b2Rot_GetAngle(BodyRotation);


			if (Object.Class == "PROJECTILE")
			{
				if (!Geometry::IsPointInRect(Object.WorldPosition, posSize{ { 0.0f, 0.0f }, State.Game.LevelRecipe.Size })) // ** Jam hacky .. refers directly to recipe, but ok for this purpose
				{
					Object.bDestroyOnNextUpdate = true;
				}

				if (b2LengthSquared(Object.WorldPosition - Object.RecentPosition) < 9.0f) // **
				{
					if (State.CumulativeTime > Object.TimeLastAtRecentPosition + 2.0f) // **
					{
						Object.bIsProjectileStillMoving = false;
					}
				}
				else
				{
					Object.RecentPosition = Object.WorldPosition;
					State.Game.Level.TimeLastMovingBananaSpotted = State.CumulativeTime;
					Object.TimeLastAtRecentPosition = State.CumulativeTime;
					Object.bIsProjectileStillMoving = true;
				}
			}
		}
		
		Object.CurrentFrameTime += ElapsedTime;
		bool bTriggerNewFrame{ false };
		if (Object.CurrentFrameTime > Object.TargetFrameTime)
		{
			Object.CurrentFrameTime -= Object.TargetFrameTime;
			bTriggerNewFrame = true;
		}
		else if(Object.bEndFrameEarly)
		{
			Object.CurrentFrameTime = 0.0f;
			bTriggerNewFrame = true;
			Object.bEndFrameEarly = false;
		}

		if (bTriggerNewFrame)
		{
			if (Object.Class == "PLAYER") ObjectRecipes::NextFrame_BabyGorilla(Object);
			else if (Object.Class == "GIBBON") ObjectRecipes::NextFrame_Gibbon(Object);

			if (Object.Class == "GIBBON" && Object.AnimationFrameIndex == 1)
			{
				--State.Game.Level.GibbonCallsRemainingBeforeSfx;
				if (State.Game.Level.GibbonCallsRemainingBeforeSfx <= 0)
				{
					Sfx::PlaySfx(SfxID::GIBBON_SHOUT, 0.3f);
					State.Game.Level.GibbonCallsRemainingBeforeSfx = 20 + rand() % 25; // **
				}
			}
		}
	}

	// ============================================================================

	void DrawObject(const objectState& Object)
	{
		{
			const float PARALLAX_SCALING_MULT{ 1.0f }; // **

			olc::vf2d WorldScale{ State.LevelView.GetWorldScale() };
			olc::vf2d RecipWorldScale{ State.LevelView.GetRecipWorldScale() };
			olc::vf2d ParallaxScalingFactor{
				(TILE_SIZE.x + PARALLAX_SCALING_MULT * std::min(Object.ParallaxFactor.y, 1.25f) * (WorldScale.x - TILE_SIZE.x)) * RecipWorldScale.x, // ** SIC: referencing "y" for both
				(TILE_SIZE.y + PARALLAX_SCALING_MULT * std::min(Object.ParallaxFactor.y, 1.25f) * (WorldScale.y - TILE_SIZE.y)) * RecipWorldScale.y
			};

			olc::vf2d CentreOfView{ 0.5f * (State.LevelView.GetWorldTL() + State.LevelView.GetWorldBR()) };
			olc::vf2d ParallaxOffset{ ((olc::vf2d{1.0f, 1.0f} - vec2_to_vf2d(Object.ParallaxFactor)) * CentreOfView) };

			olc::vf2d Position{ ParallaxOffset + ParallaxScalingFactor * vec2_to_vf2d(Object.WorldPosition) };
			olc::vf2d Size{ ParallaxScalingFactor * vec2_to_vf2d(TILE_SIZE) * vec2_to_vf2d(Object.WorldSize) };

			olc::vf2d SizeOnScreen{ ParallaxScalingFactor * State.LevelView.ScaleToScreen(vec2_to_vf2d(Object.WorldSize)) };
			mipLevel MipLevelToUse{};
			if (Object.AdvancedGfx.MipMap.size() == 1)
			{
				MipLevelToUse = Object.AdvancedGfx.MipMap.front();
			}
			else if (Object.AdvancedGfx.MipMap.size() > 1)
			{
				MipLevelToUse = Object.AdvancedGfx.MipMap.back();
				for (auto& MipLevel : Object.AdvancedGfx.MipMap)
				{
					if (MIPS_CHANGE_FACTOR * SizeOnScreen.x < MipLevel.FrameSize.x) // ** Only consider x dimension
					{
						MipLevelToUse = MipLevel;
						break;
					}
				}
			}
			else
			{
				std::cout << "Invalid mip map \n";
			}

			auto DrawFrame = [&](int FrameToDraw, olc::vf2d PositionOffset, olc::vf2d CentreOffset, float RotationInRadians, bool ForceHFlip = false)
				{
					olc::vf2d DrawPosition{ Position + PositionOffset * vec2_to_vf2d(Object.WorldSize) };
					olc::vf2d SourcePosition{ (float)FrameToDraw * MipLevelToUse.FrameSize.x, 0.0f };
					olc::vf2d SourceSize{ vec2_to_vf2d(MipLevelToUse.FrameSize) };
					olc::vf2d CentreInFrame{ 0.5f * SourceSize + CentreOffset * SourceSize }; // ** Not picking up centre of object
					olc::vf2d Scale{ ((Object.HFlip || ForceHFlip) ? -1.0f : 1.0f) * Size.x / SourceSize.x, Size.y / SourceSize.y };

					State.LevelView.DrawPartialRotatedDecal(DrawPosition, MipLevelToUse.Gfx->Decal(), RotationInRadians, CentreInFrame, SourcePosition, SourceSize, Scale);
				};

			if (Object.Class == "PLAYER" && Object.AnimationFrameIndex == 6) // Waving
			{
				// Baby gorilla waving drawing: (1.8f animation speed)
				float ProportionOfAnimation = std::abs(2.0f * Object.CurrentFrameTime / Object.TargetFrameTime - 1.0f);
				float EasedProportionOfFrame{ Easing(easingId::EASE_IN_OUT, ProportionOfAnimation) };
				float ArmRotation{std::lerp(-0.1f * PI ,0.12f * PI,  EasedProportionOfFrame) }; // **
				DrawFrame(8, { -0.15f, 0.112f }, { 0.0f, 0.0f }, ArmRotation); // **
				DrawFrame(6, { 0.0f, 0.0f }, { 0.0f, 0.0f }, 0.0f);
			}
			else if (Object.Class == "PLAYER" && Object.AnimationFrameIndex == 7) // Pointing
			{
				bool bForceHFlip{ Object.CurrentAimAngle < 0.0f};

				const std::pair<float, float> RIGHT_ARM_ROTATION_BOUNDS{ 0.0f * PI, 1.0f * PI }; // ** Previously lower bound was 0.075f
				float RIGHT_ARM_ROTATION_LENGTH{ RIGHT_ARM_ROTATION_BOUNDS.second - RIGHT_ARM_ROTATION_BOUNDS.first };
				float RightArmRotation{std::clamp(std::abs(Object.CurrentAimAngle), RIGHT_ARM_ROTATION_BOUNDS.first ,RIGHT_ARM_ROTATION_BOUNDS.second)};

				float ProportionOfRotation = std::abs(RightArmRotation / RIGHT_ARM_ROTATION_LENGTH);
				float EasedProportionOfRotation{ Easing(easingId::EASE_IN_OUT, ProportionOfRotation) };
				float LeftArmRotation{ -std::lerp(-0.12f * PI ,0.12f * PI,  EasedProportionOfRotation) }; // **
				float HeadRotation{ std::lerp(-0.05f * PI ,0.01f * PI,  EasedProportionOfRotation) }; // **
				RightArmRotation -= 0.5f * PI; // Adjusting as: Aim is zero == up; animation is zero == left

				if (bForceHFlip)
				{
					RightArmRotation = -RightArmRotation;
					LeftArmRotation = -LeftArmRotation;
					HeadRotation = -HeadRotation;
				}
				float ForceHSign{ bForceHFlip ? -1.0f : 1.0f };

				DrawFrame(10, { ForceHSign * 0.11f, 0.111f }, { 0.0f, 0.0f }, RightArmRotation, bForceHFlip); // **
				DrawFrame(7, { 0.0f, 0.0f }, { 0.0f, 0.0f }, 0.0f, bForceHFlip);
				DrawFrame(12, { ForceHSign * -0.08f, 0.075f }, { 0.0f, 0.0f }, LeftArmRotation, bForceHFlip); // **
				DrawFrame(9, { ForceHSign * 0.01f, -0.0125f }, { 0.0f, 0.0f }, HeadRotation, bForceHFlip); // **

				// Draw reticle:
				if (Object.bAimCooldownRemaining <= 0.0f) // Object.bPreparingThrow
				{
					vec2 ReticleVector{ Angle_to_Vector(Object.CurrentAimAngle + Object.CurrentAimWobble - 0.5f * PI) }; // Adjusting as: Aim is zero == up; Vector is zero == left
					olc::Pixel ReticleTint{ olc::WHITE };
					if (Object.CurrentAimWobble > 0.0f) ReticleTint = olc::RED;
					float ProportionOfAnimation = std::abs(2.0f * Object.CurrentFrameTime / Object.TargetFrameTime - 1.0f);
					ReticleTint.a = (uint8_t)std::clamp((int)(96.0f + 255.0f * sinf(Easing(easingId::EASE_IN_OUT, ProportionOfAnimation))), 96, 196); // **
					for (int i{ 0 }; i < 3; ++i)
					{
						State.Game.Level.NewParticleRecipes.emplace_back(particleRecipe{ ObjectRecipes::ParticleTemplate_BananaReticle });
						State.Game.Level.NewParticleRecipes.back().SourcePosition = { Object.WorldPosition + (3.0f + 3.0f * (float)(i + 1) * (0.3f + 0.7f * Object.ThrowStengthAsProportionMaximum)) * ReticleVector }; // **
						State.Game.Level.NewParticleRecipes.back().Tint = ReticleTint;
					}
				}
			}
			else
			{
				int AnimationFrameIndexOffset{ 0 };
				if (Object.Class == "GIBBON" && (Object.AnimationFrameIndex == 4 || Object.AnimationFrameIndex == 7))
				{
					AnimationFrameIndexOffset = std::clamp(Object.GibbonBananasInHand - 1, 0, 2);
				}

				if (Object.Class == "GRAVITY_FIELD" ) // ** Jam hacky .. last hour of the jam
				{
					olc::vf2d DrawPosition{ Position  };
					olc::vf2d SourcePosition{ 0.0f, (float)State.CumulativeTime * 20.0f };
					olc::vf2d SourceSize{ 5.0f * vec2_to_vf2d(MipLevelToUse.FrameSize) };
					olc::vf2d CentreInFrame{ 0.5f * SourceSize  }; // ** Jam hacky .. Not picking up centre of object
					olc::vf2d Scale{ Size.x / SourceSize.x, Size.y / SourceSize.y };

					State.LevelView.DrawPartialRotatedDecal(DrawPosition, MipLevelToUse.Gfx->Decal(), Object.BodyUserData->Value * 0.5f * PI, CentreInFrame, SourcePosition, SourceSize, Scale);
				}
				else if (Object.PinTag != "SMASHED") // ** Jam hacky .. quick way to stop drawing smashed pins
				{
					DrawFrame(Object.AnimationFrameIndex + AnimationFrameIndexOffset, { 0.0f, 0.0f }, { 0.0f, 0.0f }, Object.WorldRotation);
				}
			}

			if (Object.Class == "GIBBON")
			{
				if (Object.GibbonBananasEaten > 0)
				{
					advancedGfx& BananaAdvancedGfx{ ObjectRecipes::AdvancedGfxMap.at("./gfx/bananas/single-banana-base-64x64.png") }; // **

					static bool bDrawingFirstDiscardedBanana{ true };
					if (bDrawingFirstDiscardedBanana)
					{
						for (auto& MipMap : BananaAdvancedGfx.MipMap)
						{
							std::string GfxFilename{ "./assets/" + MipMap.Filename };
							MipMap.Gfx = Gfx::Load(GfxFilename, true, true);
						}
						bDrawingFirstDiscardedBanana = false;
					}

					olc::vf2d BananaSize{ vec2_to_vf2d(TILE_SIZE) * vec2_to_vf2d(ObjectRecipes::BananaTemplate.Size) };
					olc::vf2d BananaSizeOnScreen{ State.LevelView.ScaleToScreen(vec2_to_vf2d(ObjectRecipes::BananaTemplate.Size)) };
					mipLevel BananaMipLevelToUse{};
					BananaMipLevelToUse = BananaAdvancedGfx.MipMap.back();
					for (auto& BananaMipLevel : BananaAdvancedGfx.MipMap)
					{
						if (MIPS_CHANGE_FACTOR * BananaSizeOnScreen.x < BananaMipLevel.FrameSize.x) // ** Only consider x dimension
						{
							BananaMipLevelToUse = BananaMipLevel;
							break;
						}
					}

					float INNER_SPACE{ 4.0f / 6.0f };
					float BananaSpacing{ INNER_SPACE / (Object.GibbonTotalAppetite + 1) };
					for (int i{ 0 }; i < Object.GibbonBananasDiscarded; ++i)
					{
						olc::vf2d GibbonSizeInTiles{ vec2_to_vf2d(Object.WorldSize) };
						olc::vf2d BananaSkinPositionOffsetInTiles{ (-0.5f * INNER_SPACE + (float)(i + 1) * BananaSpacing) * GibbonSizeInTiles.x, 0.5f * GibbonSizeInTiles.y - 1.0f }; // **

						int BananaSkinFrameToDraw{ 6 };
						olc::vf2d BananaDrawPosition{ Position + BananaSkinPositionOffsetInTiles };
						olc::vf2d BananaSourcePosition{ (float)BananaSkinFrameToDraw * BananaMipLevelToUse.FrameSize.x, 0.0f };
						olc::vf2d BananaSourceSize{ vec2_to_vf2d(BananaMipLevelToUse.FrameSize) };
						olc::vf2d BananaCentreInFrame{ 0.5f * BananaSourceSize}; // ** Jam hacky .. Not picking up centre of object
						olc::vf2d BananaScale{ (Object.HFlip ? -1.0f : 1.0f) * BananaSize.x / BananaSourceSize.x, BananaSize.y / BananaSourceSize.y };
						float BananaRotationInRadians = 0.0f;

						State.LevelView.DrawPartialRotatedDecal(BananaDrawPosition, BananaMipLevelToUse.Gfx->Decal(), BananaRotationInRadians, 
							BananaCentreInFrame, BananaSourcePosition, BananaSourceSize, BananaScale);
					}
				}
			}

		}
	}

	// ============================================================================

}

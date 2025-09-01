#pragma once

#include "types.h"
#include "state.h"

#include "levels.h"
#include "loadSave.h"
#include "debugDraw.h"

#include "sfx.h"

extern state State;

const int MAX_LEVEL_INDEX{ 10 };

namespace Game
{
	void ResetGame()
	{
		State.Game = state::gameState{};
//		Sfx::PlayMusic(SfxID::MUSIC_MAIN, true);
	}

	void SetNewGameState(gameStateId NewGameState)
	{
		State.Game.GameStateId = NewGameState;
		State.Game.TimeCurrentGameStateStarted = State.CumulativeTime;
		State.Game.bJustEnteredNewGameState = true;
	}

	void Update(float ElapsedTime)
	{
		State.Game.bIsFirstFrameInNewGameState = State.Game.bJustEnteredNewGameState;
		State.Game.bJustEnteredNewGameState = false;

		switch (State.Game.GameStateId)
		{
		// ***********************************************************
		case (gameStateId::START_NEW_GAME):
		{
			State.Game.CurrentLevelIndex = State.Game.StartingLevelIndex - 1;
			State.Game.TotalLevelIndex = State.Game.CurrentLevelIndex;
			State.Game.TimesLevelsWrappedAround = 0;
			SetNewGameState(gameStateId::START_NEW_LEVEL);
			break;
		}

		// ***********************************************************
		case (gameStateId::START_NEW_LEVEL):
		{
			if (State.Game.bIsFirstFrameInNewGameState)
			{
				State.Game.CurrentLevelIndex = State.Game.CurrentLevelIndex + 1;
				++State.Game.TotalLevelIndex;
				if (State.Game.CurrentLevelIndex > MAX_LEVEL_INDEX)
				{
					State.Game.CurrentLevelIndex = State.Game.StartingLevelIndex;
					++State.Game.TimesLevelsWrappedAround;
				}
				State.Game.StartingLevelIndex = std::min(4, State.Game.CurrentLevelIndex);
				std::string LevelFileName{ std::format("level-{}.tmj", State.Game.CurrentLevelIndex) };

				State.Game.StageRecipe = LoadSave::LoadLevelRecipe("jungle-stage.tmj");
				State.Game.LevelRecipe = LoadSave::LoadLevelRecipe(LevelFileName);
				Levels::CreateLevelFromRecipe();
				Levels::UpdateObjectsAndPlatforms(ElapsedTime);

				State.HideAppMessage();
			}

			if (State.CumulativeTime - State.Game.TimeCurrentGameStateStarted > 0.75f) // **
			{
				SetNewGameState(gameStateId::PLAYING);
			}

			break;
		}

		// ***********************************************************
		case (gameStateId::PLAYING):
			if (State.Game.bIsFirstFrameInNewGameState)
			{
				State.Camera.CameraStateTag = "RETURN_TO_START";
				State.Camera.CameraSpeedTag = "RELAXED";
			}
			else if (State.CumulativeTime > State.Game.Level.TimeLastMovingBananaSpotted + 1.2f) // **
			{
				State.Camera.CameraStateTag = "RETURN_TO_START";
				State.Camera.BodiesToTrack.clear();
			}

			if (State.Game.CurrentLevelIndex <= 3)
			{
				// In tutorial:
				playerInputs& Inputs = State.PlayerInputs.at(0); // ** ??
				bool bThrowPressed{ Inputs.IsPressed(inputID::FACE_DOWN) };

				if (State.Game.CurrentLevelIndex == 0 || State.Game.CurrentLevelIndex == 1 || State.Game.CurrentLevelIndex == 3)
				{
					if (State.CumulativeTime - State.Game.TimeCurrentGameStateStarted > 2.0f) // **
					{
						State.ShowAppMessage("Press SPACE", 2.0f);
						if (bThrowPressed)
						{
							SetNewGameState(gameStateId::START_NEW_LEVEL);
							Sfx::PlaySfx(SfxID::BANANA_BOUNCE, 0.6f);
						};
					}
				}
				else if (State.Game.CurrentLevelIndex == 2)
				{
					if (State.Game.Level.ActiveGibbonCount == 0)
					{
						SetNewGameState(gameStateId::CLEARED_TUTORIAL);
					}
				}
				Levels::UpdateObjectsAndPlatforms(ElapsedTime);
			}
			else
			{
				// In main game:
				State.Game.Level.CurrentVolume += State.Game.Level.VolumeFlatIncreaseRate * ElapsedTime * (1.0f + (float)State.Game.TimesLevelsWrappedAround * 0.5f);
				State.Game.Level.CurrentVolume += State.Game.Level.VolumeIncreaseRatePerActiveGibbon * State.Game.Level.ActiveGibbonCount * ElapsedTime;

				Levels::UpdateObjectsAndPlatforms(ElapsedTime);
				if (State.Game.Level.ActiveGibbonCount == 0)
				{
					SetNewGameState(gameStateId::LEVEL_JUST_CLEARED);
				}
				else if (State.Game.Level.CurrentVolume > State.Game.Level.MaxVolume * 1.01f) // ** Max volume grace
				{
					SetNewGameState(gameStateId::GAME_OVER);
				}
			}

			break;

		// ***********************************************************
		case (gameStateId::CLEARED_TUTORIAL):
			{
				playerInputs& Inputs = State.PlayerInputs.at(0); // ** ??
				bool bThrowPressed{ Inputs.IsPressed(inputID::FACE_DOWN) };

				if (State.Game.bIsFirstFrameInNewGameState)
				{
					State.Game.Level.NewObjectRecipes.push_back
					(
						objectRecipe
						{
							.Class = { "STAGE" },
							.Filename = { "./gfx/baby-gorilla-instructions-4.png" },
							.Position{ State.Game.Level.PositionInstructions3 },
							.Size{ 21.5f, 21.5f },
						}
					);
				}

				Levels::UpdateObjectsAndPlatforms(ElapsedTime);

				if (State.CumulativeTime - State.Game.TimeCurrentGameStateStarted > 2.0f) // **
				{
					State.ShowAppMessage("Press SPACE", 2.0f);
					if (bThrowPressed)
					{
						SetNewGameState(gameStateId::START_NEW_LEVEL);
						Sfx::PlaySfx(SfxID::BANANA_BOUNCE, 0.6f);
					};
				}

				break;
			}

			// ***********************************************************
		case (gameStateId::LEVEL_JUST_CLEARED):
			Levels::UpdateObjectsAndPlatforms(ElapsedTime);

			for (auto& PlayerObject : Levels::GetAllOjectsOfClass("PLAYER"))
			{
				Objects::SetAnimationTag(PlayerObject, "GOOD_NEWS");
			}
			SetNewGameState(gameStateId::SCORING_LEVEL);
			break;

		// ***********************************************************
		case (gameStateId::SCORING_LEVEL):
			// ** Animate level scoring
			Levels::UpdateObjectsAndPlatforms(ElapsedTime);

			if (State.CumulativeTime - State.Game.TimeCurrentGameStateStarted > 1.0f) // **
			{
				State.Camera.CameraStateTag = "ZOOM_OUT";

				SetNewGameState(gameStateId::LEVEL_SCORED);
			}
			break;

		// ***********************************************************
		case (gameStateId::LEVEL_SCORED):
			Levels::UpdateObjectsAndPlatforms(ElapsedTime);

			if (State.CumulativeTime - State.Game.TimeCurrentGameStateStarted > 2.0f) // **
			{
				if (State.Game.TotalLevelIndex > 5 && State.Game.TotalLevelIndex % MAX_LEVEL_INDEX == 0)
				{
					SetNewGameState(gameStateId::WIN);
				}
				else
				{
					SetNewGameState(gameStateId::START_NEW_LEVEL);
				}
			}
			break;

		// ***********************************************************
		case (gameStateId::GAME_OVER):
			if (State.Game.bIsFirstFrameInNewGameState)
			{
				State.Camera.CameraStateTag = "ZOOM_OUT";

				State.Game.Level.NewObjectRecipes.push_back
				(
					objectRecipe
					{
						.Class = { "STAGE" },
						.Filename = { "./gfx/apes/gorilla-base-256x256.png" },
						.Position{ -vec2{0.0f, 0.0f} },
						.Size{ 96.0f, 96.0f },
					}
					);
			}

			Levels::UpdateObjectsAndPlatforms(ElapsedTime);
			if (State.CumulativeTime - State.Game.TimeCurrentGameStateStarted > 3.0f) // **
			{
				State.ShowAppMessage("Press SPACE", 5.0f);

				playerInputs& Inputs = State.PlayerInputs.at(0); // ** ??
				bool bThrowPressed{ Inputs.IsPressed(inputID::FACE_DOWN) };

				if (bThrowPressed)
				{
					SetNewGameState(gameStateId::START_NEW_GAME);
					State.Game.bIsReturnToTitleMenu = true;
				};
			}

			break;

		// ***********************************************************
		case (gameStateId::WIN):
			if (State.Game.bIsFirstFrameInNewGameState)
			{
				State.Camera.CameraStateTag = "ZOOM_OUT";

				State.Game.Level.NewObjectRecipes.push_back
				(
					objectRecipe
					{
						.Class = { "STAGE" },
						.Filename = { "./gfx/apes/baby-gorilla-base-96x96.png" },
						.Position{ -vec2{0.0f, 0.0f} },
						.Size{ 96.0f, 96.0f },
					}
					);
			}

			Levels::UpdateObjectsAndPlatforms(ElapsedTime);
			if (State.CumulativeTime - State.Game.TimeCurrentGameStateStarted > 3.0f) // **
			{
				State.ShowAppMessage("Well Done! Press SPACE to Keep Going", 3.0f);

				playerInputs& Inputs = State.PlayerInputs.at(0); // ** ??
				bool bThrowPressed{ Inputs.IsPressed(inputID::FACE_DOWN) };

				if (bThrowPressed)
				{
					SetNewGameState(gameStateId::START_NEW_LEVEL);
				};
			}

			break;
		}
	}

	void DrawGame()
	{
		Levels::DrawObjectsAndPlatforms();

		if (State.bDebugDraw)
		{
			b2World_Draw(State.World, &DebugDraw::DebugDraw);
			for (auto& DebugLine : State.DebugLines)
			{
				State.LevelView.DrawLineDecal(vec2_to_vf2d(DebugLine.Pos1), vec2_to_vf2d(DebugLine.Pos2), DebugLine.Tint);
			}
		}
		State.DebugLines.clear();
	}
}
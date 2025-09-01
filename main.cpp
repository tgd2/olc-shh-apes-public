#define OLC_PGE_APPLICATION
#define PGE_GFX_OPENGL33
#include "olcPixelGameEngine.h"

#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#define OLC_PGEX_SIMPLEFONT
#include "olcPGEX_SimpleBitmapFont.h"

#define OLC_PGE_GAMEPAD
#include "olcPGEX_Gamepad.h"

#define OLC_PGEX_MINIAUDIO
#include "olcPGEX_MiniAudio.h"

#include "types.h"
#include "state.h"

#include "inputs.h"
#include "game.h"
#include "camera.h"
#include "ui.h"

#include "gfx.h"
#include "sfx.h"

state State{};

class shhApes : public olc::PixelGameEngine
{
public:
	shhApes()
	{
		sAppName = "Shh.. Apes!";
	}

public:
	bool OnUserCreate() override
	{
		State.AssetPack = new olc::ResourcePack();
		State.AssetPack->LoadPack("./assets/ShhApps.dat", "Shh.. Apes OLC CODEJAM 2025 PUBLIC REPO");

		State.Create(this);
		olc::GamePad::init();
		Sfx::StartSfx();
		Sfx::LoadAllSamples();

		Game::ResetGame();
		return true;
	}

	void SetNewAppState(appStateId NewAppState)
	{
		State.AppStateId = NewAppState;
		State.TimeCurrentAppStateStarted = State.CumulativeTime;
		State.bJustEnteredNewAppState = true;
	}

	bool OnUserUpdate(float ElapsedTime) override
	{
		if (ElapsedTime > 1.0f / 50.0f) ElapsedTime = 1.0f / 50.0f; // ** To avoid large frame times
		State.CumulativeTime += ElapsedTime;

		Inputs::UpdateInputs();

//		if (GetKey(olc::Key::J).bPressed) Game::SetNewGameState(gameStateId::LEVEL_SCORED);
//		if (GetKey(olc::Key::TAB).bPressed) State.bDebugDraw = !State.bDebugDraw;
		bool bTriggerNextAppState{ GetKey(olc::Key::SPACE).bPressed }; // ** Jam hacky .. referencing space bar directly

		State.bIsFirstFrameInNewAppState = State.bJustEnteredNewAppState;
		State.bJustEnteredNewAppState = false;

		switch (State.AppStateId)
		{
		case(appStateId::SPLASH):
			SetNewAppState(appStateId::MENU);
			break;
		case(appStateId::MENU):
			if (State.bIsFirstFrameInNewAppState)
			{
				State.Game.StageRecipe = LoadSave::LoadLevelRecipe("jungle-stage.tmj");
				State.Game.LevelRecipe = LoadSave::LoadLevelRecipe("title.tmj");
				Levels::CreateLevelFromRecipe();
				State.Camera.CameraStateTag = "RETURN_TO_START";
				State.Camera.CameraSpeedTag = "IMMEDIATE";
				State.ShowAppMessage("Press SPACE to Start", 3.0f);
			}

			Camera::Update(ElapsedTime);
			Game::DrawGame();
			if (bTriggerNextAppState)
			{
				State.Camera.CameraStateTag = "FIXED";
				State.Camera.CameraSpeedTag = "RELAXED";
				SetNewAppState(appStateId::GAME);
				Sfx::PlaySfx(SfxID::BANANA_BOUNCE, 0.6f);
			}
			break;
		case(appStateId::GAME):
			{
				if(State.bDebugDraw)State.LevelView.HandlePanAndZoom(1);
				if (State.bIsFirstFrameInNewAppState)
				{
					Game::SetNewGameState(gameStateId::START_NEW_GAME);
				}

				bool bPauseButtonPressed{ false };
				for (auto& PlayerInputs : State.PlayerInputs) if (PlayerInputs.IsPressed(inputID::START)) bPauseButtonPressed = true;
				if (bPauseButtonPressed) !State.Game.bIsPaused;

				if (!State.Game.bIsPaused)
				{ 
					Game::Update(ElapsedTime);
					Ui::UpdateUi(ElapsedTime);
					if (!State.bDebugDraw) Camera::Update(ElapsedTime);
				}

				Game::DrawGame();
				Ui::DrawUi();

				if (State.Game.bIsReturnToTitleMenu)
				{
					State.Game.bIsReturnToTitleMenu = false;
					SetNewAppState(appStateId::MENU);
				}
				break;
			}
		}

		if (State.bShowAppMessage)
		{
			olc::vf2d MessageSize{ 1.5f * State.AppMessageScale * GetTextSizeProp(State.AppMessage) };
			olc::vf2d Position{ 0.5f * (SCREEN_SIZE.x - MessageSize.x), SCREEN_SIZE.y - 1.5f * MessageSize.y };
			olc::Pixel BANANA_YELLOW(255, 218, 0);

			float AnimationSpeed = 0.8f; // ** Hardcoding
			double PulsePhase = State.CumulativeTime * AnimationSpeed * PI - 1.0 * PI; // ** Hardcoding
			float PulsePercentage{ (float)std::clamp(1.5f - std::sin(PulsePhase), 0.5, 1.0) };
			olc::Pixel TextTint{
					(uint8_t)(PulsePercentage * (float)BANANA_YELLOW.r),
					(uint8_t)(PulsePercentage * (float)BANANA_YELLOW.g),
					(uint8_t)(PulsePercentage * (float)BANANA_YELLOW.b),
					0xff
				};

			DrawStringPropDecal(Position + olc::vf2d{ -2.5f, 2.5f }, State.AppMessage, olc::BLACK, { 1.5f * State.AppMessageScale, 1.5f * State.AppMessageScale });
			DrawStringPropDecal(Position, State.AppMessage, TextTint, { 1.5f * State.AppMessageScale, 1.5f * State.AppMessageScale });
		}

		Sfx::Update();

		return true;
	}

	bool OnUserDestroy() override
	{
		State.Destroy();
		Sfx::EndSfx();
		return true;
	}
};

int main()
{
	shhApes ShhApes;
	if (ShhApes.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 1, false, true))
	{
		ShhApes.Start();
	}
	return 0;
}

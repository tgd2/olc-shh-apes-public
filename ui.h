#pragma once

#include "types.h"
#include "state.h"

#include "levels.h"
#include "loadSave.h"
#include "debugDraw.h"

#include "sfx.h"

extern state State;

namespace Ui
{
	std::shared_ptr<olc::Renderable> SpeakerGfx{};
	std::shared_ptr<olc::Renderable> SpeakerNoColourGfx{};

	void UpdateUi(float ElapsedTime)
	{

		// Jam hacky .. all of this screen shake should be within the camera functions:

		float DrawWidthPerc{ State.Game.Level.CurrentVolume / State.Game.Level.MaxVolume };

		if (DrawWidthPerc > 0.8f)
		{
			float ShakeRate{ std::max(0.05f, (1.0f - DrawWidthPerc)) * 6.0f }; // **
			static float TimeSinceLastShake{ 0.0f };
			bool bShakeThisFrame{ false };

			TimeSinceLastShake += ElapsedTime;
			if (TimeSinceLastShake > ShakeRate)
			{
				bShakeThisFrame = true;
				TimeSinceLastShake = 0.0f;
			}

				if (bShakeThisFrame)
				{
					Sfx::PlaySfx(SfxID::SCREEN_SHAKE, 0.6f);

					state::camera::shakeComponent NewShakeComponent{};
					NewShakeComponent.Frequency = 3.0f; // **
					NewShakeComponent.MaximumOffset = { 1.5f, 1.5f }; // **
					NewShakeComponent.ProportionReductionRate = 2.5f; // **

					State.Camera.ShakeComponents.push_back(NewShakeComponent);

					for (int i = 0; i < 2; ++i) // **
					{
						NewShakeComponent.Frequency *= 0.5f;

						float NewMaximumOffsetLength{ (NewShakeComponent.MaximumOffset).mag() * 0.5f };
						float NewAngle{ 2 * PI * float(rand()) / float(RAND_MAX) };
						NewShakeComponent.MaximumOffset = NewMaximumOffsetLength * olc::vf2d{ cos(NewAngle), sin(NewAngle) };

						State.Camera.ShakeComponents.push_back(NewShakeComponent);
					}
				}

				State.Camera.CumulativeShake = olc::vf2d{ 0.0f, 0.0f };
				for (auto& sc : State.Camera.ShakeComponents)
				{
					State.Camera.CumulativeShake += sinf(sc.ProportionRemaining * sc.ProportionRemaining * 20.0f) * sc.MaximumOffset;
					sc.ProportionRemaining -= sc.ProportionReductionRate * ElapsedTime;
				}

				std::erase_if(State.Camera.ShakeComponents, [](state::camera::shakeComponent sc) { return sc.ProportionRemaining < 0.0f; });
		}
	}

	void DrawUi()
	{

		if (State.Game.TotalLevelIndex >= 3)
		{
			if (!SpeakerGfx) SpeakerGfx = Gfx::Load("./assets/gfx/speaker-512-64.png", true, true);
			if (!SpeakerNoColourGfx) SpeakerNoColourGfx = Gfx::Load("./assets/gfx/speaker-no-colour-512-64.png", true, true);

			float PositionShiftX{ 64.0f };

			float DrawWidthPerc{ State.Game.Level.CurrentVolume / State.Game.Level.MaxVolume };
			float DrawSourceX = 96.0f + DrawWidthPerc * (392.0f - 96.0f);
			float DrawWidth = 512.0f - DrawSourceX;

			State.PGE->DrawDecal({ PositionShiftX + 0.5f * SCREEN_SIZE.x - 256.0f, 8.0f }, SpeakerGfx->Decal(), { 1.0f, 1.0f }, olc::WHITE);
			State.PGE->DrawPartialDecal(olc::vf2d{ PositionShiftX + 0.5f * SCREEN_SIZE.x - 256.0f + DrawSourceX, 8.0f }, olc::vf2d{ DrawWidth, 64.0f }, SpeakerNoColourGfx->Decal(), { DrawSourceX, 0.0f }, olc::vf2d{ DrawWidth, 64.0f }, olc::WHITE);
		}

		if (State.Game.TotalLevelIndex >= 4)
		{
			olc::Pixel BANANA_YELLOW(255, 218, 0);
			std::string Message{ std::format("Level {}", State.Game.TotalLevelIndex - 3) };
			State.PGE->DrawStringPropDecal(olc::vf2d{ 4.0f, 4.0f } + olc::vf2d{ -2.5f, 2.5f }, Message, olc::BLACK, {3.0f, 3.0f});
			State.PGE->DrawStringPropDecal(olc::vf2d{ 4.0f, 4.0f }, Message, BANANA_YELLOW, { 3.0f, 3.0f });
		}

	}

}




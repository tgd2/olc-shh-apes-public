#pragma once

#include "types.h"
#include "state.h"

extern state State;

enum class SfxID
{
//	MUSIC_MAIN = 0,

	THROW,
	BANANA_BOUNCE,
	GIBBON_SHOUT,
	GIBBON_HAPPY,
	SCREEN_SHAKE
};

namespace Sfx
{
	const float VolumeSfx{ 0.5f }; // ** TO DO - Enable Volume to be adjusted
	const float VolumeMusic{ 0.2f }; // ** TO DO - Enable Volume to be adjusted

	const float MIN_TIME_BETWEEN_SAMPLES{ 0.06f };

	void StartSfx()
	{
		State.MiniAudio = new olc::MiniAudio;
	}

	void EndSfx()
	{
	};

	unsigned int Load(std::string Filename)
	{
		State.WavFilenames.push_back("./assets/sfx/" + Filename);
		State.WavsToPlay.push_back({ 0.0f, 0.0 });
		return 0;
	};

	void LoadAllSamples()
	{
//		Load("music/puzzle-battle.mp3");

		Load("throw.wav");
		Load("banana-bounce.wav");
		Load("gibbon-shout.wav");
		Load("gibbon-happy.wav");
		Load("screeen-shake.wav");
	}

	void PlaySfx(SfxID nSampleID, float Volume)
	{
		Volume *= (1.0f + (rand() % 10) / 50.0f );
		State.WavsToPlay[(int)nSampleID].first = (std::max)(Volume, State.WavsToPlay[(int)nSampleID].first);
	}

	void ToggleMusic()
	{
//		if (State.MusicInstanceId != 999)
//		{
//			State.MiniAudio->Toggle(State.MusicInstanceId);
//		}
	}

	void PlayMusic(SfxID nSampleID, bool bLoop)
	{
//		State.MusicInstanceId = State.MiniAudio->Play(State.WavFilenames[(int)nSampleID]);
//		State.MiniAudio->SetVolume(State.MusicInstanceId, VolumeMusic);
//		ma_sound_set_looping(State.MiniAudio->GetSound(State.MusicInstanceId), true);
	}

	void Update()
	{
		for (int i = 0; i < State.WavFilenames.size(); ++i)
		{
			if (State.WavsToPlay[i].first > 0.0f && State.CumulativeTime > State.WavsToPlay[i].second + MIN_TIME_BETWEEN_SAMPLES)
			{
				int SoundId{ State.MiniAudio->Play(State.WavFilenames[i]) };

				float PitchAdjustment{ 1.0f + (float)(rand() % 100) / 10000.0f };
				State.MiniAudio->SetVolume(SoundId, State.WavsToPlay[i].first * VolumeSfx);
				ma_sound_set_pitch(State.MiniAudio->GetSound(SoundId), PitchAdjustment);

				State.WavsToPlay[i].second = State.CumulativeTime;
			}
			State.WavsToPlay[i].first = 0.0f;
		}
	}
};


#pragma once

#include "types.h"
#include "state.h"

extern state State;

namespace Inputs
{
	struct buttonMapping
	{
		olc::GPButtons Button;
		inputID InputID;
	};

	struct axisMapping
	{
		olc::GPAxes Axis;
		inputID LowInputID;
		inputID HighInputID;
	};

	std::vector<buttonMapping> ButtonMappings
	{
		/*
		Enum      XBOX/PS
		-----------------
		FACE_D    A/X
		FACE_L    X/Square
		FACE_R    B/Circle
		FACE_U    Y/Triangle
		L1        Left shoulder button
		L2        Left trigger as a button
		L3        Left stick button
		R1        Right shoulder button
		R2        Right trigger as a button
		R3        Right stick button
		SELECT    Back/Share
		START     Start/Options
		DPAD_L    Left DPAD direction as a button
		DPAD_R    Right DPAD direction as a button
		DPAD_U    Up DPAD direction as a button
		DPAD_D    Down DPAD direction as a button
		*/

				{olc::GPButtons::DPAD_U, inputID::DPAD_UP},
				{olc::GPButtons::DPAD_L, inputID::DPAD_LEFT},
				{olc::GPButtons::DPAD_D, inputID::DPAD_DOWN},
				{olc::GPButtons::DPAD_R, inputID::DPAD_RIGHT},

				{olc::GPButtons::FACE_U, inputID::FACE_UP},
				{olc::GPButtons::FACE_L, inputID::FACE_LEFT},
				{olc::GPButtons::FACE_D, inputID::FACE_DOWN},
				{olc::GPButtons::FACE_R, inputID::FACE_RIGHT},

				{olc::GPButtons::L1, inputID::SHOULDER_LEFT},
				{olc::GPButtons::R1, inputID::SHOULDER_RIGHT},
				{olc::GPButtons::L2, inputID::SHOULDER_LEFT},
				{olc::GPButtons::R2, inputID::SHOULDER_RIGHT},

				{olc::GPButtons::START, inputID::START},
				{olc::GPButtons::SELECT, inputID::SELECT}
	};
	std::vector<axisMapping> AxisMappings // DX, DY?
	{
		/*
		Enum    Range    Name
		LX      -1..1    Left stick horizontal
		LY      -1..1    Left stick vertical
		RX      -1..1    Right stick horizontal
		RY      -1..1    Right stick vertical
		TL       0..1    Left trigger as an axis
		TR       0..1    Right trigger as an axis
		DX      -1..1    DPAD horizontal axis
		DY      -1..1    DPAD vertical axis
		*/
				{olc::GPAxes::LY, inputID::DPAD_UP, inputID::DPAD_DOWN},
				{olc::GPAxes::LX, inputID::DPAD_LEFT, inputID::DPAD_RIGHT},
				{olc::GPAxes::TL, inputID::SHOULDER_LEFT, inputID::SHOULDER_LEFT},
				{olc::GPAxes::TR, inputID::SHOULDER_RIGHT, inputID::SHOULDER_RIGHT}
	};

	bool IsGamepadValid(olc::GamePad* GamepadToTest)
	{
		if (!GamepadToTest) return false;
		if (!GamepadToTest->stillConnected) return false;
//		std::cout << "Let's go!";
		return true;
	}

	void UpdateInputs()
	{
		// Check for new gamepads:
		olc::GamePad* NewGamePadToTest{ nullptr };
		NewGamePadToTest = olc::GamePad::selectWithAnyButton();
		if (NewGamePadToTest)
		{
			bool bNewGamePadToTestAlreadyInUse{ false };
			bool bExistingGamePadHasDisconnected{ false };
			for (const auto& ExistingGamePad : State.GamePads)
			{
				if (!ExistingGamePad->stillConnected)
				{
					bExistingGamePadHasDisconnected = true;
				}
				else if (NewGamePadToTest == ExistingGamePad)
				{
					bNewGamePadToTestAlreadyInUse = true;
				}
			};

			if (bNewGamePadToTestAlreadyInUse)
			{
				// Ignore	
			}
			else if (!IsGamepadValid(NewGamePadToTest))
			{
				// Ignore	
			}
			else if (bExistingGamePadHasDisconnected)
			{
				// Allocate this to the first player needing a gamepad
				for (auto& ExistingGamePad : State.GamePads)
				{
					if (!IsGamepadValid(ExistingGamePad))
					{
						ExistingGamePad = NewGamePadToTest;
					}
				};
			}
			else
			{
				// Allocate to a new player
				State.GamePads.push_back(NewGamePadToTest);
				if (State.PlayerInputs.size() < State.GamePads.size()) State.PlayerInputs.push_back({});
			}
		}

		// Check gamepad inputs:
		State.PlayerInputs[0].ResetInputs(); // ** Temp (to allow the keyboard backup to work for player 1)

		for (int i = 0; i < State.GamePads.size(); ++i)
		{
			if (State.GamePads[i]->stillConnected)
			{
				State.PlayerInputs[i].ResetInputs();
				for (const auto& ButtonToTest : ButtonMappings)
				{
					if (State.GamePads[i]->getButton(ButtonToTest.Button).bPressed) State.PlayerInputs[i].Pressed[(int)(ButtonToTest.InputID)] = true;
					if (State.GamePads[i]->getButton(ButtonToTest.Button).bHeld) State.PlayerInputs[i].Held[(int)(ButtonToTest.InputID)] = true;
				}

				for (const auto& AxisToTest : AxisMappings)
				{
					float AxisValue{ State.GamePads[i]->getAxis(AxisToTest.Axis) };

					int LowInputAxis{ (int)(AxisToTest.LowInputID) };
					if (AxisValue < 0)
					{
						State.PlayerInputs[i].Held[LowInputAxis] = true;
						if (!State.PlayerInputs[i].HeldLastFrame[LowInputAxis]) State.PlayerInputs[i].Pressed[LowInputAxis] = true;
						State.PlayerInputs[i].HeldLastFrame[LowInputAxis] = true;
					}
					else
					{
						State.PlayerInputs[i].HeldLastFrame[LowInputAxis] = false;
					};

					int HighInputAxis{ (int)(AxisToTest.HighInputID) };
					if (AxisValue > 0)
					{
						State.PlayerInputs[i].Held[HighInputAxis] = true;
						if (!State.PlayerInputs[i].HeldLastFrame[HighInputAxis]) State.PlayerInputs[i].Pressed[HighInputAxis] = true;
						State.PlayerInputs[i].HeldLastFrame[HighInputAxis] = true;
					}
					else
					{
						State.PlayerInputs[i].HeldLastFrame[HighInputAxis] = false;
					};
				}
			}
			else if (State.PlayerInputs[i].bIsGamePadConnected)
			{
				State.PlayerInputs[i].bIsGamePadConnected = false;
				State.PlayerInputs[i].ResetInputs();
			}
		}

		// And check backup keys for player 1: // ** Temp
		int i = 0;
		for (const auto& Player1Key : Player1Keys)
		{
			if (State.PGE->GetKey(Player1Key).bPressed) State.PlayerInputs[0].Pressed[i] = true;
			if (State.PGE->GetKey(Player1Key).bHeld) State.PlayerInputs[0].Held[i] = true;
			++i;
		}
	};

};


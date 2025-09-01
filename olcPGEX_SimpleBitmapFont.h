/*
	olcPGEX_SimpleBitmapFont.h

	+-------------------------------------------------------------+
	|         OneLoneCoder Pixel Game Engine Extension            |
	|                 Simple Bitmap Font v0.01                    |
	+-------------------------------------------------------------+

	What is this?
	~~~~~~~~~~~~~
	This extension provides drawing routines giving simple bitmap
	fonts with rudimentary kerning

	** Current key limitations:
	  - Early in development
	  - Will contain bugs and not optimised
	  - Only supports drawing decals (not sprites)
	  - No blank columns within a character in font atlas png
	  - Only basic error trapping
	  - Many more ..

	Licence (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2025 OneLoneCoder.com 

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	OLC Links
	~~~~~~~~~
	Discord:	https://discord.gg/WhwHUMV
	GitHub:		https://www.github.com/onelonecoder
	Homepage:	https://www.onelonecoder.com

	Author
	~~~~~~
	tgd

	Revisions:
	0.01:	Initial Release
	
*/

#pragma once
#ifndef OLC_PGEX_SIMPLEFONT_H
#define OLC_PGEX_SIMPLEFONT_H

#include "olcPixelGameEngine.h"

namespace olc
{		
	namespace sbfont
	{
		struct settings
		{
			float SourceScale{ 4.0f };
			float SpaceBetweenCharactersInSourcePixels{ 2.5f };
			float WidthOfSpaceCharacterInSourcePixels{ 10.0f };
			float HorizontalFringeInSourcePixels{ 2.0f };
			bool bIgnoreMissingCharacters{ true };
			bool bFilter{ true };

			// Change requires font update:
			int TransparencyAlphaThreshold{ 64 };
		};

		class font : public olc::PGEX
		{
		// **********************************************************************************************************

		public:

		// **********************************************************************************************************

		private:
			std::shared_ptr<Renderable> AtlasGfx{};
			std::string RawCharacters{};
			settings Settings{};

			struct characterSpacing
			{
				float Start;
				float Width;
				float Rhs() const { return Start + Width - 1.0f; };
			};
			std::unordered_map<char, characterSpacing> CharacterSpacingMap{};
			std::unordered_map<char, int> MissingCharacters{};

			struct PairCharHash { std::size_t operator()(const std::pair<char, char>& p) const { return std::hash<char>()(p.first) ^ (std::hash<char>()(p.second) << 1); } };
			std::unordered_map<std::pair<char, char>, float, PairCharHash> KerningAdjustmentMap{};

			bool bPostponeFontUpdate{ false };
			bool bDrawText{ true };
			vf2d TextSize{ 0.0f, 0.0f };

		// **********************************************************************************************************

		public:
			font(const std::string& FontAtlasFilename, const std::string& FontCharacters, const settings& FontSettings = settings{});

			bool SetFontAtlas(const std::string& FontAtlasFilename);
			std::shared_ptr<Renderable> GetFontAtlas() const;

			bool SetFontCharacters(const std::string& FontCharacters);
			std::string GetFontCharaters() const;

			bool SetFontSettings(settings);
			settings GetFontSettings() const;

		// **********************************************************************************************************

			olc::vf2d GetTextSize(const std::string& Text, const float& TextScale = 1.0f);
			void DrawStringDecal(const olc::vf2d& Position, const std::string& Text, const Pixel Tint = olc::WHITE, const float TextScale = 1.0f);

		// **********************************************************************************************************

		private:
			bool IsGfxColumnBlank(const int ColumnIndex);
			bool UpdateFont();
		};
	}
}

#ifdef OLC_PGEX_SIMPLEFONT
#undef OLC_PGEX_SIMPLEFONT

namespace olc
{
	namespace sbfont
	{
		// **********************************************************************************************************

		font::font(const std::string& FontAtlasFilename, const std::string& FontCharacters, const settings& FontSettings)
		{
			bPostponeFontUpdate = true;
			SetFontSettings(FontSettings);
			SetFontAtlas(FontAtlasFilename);
			SetFontCharacters(FontCharacters);
			bPostponeFontUpdate = false;
			UpdateFont();
		}

		bool font::SetFontAtlas(const std::string& FontAtlasFilename)
		{
			AtlasGfx = std::make_shared<olc::Renderable>();
			AtlasGfx->Load(FontAtlasFilename, nullptr, Settings.bFilter, true);
			if (AtlasGfx->Sprite()->width == 0)
			{
				std::cout << "Missing file: " << FontAtlasFilename << '\n';
				return false;
			}
			else
			{
				if (!bPostponeFontUpdate) return UpdateFont();
				return true;
			}
		}
		std::shared_ptr<Renderable> font::GetFontAtlas() const
		{
			return AtlasGfx;
		}

		bool font::SetFontCharacters(const std::string& FontCharacters)
		{
			if (FontCharacters != RawCharacters)
			{
				RawCharacters = FontCharacters;
				if (!bPostponeFontUpdate) return UpdateFont();
				return true;
			}
		}
		std::string font::GetFontCharaters() const { return RawCharacters; };

		bool font::SetFontSettings(settings FontSettings)
		{
			bool bFontNeedsToUpdate{ FontSettings.TransparencyAlphaThreshold != Settings.TransparencyAlphaThreshold };
			Settings = FontSettings;
			if (bFontNeedsToUpdate && !bPostponeFontUpdate) return UpdateFont();
			return true;
		}
		sbfont::settings font::GetFontSettings() const { return Settings; }

		// **********************************************************************************************************

		olc::vf2d font::GetTextSize(const std::string& Text, const float& TextScale)
		{
			bDrawText = false;
			DrawStringDecal({ 0.0, 0.0 }, Text, olc::WHITE, TextScale);
			bDrawText = true;
			return TextSize;
		}

		void font::DrawStringDecal(const olc::vf2d& Position, const std::string& Text, const Pixel Tint, const float TextScale)
		{
			float Scaling{ TextScale / Settings.SourceScale };			
			char PreviousCharacter{ ' ' };
			float MessageWidth{ 0.0f };

			for (int i = 0; i < (int)Text.length(); ++i)
			{
				char Character{ Text[i] };

				float KerningAdjustment{ 0.0f };
				if (KerningAdjustmentMap.contains({ PreviousCharacter, Character }))
				{
					KerningAdjustment = (float)KerningAdjustmentMap.at({ PreviousCharacter, Character });
				}
				PreviousCharacter = Character;

				if (Character == ' ')
				{
					PreviousCharacter = Character;
					MessageWidth += Scaling * Settings.WidthOfSpaceCharacterInSourcePixels;
				}
				else if (CharacterSpacingMap.contains(Character))
				{
					characterSpacing& CharacterSpacing{ CharacterSpacingMap.at(Character) };

					if (i > 0)
					{
						MessageWidth += Scaling * Settings.SpaceBetweenCharactersInSourcePixels;
						MessageWidth -= Scaling * KerningAdjustment * 1.0f; // *** ***
					}

					// Draw character
					if (bDrawText)
					{
						vf2d  CharacterDrawPosition{ Position + vf2d{MessageWidth - Scaling * Settings.HorizontalFringeInSourcePixels, 0.0f} };
						vf2d  CharacterSourcePosition{ CharacterSpacing.Start - Settings.HorizontalFringeInSourcePixels, 0.0f };
						vf2d  CharacterSourceSize{ CharacterSpacing.Width + 2.0f * Settings.HorizontalFringeInSourcePixels, (float)AtlasGfx->Sprite()->height };
						vf2d  CharacterScale{ Scaling, Scaling };
						pge->DrawPartialDecal(CharacterDrawPosition, AtlasGfx->Decal(), CharacterSourcePosition, CharacterSourceSize, CharacterScale, Tint);
					}

					// Add character width, less right padding
					MessageWidth += Scaling * CharacterSpacing.Width;
				}
				else if (!Settings.bIgnoreMissingCharacters)
				{
					if (MissingCharacters.contains(Character))
					{
						++MissingCharacters.at(Character);
					}
					else
					{
						MissingCharacters.insert({ Character, 1 });
					}
					std::cout << std::format("Font atlas missing character {} (count: {}) \n", Character, MissingCharacters.at(Character));
				}
			};

			if ((int)Text.length() > 1) MessageWidth -= Scaling * Settings.SpaceBetweenCharactersInSourcePixels;

			TextSize = { MessageWidth, Scaling * (float)(AtlasGfx->Sprite()->height) };
		}

		// **********************************************************************************************************

		bool font::IsGfxColumnBlank(const int ColumnIndex)
		{
			vi2d AtlasGfxSize{ AtlasGfx->Sprite()->Size() };
			for (vi2d PixelPosition{ ColumnIndex, 0 }; PixelPosition.y < AtlasGfxSize.y; ++PixelPosition.y)
			{
				Pixel CurrentPixel{ AtlasGfx->Sprite()->GetPixel(PixelPosition) };
				if (CurrentPixel.a > Settings.TransparencyAlphaThreshold) return false;
			}
			return true;
		}

		bool font::UpdateFont()
		{
			if (!AtlasGfx)
			{
				std::cout << "Missing font bitmap \n";
				return false;
			}

			static int Counter{ 0 };

			bool bFoundStartOfCharacter{ false };
			characterSpacing StartWidth{ 0.0f, 0.0f };
			vi2d AtlasGfxSize{ AtlasGfx->Sprite()->Size() };

			// Calculate character start and width
			for (int i = 0; i < (int)RawCharacters.length(); ++i)
			{
				++Counter;
				if (Counter > 10000) break;

				char Character{ RawCharacters[i] };
				for (vi2d PixelPosition{ (int)StartWidth.Start, 0 }; PixelPosition.x < AtlasGfxSize.x; ++PixelPosition.x)
				{
					if (IsGfxColumnBlank(PixelPosition.x))
					{
						if (bFoundStartOfCharacter)
						{
							CharacterSpacingMap.insert({ Character, StartWidth });
							StartWidth.Start = PixelPosition.x;
							bFoundStartOfCharacter = false;
							break;
						}
					}
					else
					{
						if (!bFoundStartOfCharacter)
						{
							StartWidth.Start = PixelPosition.x;
							bFoundStartOfCharacter = true;
						}
						StartWidth.Width = PixelPosition.x - StartWidth.Start + 1.0f;
					}
				}
			}

			// Calculate kerning adjustments
			for (auto& CharacterSpacingA : CharacterSpacingMap)
			{
				++Counter;
				if (Counter > 1000) break;

				for (auto& CharacterSpacingB : CharacterSpacingMap)
				{
					++Counter;
					if (Counter > 1000) break;

					int MinCharWidth{ std::min((int)CharacterSpacingA.second.Width, (int)CharacterSpacingB.second.Width) };
					bool bMaximumOverlapFound{ false };
					int MaximumOverlap{ MinCharWidth };

					for (int OverlapToTest{ 0 }; !bMaximumOverlapFound && (OverlapToTest < MinCharWidth); ++OverlapToTest)
					{
						++Counter;
						if (Counter > 1000) break;

						for (vi2d PixelPosition{ 0, 0 }; !bMaximumOverlapFound && (PixelPosition.x < OverlapToTest); ++PixelPosition.x)
						{
							++Counter;
							if (Counter > 1000) break;

							for (PixelPosition.y = 0; !bMaximumOverlapFound && (PixelPosition.y < AtlasGfxSize.y); ++PixelPosition.y)
							{
								++Counter;
								if (Counter > 1000) break;

								// Check pixel in character A (LHS character):
								vi2d PixelPositionA{ (int)CharacterSpacingA.second.Rhs() - OverlapToTest + PixelPosition.x, PixelPosition.y };
								Pixel PixelA{ AtlasGfx->Sprite()->GetPixel(PixelPositionA) };
								if (PixelA.a > Settings.TransparencyAlphaThreshold)
								{
									// Check pixel in character B (RHS character)
									vi2d PixelPositionB{ (int)CharacterSpacingB.second.Start + PixelPosition.x, PixelPosition.y };

									if (PixelPositionB.x >= CharacterSpacingB.second.Start && PixelPositionB.x <= CharacterSpacingB.second.Rhs()
										&& PixelPositionB.y >= 0 && PixelPositionB.y < AtlasGfxSize.y)
									{
										Pixel PixelB{ AtlasGfx->Sprite()->GetPixel(PixelPositionB) };
										if (PixelB.a > Settings.TransparencyAlphaThreshold)
										{
											MaximumOverlap = OverlapToTest;
											bMaximumOverlapFound = true;
										}
									}
								}
							}
						}
					}

					if (MaximumOverlap > 0.0f)
					{
						KerningAdjustmentMap.insert({ std::pair<char, char>{CharacterSpacingA.first, CharacterSpacingB.first}, (float)MaximumOverlap });
					}
				}
			}
		}

		// **********************************************************************************************************
	}
}

#endif // OLC_PGEX_SIMPLEFONT
#endif // OLC_PGEX_SIMPLEFONT_H


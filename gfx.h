#pragma once

#include "types.h"
#include "state.h"

extern state State;

namespace Gfx
{
	std::shared_ptr<olc::Renderable> Load(std::string sFilename, bool bFilter, bool bClamp)
	{
		std::string hash = sFilename + std::to_string(bFilter) + std::to_string(bClamp);
		if (State.GfxMap.count(hash) > 0)
		{
			return State.GfxMap.at(hash);
		}
		else
		{
			std::shared_ptr<olc::Renderable> NewGfx = std::make_shared<olc::Renderable>();
			NewGfx->Load(sFilename, State.AssetPack, bFilter, bClamp);
//			if (NewGfx->Sprite()->width == 0) std::cout << "Missing file: " << sFileName;
			State.GfxMap.insert(std::pair(hash, NewGfx));
			return NewGfx;
		}
	}
};
#pragma once

#include "types.h"

namespace Geometry
{
	// ================================================================================================

	vec2 MoveTowards2d(vec2 current, vec2 target, float maxDistanceDelta)
	{
		float num = target.x - current.x;
		float num2 = target.y - current.y;
		float num3 = num * num + num2 * num2;
		if (num3 == 0.0f || (maxDistanceDelta >= 0.0f && num3 <= maxDistanceDelta * maxDistanceDelta))
		{
			return target;
		}

		float num4 = std::sqrt(num3);
		return vec2{ current.x + num / num4 * maxDistanceDelta, current.y + num2 / num4 * maxDistanceDelta };
	};

	// ================================================================================================

	bool IsPointInRect(const vec2& Point, const rectCorners& RectCorners)
	{
		bool bIsPointInRect
		{
			RectCorners.TopLeft.x < Point.x && Point.x < RectCorners.BottomRight.x &&
			RectCorners.TopLeft.y < Point.y && Point.y < RectCorners.BottomRight.y
		};

		return bIsPointInRect;
	}

	bool IsPointInRect(const vec2& Point, const posSize& PosSize)
	{
		return IsPointInRect(Point, posSize_to_rectCorners(PosSize));
	}

	// ================================================================================================

}

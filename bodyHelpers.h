#pragma once

#include "types.h"
#include "state.h"

extern state State;

namespace BodyHelpers
{
	b2BodyId CreateBody(const b2BodyType& BodyType, const vec2& WorldPosition, const vec2 StartingVelocity, const float& RotationInRad, const bool& bLockRotation, bodyUserData* BodyUserData, const b2WorldId& WorldId)
	{
		b2BodyDef BodyDef = b2DefaultBodyDef();
		BodyDef.type = BodyType;
		BodyDef.position = WorldPosition;
		BodyDef.linearVelocity = StartingVelocity;
		BodyDef.rotation = b2MakeRot(RotationInRad);
		BodyDef.userData = BodyUserData;
		BodyDef.motionLocks.angularZ = bLockRotation;
		BodyDef.angularDamping = 0.4f; // **
		BodyDef.enableSleep = true;

		return b2CreateBody(WorldId, &BodyDef);
	}

	// ============================================================================

	b2ShapeId AddPolygonToBody(const vec2& Position, const std::vector<vec2>& Vertices, float Radius, const b2ShapeDef& ShapeDef, const b2BodyId& BodyId)
	{
		b2Vec2 VertexArray[B2_MAX_POLYGON_VERTICES]{};
		int VertexCount = Vertices.size();

		for (int i = 0; i < VertexCount; ++i)
		{
			VertexArray[i] = b2Vec2{ Position.x + Vertices[i].x, Position.y + Vertices[i].y };
		}
		b2Hull Hull = b2ComputeHull(VertexArray, VertexCount);
		b2Polygon Polygon = b2MakePolygon(&Hull, Radius);

		return b2CreatePolygonShape(BodyId, &ShapeDef, &Polygon);
	}

	b2ShapeId AddBoxToBody(const rectCorners& RectCorners, const b2ShapeDef& ShapeDef, const b2BodyId& BodyId)
	{
		const int8_t VertexCount{ 4 };
		b2Vec2 Vertices[VertexCount]{};

		Vertices[0] = vec2{ RectCorners.TopLeft.x, RectCorners.TopLeft.y };
		Vertices[1] = vec2{ RectCorners.BottomRight.x, RectCorners.TopLeft.y };
		Vertices[2] = vec2{ RectCorners.BottomRight.x, RectCorners.BottomRight.y };
		Vertices[3] = vec2{ RectCorners.TopLeft.x, RectCorners.BottomRight.y };
		b2Hull Hull = b2ComputeHull(Vertices, VertexCount);
		b2Polygon Polygon = b2MakePolygon(&Hull, 0); // ** Hardcoded 0 radius

		return b2CreatePolygonShape(BodyId, &ShapeDef, &Polygon);
	}

	b2ShapeId AddCircleToBody(const vec2& Position, const float& Radius, const b2ShapeDef& ShapeDef, const b2BodyId& BodyId)
	{
		b2Circle Circle{ Position, Radius };
		return b2CreateCircleShape(BodyId, &ShapeDef, &Circle);
	}

	// ============================================================================

	b2ShapeDef GetGroundShapeDef(const entityCategoryId& EntityCategory)
	{
		b2ShapeDef ShapeDef = b2DefaultShapeDef();
		ShapeDef.density = 5.0f;
		ShapeDef.material.friction = 0.8f;
		ShapeDef.material.restitution = 0.3f;
		ShapeDef.filter.categoryBits = (uint32_t)EntityCategory;
		ShapeDef.filter.maskBits = ~(uint32_t)0;
		ShapeDef.enableContactEvents = true;
		return ShapeDef;
	}

	b2ShapeDef GetPlayerShapeDef()
	{
		b2ShapeDef ShapeDef = b2DefaultShapeDef();
		ShapeDef.density = 5.0f;
		ShapeDef.material.friction = 0.8f;
		ShapeDef.material.restitution = 0.3f;
		ShapeDef.filter.categoryBits = (uint32_t)entityCategoryId::PLAYER;
		ShapeDef.filter.maskBits = ~((uint32_t)entityCategoryId::GROUND || (uint32_t)entityCategoryId::PROJECTILE);
		ShapeDef.enableContactEvents = false;
		return ShapeDef;
	}

	b2ShapeDef GetGibbonShapeDef()
	{
		b2ShapeDef ShapeDef = b2DefaultShapeDef();
		ShapeDef.density = 5.0f;
		ShapeDef.material.friction = 0.8f;
		ShapeDef.material.restitution = 0.3f;
		ShapeDef.filter.categoryBits = (uint32_t)entityCategoryId::GIBBON;
		ShapeDef.filter.maskBits = ~((uint32_t)entityCategoryId::GROUND);
		ShapeDef.enableContactEvents = true;
		return ShapeDef;
	}

	b2ShapeDef GetProjectileShapeDef()
	{
		b2ShapeDef ShapeDef = b2DefaultShapeDef();
		ShapeDef.density = 20.0f;
		ShapeDef.material.friction = 0.8f;
		ShapeDef.material.restitution = 0.4f;
		ShapeDef.filter.categoryBits = (uint32_t)entityCategoryId::PROJECTILE;
		ShapeDef.filter.maskBits = ~(uint32_t)entityCategoryId::PLAYER;
		ShapeDef.enableContactEvents = true;
		ShapeDef.enableSensorEvents = true; // **
		return ShapeDef;
	}

	b2ShapeDef GetPinShapeDef()
	{
		b2ShapeDef ShapeDef = b2DefaultShapeDef();
		ShapeDef.density = 5.0f;
		ShapeDef.material.friction = 0.0f;
		ShapeDef.material.restitution = 0.6f;
		ShapeDef.filter.categoryBits = (uint32_t)entityCategoryId::PIN;
		ShapeDef.filter.maskBits = ~(uint32_t)0;
		ShapeDef.enableContactEvents = true;
		return ShapeDef;
	}

	b2ShapeDef GetGravityFieldShapeDef()
	{
		b2ShapeDef ShapeDef = b2DefaultShapeDef();
		ShapeDef.isSensor = true;
		ShapeDef.filter.categoryBits = (uint32_t)entityCategoryId::GRAVITY_FIELD;
		ShapeDef.filter.maskBits = ~(uint32_t)0;
		ShapeDef.enableSensorEvents = true; // **
		return ShapeDef;
	}

};

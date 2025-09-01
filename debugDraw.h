#pragma once

#include "types.h"
#include "state.h"

extern state State;

namespace DebugDraw
{
	debugLine NewDebugLine(b2Vec2 PosA, b2Vec2 PosB, const b2HexColor& color)
	{
		uint32_t int32_colour{ (uint32_t)color };
		uint8_t r = static_cast<uint8_t>(int32_colour >> 0);
		uint8_t g = static_cast<uint8_t>(int32_colour >> 8);
		uint8_t b = static_cast<uint8_t>(int32_colour >> 16);
		uint8_t a = static_cast<uint8_t>(0xff); // int32_colour >> 24

		debugLine NewDebugLine
		{
			vec2{PosA.x, PosA.y}, vec2{PosB.x, PosB.y} , olc::Pixel{ r, g, b, a }
		};
		return NewDebugLine;
	};

	void DrawPolygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
	{
		for (int i = 0; i < vertexCount - 1; ++i) State.DebugLines.push_back(NewDebugLine(vertices[i], vertices[i + 1], color));
		if (vertexCount > 1) State.DebugLines.push_back(NewDebugLine(vertices[vertexCount - 1], vertices[0], color));
	};

	void DrawSolidPolygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
	{
		for (int i = 0; i < vertexCount - 1; ++i) State.DebugLines.push_back(NewDebugLine(b2TransformPoint(transform, vertices[i]), b2TransformPoint(transform, vertices[i + 1]), color));
		if (vertexCount > 1) State.DebugLines.push_back(NewDebugLine(b2TransformPoint(transform, vertices[vertexCount - 1]), b2TransformPoint(transform, vertices[0]), color));
	};

	void DrawCircle(b2Vec2 center, float Radius, b2HexColor color, void* context)
	{
		const int COUNT_CIRCLE_NODES{ 32 }; // **
		for (int i = 0; i < COUNT_CIRCLE_NODES; ++i)
		{
			float Angle0 = (float)i / (float)COUNT_CIRCLE_NODES * 2.0f * PI;
			vec2 Position0{ Radius * cosf(Angle0), Radius * sinf(Angle0) };

			float Angle1 = (float)((i + 1) % COUNT_CIRCLE_NODES) / (float)COUNT_CIRCLE_NODES * 2.0f * PI; // ** Strictly, doesn't require "% COUNT_CIRCLE_NODES"
			vec2 Position1{ Radius * cosf(Angle1), Radius * sinf(Angle1) };

			State.DebugLines.push_back(NewDebugLine(center + Position0, center + Position1, color));
		}
	};

	void DrawSolidCircle(b2Transform transform, float radius, b2HexColor color, void* context)
	{
		DrawCircle(transform.p, radius, color, context);
	};

	void DrawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context)
	{
		//		std::cout << "DrawSolidCapsule"; // ** - Not implemented
	};

	void DrawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
	{
		State.DebugLines.push_back(NewDebugLine(p1, p2, color));
	};

	void DrawTransform(b2Transform transform, void* context)
	{
		//		std::cout << "DrawTransform"; // ** - Not implemented
	};

	void DrawPoint(b2Vec2 p, float size, b2HexColor color, void* context)
	{
		DrawCircle(p, size, color, context);
	};

	void DrawString(b2Vec2 p, const char* s, b2HexColor color, void* context)
	{
		//		std::cout << "DrawString: " << s; // ** - Not implemented
	};

	b2DebugDraw DebugDraw
	{
		DrawPolygon,
		DrawSolidPolygon,
		DrawCircle,
		DrawSolidCircle,
		DrawSolidCapsule,
		DrawSegment,
		DrawTransform,
		DrawPoint,
		DrawString,
		{ { -999999999.0f, -999999999.0f }, { 999999999.0f, 999999999.0f } }, // bounds [+/-FLT_MAX]
		true,  // shapes
		true,  // joints
		false, // joint extras
		false, // bounds
		false, // mass
		false, // body names
		false, // contacts
		false, // graph colours
		false, // normals
		false, // impulse
		false, // contact features
		false, // friction impluses
		false, // islands
		nullptr // context
	};

}

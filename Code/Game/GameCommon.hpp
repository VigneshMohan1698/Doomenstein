#pragma once
#include <ThirdParty/TinyXML2/tinyxml2.h>

struct Vec2;
struct Rgba8;

typedef tinyxml2::XMLElement XmlElement;
//typedef tinyxml2::XMLDocument XMLDocument;
//typedef tinyxml2::XMLError XMLError;
#define  UNUSED(x) void(x);
//#define  UNUSED(x) float(x);
	//Const expressions For Game
	static constexpr float WORLD_SIZE_X = 200.f;
	static constexpr float WORLD_SIZE_Y = 100.f;
	static constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
	static constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

	//CAMERA OPTIONS
	static constexpr float CAMERA_WORLD_SIZEX = 1.0f;
	static constexpr float CAMERA_WORLD_SIZEY = 1.0f;
	static constexpr float CAMERA_SCREEN_SIZEX = 1600.0f;
	static constexpr float CAMERA_SCREEN_SIZEY = 800.0f;
	//Functions
	void DrawDebugLine(Vec2 start, Vec2 end, Rgba8 color, float thickness);
	void DrawDebugDisk(Vec2 center, float radius, Rgba8 color, float thickness);
	void DrawEllipse(Vec2 center, float radius, Rgba8 color, float thickness);

	



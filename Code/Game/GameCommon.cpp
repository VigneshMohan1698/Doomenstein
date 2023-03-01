#include "GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"

extern Renderer* g_theRenderer;

 void DrawDebugLine(Vec2 start, Vec2 end, Rgba8 color, float thickness)
 {
	
	float halfWidth = thickness * 0.5f;
	Vec2 unitForwardVector = (end - start) / (end - start).GetLength();
	Vec2 unitLeftVector = unitForwardVector.GetRotated90Degrees();
	Vec2 leftTopCorner = start + (halfWidth * unitLeftVector);
	Vec2 leftBottomCorner = start - (halfWidth * unitLeftVector) ;
	Vec2 rightTopCorner = end + (halfWidth * unitLeftVector) ;
	Vec2 rightBottomCorner = end - (halfWidth * unitLeftVector);

	Vertex_PCU tempVertexArrays[6];
	tempVertexArrays[0].m_position = Vec3(leftTopCorner.x, leftTopCorner.y, 0.f);
	tempVertexArrays[1].m_position = Vec3(leftBottomCorner.x, leftBottomCorner.y, 0.f);
	tempVertexArrays[2].m_position = Vec3(rightTopCorner.x, rightTopCorner.y, 0.f);
	tempVertexArrays[3].m_position = Vec3(leftBottomCorner.x, leftBottomCorner.y, 0.f);
	tempVertexArrays[4].m_position = Vec3(rightBottomCorner.x, rightBottomCorner.y, 0.f);
	tempVertexArrays[5].m_position = Vec3(rightTopCorner.x, rightTopCorner.y, 0.f);

	tempVertexArrays[0].m_color = color;
	tempVertexArrays[1].m_color = color;
	tempVertexArrays[2].m_color = color;
	tempVertexArrays[3].m_color = color;
	tempVertexArrays[4].m_color = color;
	tempVertexArrays[5].m_color = color;

	g_theRenderer->DrawVertexArray(6, tempVertexArrays);
 }

 void DrawDebugDisk(Vec2 center, float radius, Rgba8 color, float thickness)
 {
	const float totalDegrees = 360.0f;
	const int totalSides = 32;
	const int totalVertices = totalSides * 6;
	float thicknessHalf = thickness * 0.5f;
	float splitDegrees = totalDegrees / (float)totalSides;
	float radiusInner = radius - thicknessHalf;
	float radiusOuter = radius + thicknessHalf;
	Vertex_PCU tempVertexArrays[totalVertices];
	for (int sideIndex = 0; sideIndex < totalSides; sideIndex++)
	{
		float point1degrees = splitDegrees * sideIndex;
		float point2Degrees = splitDegrees * (sideIndex + 1);
		Vec3 startPoint1 = Vec3(center.x + (CosDegrees(point1degrees) * radiusInner), center.y + (SinDegrees(point1degrees) * radiusInner), 0.f);
		Vec3 endPoint1 = Vec3(center.x + (CosDegrees(point1degrees) * radiusOuter), center.y + (SinDegrees(point1degrees) * radiusOuter), 0.f);
		Vec3 startPoint2 = Vec3(center.x + (CosDegrees(point2Degrees) * radiusInner), center.y + (SinDegrees(point2Degrees) * radiusInner), 0.f);
		Vec3 endPoint2 = Vec3(center.x + (CosDegrees(point2Degrees) * radiusOuter), center.y + (SinDegrees(point2Degrees) * radiusOuter), 0.f);
		int boxIndex = sideIndex * 6;
		tempVertexArrays[boxIndex].m_position = startPoint1;
		tempVertexArrays[boxIndex + 1].m_position = endPoint1;
		tempVertexArrays[boxIndex + 2].m_position = startPoint2;
		tempVertexArrays[boxIndex + 3].m_position = startPoint2;
		tempVertexArrays[boxIndex + 4].m_position = endPoint1;
		tempVertexArrays[boxIndex + 5].m_position = endPoint2;

		tempVertexArrays[boxIndex].m_color = color;
		tempVertexArrays[boxIndex + 1].m_color = color;
		tempVertexArrays[boxIndex + 2].m_color = color;
		tempVertexArrays[boxIndex + 3].m_color = color;
		tempVertexArrays[boxIndex + 4].m_color = color;
		tempVertexArrays[boxIndex + 5].m_color = color;

	}
	g_theRenderer->DrawVertexArray(totalVertices, tempVertexArrays);
 }

 void DrawEllipse(Vec2 center, float radius, Rgba8 color, float thickness)
 {
	 UNUSED((void)thickness);
	 UNUSED((void)color);
	 UNUSED((void)radius);
	 const int totalVertices = 24;
	 Vertex_PCU tempVertexArrays[totalVertices];
	 float degrees = 0.0f;
	 float ellipseDegrees = 360.0f;
	 int steps = 15;
	 Vec2 point1;
	 Vec2 point2;
	 point2.x = CosDegrees(0.0f);
	 point2.y = SinDegrees(0.0f);
	 Vec2 point3;
	 Vec2 previousPoint;
	 for (int index = 0; index < totalVertices; ellipseDegrees++)
	 {
		 point1 = Vec2(0.0f, 0.0f);
		 previousPoint = point2;

		 point3.x = CosDegrees(degrees);
		 point3.y = SinDegrees(degrees);
		 g_theRenderer->DrawTriangle(point1, point2, point3, Rgba8(100, 100, 100, 255));
		 point2 = point3;
		 degrees += steps;
		 if (degrees > 360)
			 break;

	 }
 }

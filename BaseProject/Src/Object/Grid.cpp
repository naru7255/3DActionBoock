#include "Grid.h"

Grid::Grid(void)
{
}

Grid::~Grid(void)
{
}

void Grid::Init(void)
{
}

void Grid::Update(void)
{
}

void Grid::Draw(void)
{
	// 格子状のグリッド線 (XZ平面)
	for (int i = -HNUM; i <= HNUM; ++i)
	{
		float offset = i * TERM;

		// X方向の線 (青色: Z軸方向に並ぶ)
		VECTOR sPosX = { -HLEN, 0.0f, offset };
		VECTOR ePosX = { HLEN, 0.0f, offset };
		DrawLine3D(sPosX, ePosX, GetColor(0, 100, 255));

		// Z方向の線 (青色: X軸方向に並ぶ)
		VECTOR sPosZ = { offset, 0.0f, -HLEN };
		VECTOR ePosZ = { offset, 0.0f,  HLEN };
		DrawLine3D(sPosZ, ePosZ, GetColor(0, 100, 255));
	}

	// Y軸の縦線 (緑色)
	VECTOR sPosY = { 0.0f, 0.0f, 0.0f };
	VECTOR ePosY = { 0.0f, HLEN, 0.0f };
	DrawLine3D(sPosY, ePosY, GetColor(0, 255, 0));

	// 各軸の端に球体を描画 (赤:X軸, 緑:Y軸, 青:Z軸)
	float sphereRadius = 15.0f;
	int divNum = 8;

	// X軸の先端球体 (赤)
	for (int i = -HNUM; i <= HNUM; ++i)
	{
		VECTOR pos = { HLEN, 0.0f, i * TERM };
		DrawSphere3D(pos, sphereRadius, divNum, GetColor(255, 0, 0), GetColor(255, 0, 0), TRUE);
	}

	// Z軸の先端球体 (青)
	for (int i = -HNUM; i <= HNUM; ++i)
	{
		VECTOR pos = { i * TERM, 0.0f, HLEN };
		DrawSphere3D(pos, sphereRadius, divNum, GetColor(0, 0, 255), GetColor(0, 0, 255), TRUE);
	}
}

void Grid::Release(void)
{
}
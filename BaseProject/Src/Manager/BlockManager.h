#pragma once
#include <DxLib.h>

class Block;

class BlockManager
{
public:
   // マップとブロックモデルの基本サイズ
	static constexpr int NUM_BLOCK_MODELS = 14; // ブロックモデルの種類数
	static constexpr int NUM_BLOCK_X = 20;      // マップの横幅（ブロック数）
	static constexpr int NUM_BLOCK_Z = 20;      // マップの奥行き（ブロック数）
	static constexpr float SIZE_BLOCK = 100.0f; // ブロック1個の一辺の長さ
	static constexpr float WORLD_SIZE = SIZE_BLOCK * static_cast<float>(NUM_BLOCK_X);

	BlockManager(void);
	~BlockManager(void);

    // マップ内のブロックを生成・更新・描画し、使用したモデルを解放する
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);
  // CSVファイルからマップ配置を読み込む
	void LoadMapCsvData(void);

	// 線分とブロックの当たり判定結果を取得する
	bool IsCollisionLine(VECTOR topPos, VECTOR downPos, MV1_COLL_RESULT_POLY* result);
 // カプセルとブロックの当たり判定結果を取得する
	bool IsCollisionCapsule(VECTOR capPos1, VECTOR capPos2, float radius, MV1_COLL_RESULT_POLY_DIM* resultDim);

private:
	int baseBlockModelIds_[NUM_BLOCK_MODELS];

 // CSVを読み込めない場合に使う初期マップ（0: GRASS、1: METAL、2: ICE）
	int mapData_[NUM_BLOCK_Z][NUM_BLOCK_X] =
	{
		{ 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 1 },
		{ 1, 0, 2, 0, 1 },
		{ 1, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1 },
	};

	Block* blocks_[NUM_BLOCK_Z][NUM_BLOCK_X];
};
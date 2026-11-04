#pragma once
#include <DxLib.h>

class Block;

// =============================================================================
// ステージブロック管理クラス（BlockManager）
// マップ上の全ブロックの生成、描画、CSVロード、レイキャスト衝突判定を管理
// =============================================================================
class BlockManager
{
public:
	// マップとブロックモデルの基本サイズ定義
	static constexpr int NUM_BLOCK_MODELS = 14; // 使用するブロックモデルの種類数
	static constexpr int NUM_BLOCK_X = 20;      // マップの横幅（ブロック数）
	static constexpr int NUM_BLOCK_Z = 20;      // マップの奥行き（ブロック数）
	static constexpr float SIZE_BLOCK = 100.0f; // ブロック1個の1辺の大きさ
	static constexpr float WORLD_SIZE = SIZE_BLOCK * static_cast<float>(NUM_BLOCK_X);

	BlockManager(void);
	~BlockManager(void);

	// 基本関数（初期化、更新、描画、解放）
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	// CSVファイルからマップ配置データを読み込む
	void LoadMapCsvData(void);

	// 線分とブロックモデルの衝突判定（地面レイキャスト）
	bool IsCollisionLine(VECTOR topPos, VECTOR downPos, MV1_COLL_RESULT_POLY* result);
	// カプセルとブロックモデルの衝突判定
	bool IsCollisionCapsule(VECTOR capPos1, VECTOR capPos2, float radius, MV1_COLL_RESULT_POLY_DIM* resultDim);

private:
	int baseBlockModelIds_[NUM_BLOCK_MODELS]; // ブロック基本モデルハンドル配列

	// マップ配置データ（CSV読み込み前のデフォルト値：0: GRASS, 1: METAL, 2: ICE）
	int mapData_[NUM_BLOCK_Z][NUM_BLOCK_X] =
	{
		{ 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 1 },
		{ 1, 0, 2, 0, 1 },
		{ 1, 0, 0, 0, 1 },
		{ 1, 1, 1, 1, 1 },
	};

	Block* blocks_[NUM_BLOCK_Z][NUM_BLOCK_X]; // 配置されたブロックポインタ配列
};

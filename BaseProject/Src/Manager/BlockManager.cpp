#include <string>
#include <fstream>
#include <cmath>
#include "../Application.h"
#include "../Object/Stage/Block.h"
#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "BlockManager.h"

// =============================================================================
// コンストラクタ
// =============================================================================
BlockManager::BlockManager(void)
{
	for (int i = 0; i < NUM_BLOCK_MODELS; ++i)
	{
		baseBlockModelIds_[i] = -1;
	}

	for (int y = 0; y < NUM_BLOCK_Y; ++y)
	{
		for (int z = 0; z < NUM_BLOCK_Z; ++z)
		{
			for (int x = 0; x < NUM_BLOCK_X; ++x)
			{
				blocks_[y][z][x] = nullptr;
			}
		}
	}
}

// =============================================================================
// デストラクタ
// =============================================================================
BlockManager::~BlockManager(void)
{
}

// =============================================================================
// 初期化処理（ブロックの元モデル読み込みと3次元マップ生成）
// =============================================================================
void BlockManager::Init(void)
{
	// 基礎となる各種ブロックモデルデータのロード
	std::string PATH = Application::PATH_MODEL + "Blocks/";
	baseBlockModelIds_[0] = MV1LoadModel((PATH + "Block_Grass.mv1").c_str());
	baseBlockModelIds_[1] = MV1LoadModel((PATH + "Block_Metal.mv1").c_str());
	baseBlockModelIds_[2] = MV1LoadModel((PATH + "Block_Ice.mv1").c_str());

	// 1段目（y=0）：CSVファイルから床マップデータを読み込んで配置
	LoadMapCsvData();

	// 2段目（y=1）：ステージ上に立体的な壁（遮蔽物）ブロックを配置
	BuildWallBlocks();
}

// =============================================================================
// 毎フレーム更新処理
// =============================================================================
void BlockManager::Update(void)
{
	for (int y = 0; y < NUM_BLOCK_Y; y++)
	{
		for (int z = 0; z < NUM_BLOCK_Z; z++)
		{
			for (int x = 0; x < NUM_BLOCK_X; x++)
			{
				if (blocks_[y][z][x] != nullptr)
				{
					blocks_[y][z][x]->Update();
				}
			}
		}
	}
}

// =============================================================================
// 描画処理
// =============================================================================
void BlockManager::Draw(void)
{
	for (int y = 0; y < NUM_BLOCK_Y; y++)
	{
		for (int z = 0; z < NUM_BLOCK_Z; z++)
		{
			for (int x = 0; x < NUM_BLOCK_X; x++)
			{
				if (blocks_[y][z][x] != nullptr)
				{
					blocks_[y][z][x]->Draw();
				}
			}
		}
	}
}

// =============================================================================
// 解放処理（配置ブロックインスタンスと元モデルハンドルの破棄）
// =============================================================================
void BlockManager::Release(void)
{
	// 1. 各ブロックのインスタンス解放
	for (int y = 0; y < NUM_BLOCK_Y; y++)
	{
		for (int z = 0; z < NUM_BLOCK_Z; z++)
		{
			for (int x = 0; x < NUM_BLOCK_X; x++)
			{
				if (blocks_[y][z][x] != nullptr)
				{
					blocks_[y][z][x]->Release();
					delete blocks_[y][z][x];
					blocks_[y][z][x] = nullptr;
				}
			}
		}
	}

	// 2. 元モデルハンドル（共有データ）の解放
	for (int i = 0; i < NUM_BLOCK_MODELS; i++)
	{
		if (baseBlockModelIds_[i] != -1)
		{
			MV1DeleteModel(baseBlockModelIds_[i]);
			baseBlockModelIds_[i] = -1;
		}
	}
}

// =============================================================================
// CSVマップデータの読み込み処理（1段目 y=0 の床配置）
// =============================================================================
void BlockManager::LoadMapCsvData(void)
{
	std::ifstream ifs(Application::PATH_MAP_DATA);
	if (!ifs)
	{
		OutputDebugStringA("[ERROR] CSVファイルが開けませんでした。パスを確認してください。\n");
		return;
	}

	std::string line;
	int z = 0;

	while (std::getline(ifs, line))
	{
		if (z >= NUM_BLOCK_Z) break;

		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}

		std::vector<std::string> strSplit = AsoUtility::Split(line, ',');
		int splitSize = static_cast<int>(strSplit.size());

		for (int x = 0; x < splitSize; x++)
		{
			if (x >= NUM_BLOCK_X) break;
			if (strSplit[x].empty()) continue;

			int chipNo = 0;
			try
			{
				chipNo = std::stoi(strSplit[x]);
			}
			catch (...)
			{
				continue;
			}

			if (chipNo < 0 || chipNo >= NUM_BLOCK_MODELS) continue;

			int baseModelId = baseBlockModelIds_[chipNo];
			if (baseModelId == -1) continue;

			Block::TYPE type = static_cast<Block::TYPE>(chipNo);

			Block* block = new Block();
			block->Create(type, baseModelId, x, 0, z);
			blocks_[0][z][x] = block;
		}
		z++;
	}
	ifs.close();
}

// =============================================================================
// 2段目（y=1）の立体壁ブロック配置（4箇所の2×2遮蔽物ブロック）
// =============================================================================
void BlockManager::BuildWallBlocks(void)
{
	int metalModelId = baseBlockModelIds_[static_cast<int>(Block::TYPE::METAL)];
	if (metalModelId == -1) return;

	// ステージ四方に2×2の立体的な壁（遮蔽物）を配置（中央の初期位置は確保）
	const int wallOrigins[4][2] = {
		{ 5, 5 },
		{ 13, 5 },
		{ 5, 13 },
		{ 13, 13 }
	};

	for (int i = 0; i < 4; ++i)
	{
		int ox = wallOrigins[i][0];
		int oz = wallOrigins[i][1];
		for (int dz = 0; dz < 2; ++dz)
		{
			for (int dx = 0; dx < 2; ++dx)
			{
				int bx = ox + dx;
				int bz = oz + dz;
				if (bx >= 0 && bx < NUM_BLOCK_X && bz >= 0 && bz < NUM_BLOCK_Z)
				{
					Block* block = new Block();
					block->Create(Block::TYPE::METAL, metalModelId, bx, 1, bz);
					blocks_[1][bz][bx] = block;
				}
			}
		}
	}
}

// =============================================================================
// 線分とブロックモデルの衝突判定（地面・壁レイキャスト）
// =============================================================================
bool BlockManager::IsCollisionLine(VECTOR topPos, VECTOR downPos, MV1_COLL_RESULT_POLY* result)
{
	bool isHit = false;
	float minDistanceSq = 99999999.0f;

	// 線分の中心と長さからXZ平面の大まかな探索距離を算出し、遠方ブロックを高速スキップ
	VECTOR mid = VScale(VAdd(topPos, downPos), 0.5f);
	float halfLen = VSize(VSub(downPos, topPos)) * 0.5f;
	float maxRange = halfLen + (SIZE_BLOCK * 0.85f);
	float maxRangeSq = maxRange * maxRange;

	for (int y = 0; y < NUM_BLOCK_Y; y++)
	{
		for (int z = 0; z < NUM_BLOCK_Z; z++)
		{
			for (int x = 0; x < NUM_BLOCK_X; x++)
			{
				Block* block = blocks_[y][z][x];
				if (block == nullptr) continue;

				VECTOR bPos = block->GetPos();
				float dx = bPos.x - mid.x;
				float dz = bPos.z - mid.z;
				if ((dx * dx + dz * dz) > maxRangeSq) continue;

				MV1_COLL_RESULT_POLY res = MV1CollCheck_Line(block->GetModelId(), -1, topPos, downPos);
				if (res.HitFlag)
				{
					float distSq = VSquareSize(VSub(res.HitPosition, topPos));
					if (distSq < minDistanceSq)
					{
						minDistanceSq = distSq;
						*result = res;
						isHit = true;
					}
				}
			}
		}
	}
	return isHit;
}

// =============================================================================
// カプセルとブロックモデルの衝突判定
// =============================================================================
bool BlockManager::IsCollisionCapsule(VECTOR capPos1, VECTOR capPos2, float radius, MV1_COLL_RESULT_POLY_DIM* resultDim)
{
	VECTOR center = VScale(VAdd(capPos1, capPos2), 0.5f);
	float searchDist = (SIZE_BLOCK * 0.7071f) + radius + 10.0f;
	float searchDistSq = searchDist * searchDist;

	for (int y = 0; y < NUM_BLOCK_Y; y++)
	{
		for (int z = 0; z < NUM_BLOCK_Z; z++)
		{
			for (int x = 0; x < NUM_BLOCK_X; x++)
			{
				Block* block = blocks_[y][z][x];
				if (block == nullptr) continue;

				VECTOR bPos = block->GetPos();
				float dx = bPos.x - center.x;
				float dz = bPos.z - center.z;
				if ((dx * dx + dz * dz) > searchDistSq) continue;

				MV1_COLL_RESULT_POLY_DIM dim = MV1CollCheck_Capsule(block->GetModelId(), -1, capPos1, capPos2, radius);
				if (dim.HitNum > 0)
				{
					*resultDim = dim;
					return true;
				}
				MV1CollResultPolyDimTerminate(dim);
			}
		}
	}
	return false;
}
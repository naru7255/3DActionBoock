#include <string>
#include <fstream>
#include "../Application.h"
#include "../Object/Stage/Block.h"
#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "BlockManager.h"

BlockManager::BlockManager(void)
{
	for (int i = 0; i < NUM_BLOCK_MODELS; ++i)
	{
		baseBlockModelIds_[i] = -1;
	}

	for (int z = 0; z < NUM_BLOCK_Z; ++z)
	{
		for (int x = 0; x < NUM_BLOCK_X; ++x)
		{
			blocks_[z][x] = nullptr;
		}
	}
}

BlockManager::~BlockManager(void)
{
}

void BlockManager::Init(void)
{
	// 元となるモデルデータのロード
	std::string PATH = Application::PATH_MODEL + "Blocks/";
	baseBlockModelIds_[0] = MV1LoadModel((PATH + "Block_Grass.mv1").c_str());
	baseBlockModelIds_[1] = MV1LoadModel((PATH + "Block_Metal.mv1").c_str());
	baseBlockModelIds_[2] = MV1LoadModel((PATH + "Block_Ice.mv1").c_str());

	// CSV からマップを生成する（これだけでOK）
	LoadMapCsvData();
}

void BlockManager::Update(void)
{
	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			if (blocks_[z][x] != nullptr)
			{
				blocks_[z][x]->Update();
			}
		}
	}
}

void BlockManager::Draw(void)
{
	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			if (blocks_[z][x] != nullptr)
			{
				blocks_[z][x]->Draw();
			}
		}
	}
}

void BlockManager::Release(void)
{
	// 1. 各ブロックのインスタンス解放
	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			if (blocks_[z][x] != nullptr)
			{
				blocks_[z][x]->Release();
				delete blocks_[z][x];
				blocks_[z][x] = nullptr;
			}
		}
	}

	// 2. 元モデル（親データ）のメモリ解放
	for (int i = 0; i < NUM_BLOCK_MODELS; i++)
	{
		if (baseBlockModelIds_[i] != -1)
		{
			MV1DeleteModel(baseBlockModelIds_[i]);
			baseBlockModelIds_[i] = -1;
		}
	}
}

void BlockManager::LoadMapCsvData(void)
{
	std::ifstream ifs(Application::PATH_MAP_DATA);
	if (!ifs)
	{
		OutputDebugStringA("【ERROR】CSVファイルが開けませんでした！パスを確認してください。\n");
		return;
	}

	std::string line;
	int z = 0;

	while (std::getline(ifs, line))
	{
		// 配列（Z方向）のオーバーフロー防止
		if (z >= NUM_BLOCK_Z) break;

		// 空行対策（\r や \n の除去）
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}

		std::vector<std::string> strSplit = AsoUtility::Split(line, ',');

		for (int x = 0; x < strSplit.size(); x++)
		{
			// 配列（X方向）のオーバーフロー防止
			if (x >= NUM_BLOCK_X) break;

			if (strSplit[x].empty()) continue;

			// 文字列から数値変換（変換不能な文字列が入っていた場合のクラッシュ防止）
			int chipNo = 0;
			try {
				chipNo = std::stoi(strSplit[x]);
			}
			catch (...) {
				continue; // 数値化できない文字はスキップ
			}

			// ロード済みモデルIDの範囲外アクセス防止（0?NUM_BLOCK_MODELS-1 以外なら無視）
			if (chipNo < 0 || chipNo >= NUM_BLOCK_MODELS) continue;

			// モデルハンドルが未ロード（-1）の場合は作らない
			int baseModelId = baseBlockModelIds_[chipNo];
			if (baseModelId == -1) continue;

			Block::TYPE type = static_cast<Block::TYPE>(chipNo);

			Block* block = new Block();
			block->Create(type, baseModelId, x, z);
			blocks_[z][x] = block;
		}
		z++;
	}
	ifs.close();
	OutputDebugStringA("【SUCCESS】CSV読み込み完了！\n");
}

bool BlockManager::IsCollisionLine(VECTOR topPos, VECTOR downPos, MV1_COLL_RESULT_POLY* result)
{
	bool isHit = false;
	float minDistanceSq = 99999999.0f; // 一番近い衝突点までの距離（二乗）

	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			Block* block = blocks_[z][x];
			if (block == nullptr) continue;

			// 線分とモデルの衝突判定
			MV1_COLL_RESULT_POLY res = MV1CollCheck_Line(block->GetModelId(), -1, topPos, downPos);
			if (res.HitFlag)
			{
				// 線の始点（topPos）からヒット位置までの距離を計算
				float distSq = VSquareSize(VSub(res.HitPosition, topPos));

				// 「今まで見つかった衝突点の中で、一番手前にあるもの」を採用する！
				if (distSq < minDistanceSq)
				{
					minDistanceSq = distSq;
					*result = res;
					isHit = true;
				}
			}
		}
	}
	return isHit; // 全部チェックし終わって、一番手前の壁の情報を返す
}

bool BlockManager::IsCollisionCapsule(VECTOR capPos1, VECTOR capPos2, float radius, MV1_COLL_RESULT_POLY_DIM* resultDim)
{
	bool isHit = false;
	// 検索範囲（XZ平面）を絞り込むためカプセルの中心位置と判定半径を計算
	VECTOR center = VScale(VAdd(capPos1, capPos2), 0.5f);
	float searchDist = (SIZE_BLOCK * 0.7071f) + radius + 10.0f;
	float searchDistSq = searchDist * searchDist;

	for (int z = 0; z < NUM_BLOCK_Z; z++)
	{
		for (int x = 0; x < NUM_BLOCK_X; x++)
		{
			Block* block = blocks_[z][x];
			if (block == nullptr) continue;

			// ブロック中心とカプセル中心のXZ距離で大まかにカリング
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
	return false;
}
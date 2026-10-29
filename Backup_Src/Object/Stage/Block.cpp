#include "Block.h"
#include "../../Manager/BlockManager.h"

Block::Block(void)
{
	type_ = TYPE::NONE;
	modelId_ = -1;
	pos_ = VGet(0.0f, 0.0f, 0.0f);
	scales_ = SCALES;
}

Block::~Block(void)
{
}

void Block::Create(TYPE type, int baseModelId, int x, int z)
{
	type_ = type;

	// ベースモデルから複製
	modelId_ = MV1DuplicateModel(baseModelId);

	// スケール適用
	MV1SetScale(modelId_, scales_);

	// --- マップ全体の完全中央揃え計算 ---
	// マップ幅の半分 (SIZE_BLOCK * NUM_BLOCK / 2) を引いたうえで、半マス分(0.5f)ずらして中心を揃える
	float halfMapX = (static_cast<float>(BlockManager::NUM_BLOCK_X) * BlockManager::SIZE_BLOCK) * 0.5f;
	float halfMapZ = (static_cast<float>(BlockManager::NUM_BLOCK_Z) * BlockManager::SIZE_BLOCK) * 0.5f;

	pos_.x = (static_cast<float>(x) * BlockManager::SIZE_BLOCK) - halfMapX + (BlockManager::SIZE_BLOCK * 0.5f);
	pos_.y = 0.0f;
	pos_.z = (static_cast<float>(z) * BlockManager::SIZE_BLOCK) - halfMapZ + (BlockManager::SIZE_BLOCK * 0.5f);

	// モデルに座標を反映
	MV1SetPosition(modelId_, pos_);
	// ★ 衝突判定（モデルの全て）の前準備を行う
	MV1SetupCollInfo(modelId_, -1);
}
void Block::Update(void)
{
}

void Block::Draw(void)
{
	if (modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}
}

void Block::Release(void)
{
	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

int Block::GetModelId(void)
{
	return modelId_;
}
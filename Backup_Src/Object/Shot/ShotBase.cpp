#include "ShotBase.h"

ShotBase::ShotBase(TYPE type, int baseModelId) :
	modelId_(MV1DuplicateModel(baseModelId)),
	type_(type),
	pos_({ 0.0f, 0.0f, 0.0f }),
	dir_({ 0.0f, 0.0f, 0.0f }),
	scales_({ 1.0f, 1.0f, 1.0f }),
	speed_(0.0f),
	isAlive_(false),
	cntAlive_(0),
	collisionRadius_(0.0f)
{
}

ShotBase::~ShotBase(void)
{
	Release();
}

void ShotBase::CreateShot(VECTOR pos, VECTOR dir)
{
	// 弾の発射位置を設定
	pos_ = pos;
	// 弾の発射方向の設定
	dir_ = dir;
	// パラメータ設定
	SetParam();
	// 大きさの設定
	MV1SetScale(modelId_, scales_);
	// 位置の設定
	MV1SetPosition(modelId_, pos_);
}

void ShotBase::Update(void)
{
	if (!IsAlive())
	{
		// 生存していなければ処理中断
		return;
	}

	// 移動処理
	pos_ = VAdd(pos_, VScale(dir_, speed_));
	// 位置の設定
	MV1SetPosition(modelId_, pos_);

	// 生存カウンタの減少
	ReduceCntAlive();
}

void ShotBase::Draw(void)
{
	if (!IsAlive())
	{
		// 生存していなければ処理中断
		return;
	}

	// モデルの描画
	MV1DrawModel(modelId_);

#ifdef _DEBUG
	// デバッグ用:衝突判定用球体
	DrawSphere3D(pos_, collisionRadius_, 10, 0x0000ff, 0x0000ff, false);
#endif //_DEBUG
}

void ShotBase::Release(void)
{
	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

bool ShotBase::IsAlive(void)
{
	return isAlive_;
}

VECTOR ShotBase::GetPos(void)
{
	return pos_;
}

float ShotBase::GetCollisionRadius(void)
{
	return collisionRadius_;
}

ShotBase::TYPE ShotBase::GetType(void)
{
	return type_;
}

void ShotBase::ReduceCntAlive(void)
{
	cntAlive_--;
	if (cntAlive_ < 0)
	{
		isAlive_ = false;
	}
}

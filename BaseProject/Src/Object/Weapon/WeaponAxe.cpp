#include "WeaponAxe.h"
#include "../../Utility/AsoUtility.h"

WeaponAxe::WeaponAxe(void) :
	modelId_(-1),
	attackTimer_(0),
	pos_({ 0.0f, 0.0f, 0.0f }),
	dir_({ 0.0f, 0.0f, 1.0f }),
	angles_({ 0.0f, 0.0f, 0.0f })
{
}

WeaponAxe::~WeaponAxe(void)
{
	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

void WeaponAxe::Init(TYPE type)
{
	// 3Dモデル「Axe_Wood.mv1」を読み込み
	modelId_ = MV1LoadModel("Data/Model/Weapon/Axe_Wood.mv1");
	if (modelId_ != -1)
	{
		// 適切な大きさにスケール設定
		MV1SetScale(modelId_, { 0.5f, 0.5f, 0.5f });
	}

	attackTimer_ = 0;
}

void WeaponAxe::Update(void)
{
	// 攻撃中（タイマーが動いている間）のみ更新
	if (attackTimer_ > 0)
	{
		attackTimer_--;

		// 攻撃中に斧を振り下ろす回転演出
		// attackTimer_ が減るにつれて角度が変わる
		float progress = 1.0f - (static_cast<float>(attackTimer_) / ATTACK_TIME);
		angles_.x = AsoUtility::Deg2RadF(progress * 90.0f); // 0度から90度へ振り下ろし

		if (modelId_ != -1)
		{
			MV1SetPosition(modelId_, pos_);
			MV1SetRotationXYZ(modelId_, angles_);
		}
	}
}

void WeaponAxe::Draw(void)
{
	// 攻撃中のみ描画
	if (IsUse() && modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}
}

void WeaponAxe::Use(VECTOR pos, VECTOR dir)
{
	// プレイヤーの前方に斧を配置
	dir_ = dir;
	pos_ = VAdd(pos, VScale(dir_, 50.0f)); // 前方に50ユニット離す
	pos_.y += 40.0f;                       // 腰～胸の高さに合わせる

	// 向きに合わせてY軸回転を設定
	angles_.y = atan2f(dir_.x, dir_.z);
	angles_.x = 0.0f;
	angles_.z = 0.0f;

	// 攻撃タイマースタート（30フレーム持続）
	attackTimer_ = ATTACK_TIME;

	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		MV1SetRotationXYZ(modelId_, angles_);
	}
}

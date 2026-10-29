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
		MV1SetScale(modelId_, { 0.6f, 0.6f, 0.6f });
	}

	attackTimer_ = 0;
}

void WeaponAxe::Update(void)
{
	// 投擲中（タイマーが動いている間）のみ更新
	if (attackTimer_ > 0)
	{
		attackTimer_--;

		// 1. 前方にスピードをつけて飛ばす！
		pos_ = VAdd(pos_, VScale(dir_, FLY_SPEED));

		// 2. 斧をブーメランのように回転させる（Z軸またはX軸でダイナミックに回転）
		angles_.x += AsoUtility::Deg2RadF(25.0f);

		if (modelId_ != -1)
		{
			MV1SetPosition(modelId_, pos_);
			MV1SetRotationXYZ(modelId_, angles_);
		}
	}
}

void WeaponAxe::Draw(void)
{
	// 飛んでいる間のみ描画
	if (IsUse() && modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}
}

void WeaponAxe::Use(VECTOR pos, VECTOR dir)
{
	// プレイヤーの位置の少し前方・高さ40から射出
	dir_ = dir;
	pos_ = VAdd(pos, VScale(dir_, 40.0f));
	pos_.y += 40.0f;

	// 初期の向き設定（飛ぶ方向に向ける）
	angles_.y = atan2f(dir_.x, dir_.z);
	angles_.x = 0.0f;
	angles_.z = 0.0f;

	// 投擲タイマースタート（45フレーム持続して遠くまで飛ぶ）
	attackTimer_ = ATTACK_TIME;

	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		MV1SetRotationXYZ(modelId_, angles_);
	}
}

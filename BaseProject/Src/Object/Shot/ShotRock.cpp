#include "ShotRock.h"
#include "../../Utility/AsoUtility.h"

// =============================================================================
// コンストラクタ / デストラクタ
// =============================================================================
ShotRock::ShotRock(TYPE type, int baseModelId)
	: ShotBase(type, baseModelId),
	rotAngles_({ 0.0f, 0.0f, 0.0f })
{
}

ShotRock::~ShotRock(void)
{
}

// =============================================================================
// 岩石弾の固有パラメータ設定
// =============================================================================
void ShotRock::SetParam(void)
{
	scales_ = { 0.9f, 0.9f, 0.9f };    // 通常弾（0.4）の2倍以上の巨大スケール
	speed_ = 7.0f;                    // 重量感のある速度
	isAlive_ = true;                  // 生存フラグ
	cntAlive_ = 180;                  // 生存期間（約3秒）
	collisionRadius_ = 45.0f;         // 巨大岩石に合わせた大きめの衝突半径
	rotAngles_ = { 0.0f, 0.0f, 0.0f };
}

// =============================================================================
// 毎フレーム更新処理（基底クラスの移動＋岩石の自転アニメーション）
// =============================================================================
void ShotRock::Update(void)
{
	if (!IsAlive()) return;

	// 基底クラスの直線移動・生存時間減少処理
	ShotBase::Update();

	// 飛翔中にゴロゴロと回転させて迫力を出す
	rotAngles_.x += AsoUtility::Deg2RadF(5.0f);
	rotAngles_.z += AsoUtility::Deg2RadF(3.0f);

	if (modelId_ != -1)
	{
		MV1SetRotationXYZ(modelId_, rotAngles_);
	}
}

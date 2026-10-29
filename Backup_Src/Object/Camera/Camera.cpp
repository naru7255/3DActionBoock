#include "Camera.h"
#include "../../Manager/InputManager.h"
#include "../../Utility/AsoUtility.h"
#include "../../Object/Player/Player.h"
#include <cmath>

Camera::Camera(void)
{
	Init();
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	pos_ = DEFAULT_POS;
	angles_ = DEFAULT_ANGLES;
	distance_ = DEFAULT_DISTANCE;
	targetPos_ = AsoUtility::VECTOR_ZERO;
	player_ = nullptr;
}

void Camera::Update(void)
{
	InputManager& ins = InputManager::GetInstance();
	// 1フレームあたりの回転量（度数からラジアンへ変換）
	float anglePowRad = AsoUtility::Deg2RadF(SPEED_ANGLE_DEG);

	// -------------------------------------------------------------------------
	// 1. キー入力によるカメラ回転の制御
	// -------------------------------------------------------------------------
	// J/L キーで左右旋回（Y軸周りの回転：ヨー）
	if (ins.IsNew(KEY_INPUT_J)) { angles_.y -= anglePowRad; }
	if (ins.IsNew(KEY_INPUT_L)) { angles_.y += anglePowRad; }

	// I/K キーで上下の見上げ・見下ろし（X軸周りの回転：ピッチ）
	if (ins.IsNew(KEY_INPUT_I)) { angles_.x += anglePowRad; }
	if (ins.IsNew(KEY_INPUT_K)) { angles_.x -= anglePowRad; }

	// 上下の回転角度をクランプ（制限）する
	// ※真上（90度）や真下（-90度）に行くと画面がガタついたり反転するため
	if (angles_.x > LIMIT_ANGLE_HIGH) { angles_.x = LIMIT_ANGLE_HIGH; }
	if (angles_.x < LIMIT_ANGLE_LOW) { angles_.x = LIMIT_ANGLE_LOW; }

	// -------------------------------------------------------------------------
	// 2. カメラ座標の計算
	// -------------------------------------------------------------------------
	if (player_ != nullptr)
	{
		// 【ステップA】注視点（見つめる場所）をプレイヤーの頭部あたりに設定
		VECTOR playerPos = player_->GetPos();
		targetPos_ = playerPos;
		targetPos_.y += TARGET_OFFSET_Y;

		// 【ステップB】球座標（極座標）を用いてカメラのワールド座標を計算
		// プレイヤーを中心とした半径 distance_ の球面上をカメラが周回する仕組み
		//
		// 水平方向の見かけ上の距離 = 全体の距離 * cos(上下角)
		float hDist = distance_ * cosf(angles_.x);

		// X座標: 注視点から水平距離 * sin(水平角) を引く
		pos_.x = targetPos_.x - hDist * sinf(angles_.y);
		// Y座標: 上下角の sin に応じてカメラの高さを上下させる
		pos_.y = targetPos_.y + distance_ * sinf(angles_.x);
		// Z座標: 注視点から水平距離 * cos(水平角) を引く（手前側に引く）
		pos_.z = targetPos_.z - hDist * cosf(angles_.y);
	}
	else
	{
		// 追従対象がいない場合は矢印キーでフリー移動
		VECTOR moveDir = AsoUtility::VECTOR_ZERO;
		if (ins.IsNew(KEY_INPUT_UP)) { moveDir = AsoUtility::DIR_F; }
		if (ins.IsNew(KEY_INPUT_DOWN)) { moveDir = AsoUtility::DIR_B; }
		if (ins.IsNew(KEY_INPUT_LEFT)) { moveDir = AsoUtility::DIR_L; }
		if (ins.IsNew(KEY_INPUT_RIGHT)) { moveDir = AsoUtility::DIR_R; }

		VECTOR movePow = VScale(moveDir, SPEED_MOVE);
		pos_ = VAdd(pos_, movePow);
		targetPos_ = VAdd(pos_, VGet(0.0f, 0.0f, 100.0f));
	}
}

void Camera::SetBeforeDraw(void)
{
	// カメラのクリップ面（描画範囲）を設定
	SetCameraNearFar(CAMERA_NEAR, CAMERA_FAR);

	// 【注視点方式によるカメラ設定】
	// 「カメラの座標(pos_)」から「プレイヤーの注視点(targetPos_)」を常に見つめるようにセット
	// 第3引数(UpVecY)により、カメラの「上方向」がワールドの上(+Y)に固定されるため画面が傾かない
	SetCameraPositionAndTarget_UpVecY(pos_, targetPos_);
}

void Camera::DrawDebug(void)
{
	DrawFormatString(0, 0, GetColor(255, 255, 255), "カメラ座標: (%.2f, %.2f, %.2f)", pos_.x, pos_.y, pos_.z);
	DrawFormatString(
		0, 20, GetColor(255, 255, 255),
		"カメラ角度(度): (%.2f, %.2f, %.2f)",
		AsoUtility::Rad2DegF(angles_.x),
		AsoUtility::Rad2DegF(angles_.y),
		AsoUtility::Rad2DegF(angles_.z)
	);
}

void Camera::Release(void)
{
}

void Camera::SetFollow(Player* player)
{
	player_ = player;
}
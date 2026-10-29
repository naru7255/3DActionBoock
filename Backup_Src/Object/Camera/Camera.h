#pragma once
#include <DxLib.h>

class Player;

class Camera
{
public:
	// --- カメラの初期パラメータ ---
	// カメラの初期座標（プレイヤーがいない時の初期配置）
	static constexpr VECTOR DEFAULT_POS = { 0.0f, 1000.0f, -1000.0f };
	// カメラの初期角度 (X: 上下の仰角/ピッチ, Y: 左右の旋回/ヨー, Z: 画面の傾き/ロール)
	static constexpr VECTOR DEFAULT_ANGLES = { 20.0f * DX_PI_F / 180.0f, 0.0f, 0.0f };
	// 描画クリップ距離：手前（これより近いものは描画しない）
	static constexpr float CAMERA_NEAR = 10.0f;
	// 描画クリップ距離：奥（これより遠いものは描画しない）
	static constexpr float CAMERA_FAR = 30000.0f;
	// フリーカメラ時の移動速度
	static constexpr float SPEED_MOVE = 10.0f;
	// カメラの回転速度（1フレームあたりの回転度数）
	static constexpr float SPEED_ANGLE_DEG = 1.5f;

	// --- TPS追従カメラ用の調整定数 ---
	// プレイヤーとカメラの距離（カメラの引き具合）
	static constexpr float DEFAULT_DISTANCE = 1000.0f;
	// プレイヤーのどの高さを見つめるか（足元ではなく、胸?頭の高さに注視点を置く）
	static constexpr float TARGET_OFFSET_Y = 150.0f;
	// 見下ろし角度の限界値（真上から見下ろしすぎて画面が反転するのを防止）
	static constexpr float LIMIT_ANGLE_HIGH = 80.0f * DX_PI_F / 180.0f;
	// 見上げ角度の限界値（地面の下に潜り込むのを防止）
	static constexpr float LIMIT_ANGLE_LOW = -20.0f * DX_PI_F / 180.0f;

	Camera(void);
	~Camera(void);

	void Init(void);
	void Update(void);
	void SetBeforeDraw(void);
	void DrawDebug(void);
	void Release(void);

	// 追従対象のプレイヤーを登録
	void SetFollow(Player* player);

	// カメラのワールド座標を取得
	VECTOR GetCamera(void) const { return pos_; }

	// カメラの水平回転角度（Y軸回転：ラジアン）を取得
	// ※プレイヤーの移動方向をカメラの向きと連動させるために使用
	float GetAngleY(void) const { return angles_.y; }

private:
	VECTOR pos_;         // カメラのワールド座標
	VECTOR angles_;      // カメラの回転角度 (X: ピッチ / Y: ヨー / Z: ロール)
	VECTOR targetPos_;   // カメラが見つめる目標地点（注視点）
	float distance_;     // プレイヤーからの距離
	Player* player_;     // 追従対象のプレイヤー
};
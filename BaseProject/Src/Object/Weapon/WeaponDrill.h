#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// ドリル武器クラス（高速回転ロケット貫通型）
// 教材第42章：追加実装例（WeaponBase派生クラス）
// プレイヤーの前方へ一直線に高速移動し、回転スプライトで敵を穿つ！
// =============================================================================
class WeaponDrill : public WeaponBase
{
public:
	// スプライト分割数（横4コマ × 縦3コマ = 合計12コマ）
	static constexpr int NUM_SPRITE_X = 4;
	static constexpr int NUM_SPRITE_Y = 3;
	static constexpr int NUM_SPRITE_ALL = NUM_SPRITE_X * NUM_SPRITE_Y;

	// スプライト1コマあたりの画像サイズ（400x240 を 4x3 分割）
	static constexpr int SIZE_SPRITE_X = 100;
	static constexpr int SIZE_SPRITE_Y = 80;

	// アニメーション再生速度（高速回転させるため速めのコマ進行）
	static constexpr float ANIM_SPEED = 0.8f;

	// 移動スピード（斧8.0fよりさらに高速なロケット速度）
	static constexpr float FLY_SPEED = 14.0f;

	// 生存時間フレーム（約0.8秒間飛翔）
	static constexpr int LIFE_TIME = 48;

	// 当たり判定の球体半径
	static constexpr float COLLISION_RADIUS = 45.0f;

	WeaponDrill(void);
	virtual ~WeaponDrill(void);

	// 基本関数（初期化、更新、描画）
	void Init(TYPE type) override;
	void Update(void) override;
	void Draw(void) override;

	// 武器を使用（攻撃発射）する
	void Use(VECTOR pos, VECTOR dir) override;

	// 武器が使用中（判定有効）かどうか
	bool IsUse(void) const { return isUse_; }
	bool IsAlive(void) const override { return isUse_; }

	// 衝突判定用（現在座標と球体半径の取得）
	VECTOR GetPos(void) const override { return pos_; }
	float GetCollisionRadius(void) const override { return COLLISION_RADIUS; }

private:
	TYPE type_;                      // 武器種別
	int imgHandles_[NUM_SPRITE_ALL]; // 分割読み込みした連番画像ハンドル配列
	float animFrame_;                // 現在のアニメーションコマ進行カウンタ
	bool isUse_;                     // 攻撃発動中フラグ
	int lifeTimer_;                  // 飛翔生存タイマー
	VECTOR pos_;                     // 現在の3Dワールド座標
	VECTOR dir_;                     // 飛翔方向ベクトル
};
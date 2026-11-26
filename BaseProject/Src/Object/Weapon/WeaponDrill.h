#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// ドリル武器クラス（第3の武器：超高速ロケットドリル）
// 直線的に超高速前進し、激しく回転しながら敵を貫通攻撃する
// =============================================================================
class WeaponDrill : public WeaponBase
{
public:
	// スプライト分割数（横4コマ × 縦3コマ = 全12コマ）
	static constexpr int NUM_SPRITE_X = 4;
	static constexpr int NUM_SPRITE_Y = 3;
	static constexpr int NUM_SPRITE_ALL = NUM_SPRITE_X * NUM_SPRITE_Y;

	// スプライト1コマあたりの画像サイズ
	static constexpr int SIZE_SPRITE_X = 100;
	static constexpr int SIZE_SPRITE_Y = 80;

	// 高速回転アニメーション速度（1フレームあたりのコマ進行度）
	static constexpr float ANIM_SPEED = 1.0f;

	// 直線超高速ロケット前進スピード
	static constexpr float FLY_SPEED = 14.0f;

	// 飛翔持続時間（48フレーム＝約0.8秒間飛翔）
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
	int imgHandles_[NUM_SPRITE_ALL];   // 分割読み込みした連番画像ハンドル配列
	float animFrame_;                  // 現在のアニメーションコマ進行カウンタ
	bool isUse_;                       // 攻撃発動中フラグ
	int lifeTimer_;                    // 飛翔タイマー（寿命）
	VECTOR pos_;                       // 3Dワールド座標
	VECTOR dir_;                       // 攻撃発射方向ベクトル
};
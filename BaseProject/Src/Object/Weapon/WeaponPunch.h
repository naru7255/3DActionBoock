#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// パンチ武器クラス
// =============================================================================
class WeaponPunch : public WeaponBase
{
public:
	// スプライト分割数（横4 × 縦3 = 12コマ）
	static constexpr int NUM_SPRITE_X = 4;
	static constexpr int NUM_SPRITE_Y = 3;
	static constexpr int NUM_SPRITE_ALL = NUM_SPRITE_X * NUM_SPRITE_Y;

	// スプライトの1コマあたりのサイズ
	static constexpr int SIZE_XSPRITE_X = 100;
	static constexpr int SIZE_XSPRITE_Y = 80;

	static constexpr int SIZE_YSPRITE_X = 80;
	static constexpr int SIZE_YSPRITE_Y = 100;

	// アニメーション再生速度（フレーム毎の進行）
	static constexpr float ANIM_SPEED = 0.5f;

	WeaponPunch(void);
	virtual ~WeaponPunch(void);

	void Init(TYPE type) override;
	void Update(void) override;
	void Draw(void) override;

	// 攻撃の発動（プレイヤーの座標と向きを受け取る）
	void Use(VECTOR pos, VECTOR dir) override;

	// 攻撃中かどうかの取得
	bool IsUse(void) const { return isUse_; }

	// 当たり判定用
  bool IsAlive(void) const override { return isUse_; }
	VECTOR GetPos(void) const { return pos_; }
	float GetCollisionRadius(void) const { return 40.0f; }

private:
	TYPE type_;
	int imgHandles_[NUM_SPRITE_ALL]; // 分割読み込みしたアニメーション画像
	float animFrame_;                // 現在のアニメーションコマ（浮動小数点で管理）
	bool isUse_;                     // 攻撃中フラグ
	VECTOR pos_;                     // パンチエフェクトの表示3D座標
	VECTOR dir_;                     // 攻撃の向き


};
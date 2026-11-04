#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// パンチ武器クラス（近接攻撃スプライトビルボード）
// プレイヤーの前方にスプライトアニメーションを描画して攻撃する
// =============================================================================
class WeaponPunch : public WeaponBase
{
public:
	// スプライト分割数（横4コマ × 縦3コマ = 合計12コマ）
	static constexpr int NUM_SPRITE_X = 4;
	static constexpr int NUM_SPRITE_Y = 3;
	static constexpr int NUM_SPRITE_ALL = NUM_SPRITE_X * NUM_SPRITE_Y;

	// スプライト1コマあたりの画像サイズ
	static constexpr int SIZE_XSPRITE_X = 100;
	static constexpr int SIZE_XSPRITE_Y = 80;

	static constexpr int SIZE_YSPRITE_X = 80;
	static constexpr int SIZE_YSPRITE_Y = 100;

	// アニメーション再生速度（1フレームあたりのコマ進行度）
	static constexpr float ANIM_SPEED = 0.5f;

	// 当たり判定の球体半径
	static constexpr float COLLISION_RADIUS = 40.0f;

	WeaponPunch(void);
	virtual ~WeaponPunch(void);

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
	VECTOR pos_;                     // パンチエフェクトの表示3Dワールド座標
	VECTOR dir_;                     // 攻撃の向きベクトル
};

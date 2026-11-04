#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// 斧武器クラス（第2の武器：3Dモデル投擲攻撃）
// プレイヤーの前方へ回転しながら飛翔し、遠距離の敵を攻撃する
// =============================================================================
class WeaponAxe : public WeaponBase
{
public:
	// 攻撃継続時間（45フレーム＝約0.75秒間飛翔）
	static constexpr int ATTACK_TIME = 45;

	// 斧の飛翔スピード
	static constexpr float FLY_SPEED = 18.0f;

	// 斧の当たり判定球体半径
	static constexpr float COLLISION_RADIUS = 60.0f;

	WeaponAxe(void);
	virtual ~WeaponAxe(void);

	// 基本関数（初期化、更新、描画）
	void Init(TYPE type) override;
	void Update(void) override;
	void Draw(void) override;

	// 武器を使用（攻撃発射）する
	void Use(VECTOR pos, VECTOR dir) override;

	// 武器が使用中（判定有効）かどうか
	bool IsUse(void) const { return attackTimer_ > 0; }
	bool IsAlive(void) const override { return IsUse(); }

	// 衝突判定用（現在座標と球体半径の取得）
	VECTOR GetPos(void) const override { return pos_; }
	float GetCollisionRadius(void) const override { return COLLISION_RADIUS; }

private:
	int modelId_;        // 斧の3DモデルハンドルID
	int attackTimer_;    // 攻撃タイマー（カウントダウン）
	VECTOR pos_;         // 斧の現在座標
	VECTOR dir_;         // 飛翔方向ベクトル
	VECTOR angles_;      // 斧の回転角度（ブーメラン回転用）
};

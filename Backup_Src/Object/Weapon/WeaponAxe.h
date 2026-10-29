#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// 斧クラス（第2の武器: 3D投擲武器）
// =============================================================================
class WeaponAxe : public WeaponBase
{
public:
	// 攻撃持続時間（45フレーム＝約0.75秒間飛ぶ）
	static constexpr int ATTACK_TIME = 45;

	// 斧の飛翔スピード
	static constexpr float FLY_SPEED = 18.0f;

	// 斧の当たり判定半径
	static constexpr float COLLISION_RADIUS = 60.0f;

	WeaponAxe(void);
	virtual ~WeaponAxe(void);

	void Init(TYPE type) override;
	void Update(void) override;
	void Draw(void) override;

	// 攻撃実行（プレイヤーの位置と向きを受け取る）
	void Use(VECTOR pos, VECTOR dir) override;

	// 攻撃中判定（飛んでいる間有効）
	bool IsUse(void) const { return attackTimer_ > 0; }
	bool IsAlive(void) const override { return IsUse(); }

	// 当たり判定用座標と半径
	VECTOR GetPos(void) const override { return pos_; }
	float GetCollisionRadius(void) const override { return COLLISION_RADIUS; }

private:
	int modelId_;        // 斧の3Dモデルハンドル
	int attackTimer_;    // 攻撃タイマー（カウントダウン）
	VECTOR pos_;         // 斧の現在位置
	VECTOR dir_;         // 飛翔方向
	VECTOR angles_;      // 斧の回転角度
};

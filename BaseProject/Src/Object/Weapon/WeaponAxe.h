#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// 斧クラス（第2の武器: 3Dモデル）
// =============================================================================
class WeaponAxe : public WeaponBase
{
public:
	// 攻撃持続時間（フレーム数: 30フレーム＝約0.5秒）
	static constexpr int ATTACK_TIME = 30;

	// 斧の当たり判定半径
	static constexpr float COLLISION_RADIUS = 60.0f;

	WeaponAxe(void);
	virtual ~WeaponAxe(void);

	void Init(TYPE type) override;
	void Update(void) override;
	void Draw(void) override;

	// 攻撃実行（プレイヤーの位置と向きを受け取る）
	void Use(VECTOR pos, VECTOR dir) override;

	// 攻撃中判定（タイマーが残っている間有効）
	bool IsUse(void) const { return attackTimer_ > 0; }
	bool IsAlive(void) const override { return IsUse(); }

	// 当たり判定用座標と半径
	VECTOR GetPos(void) const override { return pos_; }
	float GetCollisionRadius(void) const override { return COLLISION_RADIUS; }

private:
	int modelId_;        // 斧の3Dモデルハンドル
	int attackTimer_;    // 攻撃タイマー（カウントダウン）
	VECTOR pos_;         // 斧の位置
	VECTOR dir_;         // 攻撃の向き
	VECTOR angles_;      // 斧の回転角度
};

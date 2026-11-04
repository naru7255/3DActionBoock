#pragma once
#include "EnemyBase.h"

// =============================================================================
// エネミー派生クラス：デーモン（EnemyDemon）
// プレイヤーに向かって直進し、体当たりで攻撃してくる近接型エネミー
// =============================================================================
class EnemyDemon : public EnemyBase
{
public:
	EnemyDemon(void);
	~EnemyDemon(void);

	// 通常待機状態の更新（プレイヤー追従直進）
	void UpdateStandby(void) override;

private:
	// デーモン固有のパラメータ初期化（教材第24, 27, 32章）
	void SetParam(void) override;
};

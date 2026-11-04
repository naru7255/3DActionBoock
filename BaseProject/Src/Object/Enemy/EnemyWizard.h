#pragma once
#include "EnemyBase.h"

// =============================================================================
// エネミー派生クラス：ウィザード（EnemyWizard）
// 移動せず定位置から定期的に魔法弾を発射する遠距離攻撃型エネミー
// =============================================================================
class EnemyWizard : public EnemyBase
{
public:
	// 攻撃間隔フレーム数（教材第30章：120フレーム＝約2秒間隔で発射）
	static constexpr int TERM_ATTACK = 120;

	EnemyWizard(void);
	virtual ~EnemyWizard(void);

	// 通常待機状態の更新（プレイヤーの方向を向き、時間経過で攻撃へ遷移）
	void UpdateStandby(void) override;

private:
	// ウィザード固有パラメータの設定（教材第26, 27, 30, 32章）
	void SetParam(void) override;
};

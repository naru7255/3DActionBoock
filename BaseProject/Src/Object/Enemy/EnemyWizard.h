#pragma once
#include "EnemyBase.h"

// エネミー派生クラス：ウィザード（遠距離攻撃型エネミー）
class EnemyWizard : public EnemyBase
{
public:

	// 攻撃間隔用の定数（教材 30章：120フレーム = 約2秒間隔）
	static constexpr int TERM_ATTACK = 120;

	EnemyWizard(void);
	virtual ~EnemyWizard(void);

	// 待機状態の更新処理（オーバーライド：移動せず、一定間隔で攻撃へ）
	void UpdateStandby(void) override;

private:

	// ウィザードの固有パラメータ設定
	void SetParam(void) override;

};

#pragma once
#include "EnemyBase.h"

class EnemyWizard : public EnemyBase
{
	public:
	EnemyWizard(void);
	~EnemyWizard(void);
private:
	// エネミーのパラメータを設定する純粋仮想関数の実装
	void SetParam(void) override;
};


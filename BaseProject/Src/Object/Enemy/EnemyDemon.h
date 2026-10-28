#pragma once
#include "EnemyBase.h"


class EnemyDemon : public EnemyBase
{
public:
	EnemyDemon(void);
	~EnemyDemon(void);

	void UpdateStandby(void) override;
private:
	// エネミーのパラメータを設定する純粋仮想関数の実装
	void SetParam(void) override;
};
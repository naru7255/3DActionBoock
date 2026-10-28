#include "EnemyWizard.h"

EnemyWizard::EnemyWizard(void)
{
}

EnemyWizard::~EnemyWizard(void)
{
}

void EnemyWizard::SetParam(void)
{
	scales_ = { 0.6f, 0.6f, 0.6f }; // 大きさ
	speed_ = 4.0f;                  // 移動スピード
	hp_ = 2;                        // HPの初期値
	spawnRange_ = 1.0f;             // 出現範囲の比率（ステージサイズに対する比率）
	cntAttack_ = 0;                 // カウンタ変数の初期化
}

void EnemyWizard::UpdateStandby(void)
{
	// 常にプレイヤー方向を向く
	LookPlayer();

	// 一定間隔ごとにATTACK状態へ移行
	cntAttack_++;
	if (cntAttack_ >= TERM_ATTACK)
	{
		cntAttack_ = 0; // カウンタ変数の初期化
		ChangeState(STATE::ATTACK);
	}
}

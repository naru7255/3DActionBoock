#include "EnemyWizard.h"

EnemyWizard::EnemyWizard(void)
{
}

EnemyWizard::~EnemyWizard(void)
{
}

// ★ ウィザードの固有パラメータを設定（教材 26・27・30・32章 準拠）
void EnemyWizard::SetParam(void)
{
	scales_ = { 0.5f, 0.5f, 0.5f }; // 大きさ
	speed_ = 0.0f;                  // 移動スピード
	hp_ = 2;                        // HPの初期値
	spawnRange_ = 0.8f;             // 出現範囲の比率
	collisionRadius_ = 40.0f;       // 衝突判定用球体の半径
	cntAttack_ = 0;                 // カウンタ変数の初期化
}

// ★ 待機状態の更新処理（オーバーライド）
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

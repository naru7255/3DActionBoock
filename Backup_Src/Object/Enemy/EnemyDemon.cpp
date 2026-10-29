#include "EnemyDemon.h"

EnemyDemon::EnemyDemon(void)
{
}

EnemyDemon::~EnemyDemon(void)
{
}

// ★ デーモンの固有パラメータを設定（教材 24・27・32章 準拠）
void EnemyDemon::SetParam(void)
{
	scales_ = { 0.7f, 0.7f, 0.7f }; // 大きさ（教材 27章：0.7f）
	speed_ = 5.0f;                  // 移動スピード（突進型）
	hp_ = 3;                        // HPの初期値（教材 27章：3）
	spawnRange_ = 1.0f;             // 出現範囲の比率（教材 27章：1.0f）
	collisionRadius_ = 50.0f;       // 衝突判定用球体の半径（教材 32章：50.0f）
	cntAttack_ = 0;                 // カウンタ変数の初期化
}

// ★ 待機状態の更新処理（オーバーライド）
void EnemyDemon::UpdateStandby(void)
{
	// 常にプレイヤー方向を向く
	LookPlayer();
	// プレイヤーに向かって移動する（デーモンは移動による衝突が攻撃となる）
	Move();
}

#include "EnemyDemon.h"

EnemyDemon::EnemyDemon(void)
{
}

EnemyDemon::~EnemyDemon(void)
{
}
// ★ デーモンの固有パラメータを設定
void EnemyDemon::SetParam(void)
{
	scales_ = { 0.7f, 0.7f, 0.7f }; // 大きさ
	speed_ = 5.0f;                  // 移動スピード
	hp_ = 3;                        // HPの初期値
	spawnRange_ = 1.0f;             // 出現範囲の比率（ステージサイズに対する比率）
}
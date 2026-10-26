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

}

#include "EnemyGiant.h"
#include "../Shot/ShotRock.h"
#include "../Player/Player.h"
#include "../Player/AnimationController.h"

// =============================================================================
// コンストラクタ / デストラクタ
// =============================================================================
EnemyGiant::EnemyGiant(void)
{
}

EnemyGiant::~EnemyGiant(void)
{
}

// =============================================================================
// ボス（ジャイアント）固有のパラメータ設定
// =============================================================================
void EnemyGiant::SetParam(void)
{
	scales_ = { 1.3f, 1.3f, 1.3f }; // 通常敵より圧倒的に巨大なスケール
	speed_ = 2.5f;                  // 重厚な歩行速度
	hp_ = 10;                       // ボス耐久力（10発）
	spawnRange_ = 0.5f;             // ステージ中央寄りに出現
	collisionRadius_ = 80.0f;       // 巨体に合わせた大きな衝突判定半径
	cntAttack_ = 0;                 // 攻撃間隔カウンタ
}

// =============================================================================
// 通常待機状態の更新処理（プレイヤー追従歩行＋時間経過で岩石攻撃）
// =============================================================================
void EnemyGiant::UpdateStandby(void)
{
	// プレイヤーの方向を向きながら歩行前進
	LookPlayer();
	Move();

	// 攻撃間隔カウントアップ
	cntAttack_++;
	if (cntAttack_ >= TERM_ATTACK)
	{
		cntAttack_ = 0;
		ChangeState(STATE::ATTACK);
	}
}

// =============================================================================
// 攻撃ステートへの切り替え処理（攻撃モーション再生＋巨大岩石投擲）
// =============================================================================
void EnemyGiant::ChangeAttack(void)
{
	if (animController_ != nullptr)
	{
		// 攻撃アニメーションを単発再生
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
	}

	// 巨大岩石を発射
	Shot();
}

// =============================================================================
// 攻撃ステートの更新処理（モーション終了で通常歩行へ復帰）
// =============================================================================
void EnemyGiant::UpdateAttack(void)
{
	LookPlayer();

	// 攻撃アニメーション終了で通常待機へ復帰
	if (animController_ != nullptr && animController_->IsEnd())
	{
		// 再び歩行アニメーションを再生
		if (animController_ != nullptr)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
		}
		ChangeState(STATE::STANDBY);
	}
}

// =============================================================================
// ボス専用の弾発射処理（高い胸・肩の打点から巨大岩石を発射）
// =============================================================================
void EnemyGiant::Shot(void)
{
	if (baseAttackEffectModelId_ == -1) return;

	ShotBase* shot = GetValidShot();
	if (shot != nullptr)
	{
		VECTOR shotPos = pos_;
		shotPos.y += 90.0f; // 巨人の高い胸・肩の高さから発射
		shot->CreateShot(shotPos, moveDir_);
	}
}

// =============================================================================
// ボス専用の弾インスタンス取得（ShotRockを生成・オブジェクトプール）
// =============================================================================
ShotBase* EnemyGiant::GetValidShot(void)
{
	for (size_t i = 0; i < shots_.size(); i++)
	{
		if (!shots_[i]->IsAlive())
		{
			return shots_[i];
		}
	}

	// ボス専用の大型岩石弾（ShotRock）を生成してプール
	ShotBase* shot = new ShotRock(ShotBase::TYPE::ROCK, baseAttackEffectModelId_);
	shots_.push_back(shot);
	return shot;
}

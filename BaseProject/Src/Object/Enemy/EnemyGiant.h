#pragma once
#include "EnemyBase.h"

// =============================================================================
// ボスクラス：ジャイアント（EnemyGiant）
// 教材第42章：追加実装例（EnemyBase派生クラス）
// 巨体を活かした高いHP、歩行移動、巨大岩石投擲攻撃を行う最終ボス
// =============================================================================
class EnemyGiant : public EnemyBase
{
public:
	// 攻撃間隔フレーム数（180フレーム＝約3秒間隔で岩石投擲）
	static constexpr int TERM_ATTACK = 180;

	EnemyGiant(void);
	virtual ~EnemyGiant(void);

	// 通常待機状態の更新（プレイヤー追従歩行、一定間隔で攻撃へ遷移）
	void UpdateStandby(void) override;

	// 攻撃ステートへの切り替え（攻撃モーション再生＋岩石発射）
	void ChangeAttack(void) override;

	// 攻撃ステートの更新（モーション終了で通常待機へ復帰）
	void UpdateAttack(void) override;

protected:
	// ボス固有のパラメータ設定（巨体・高HP・判定半径）
	void SetParam(void) override;

	// ボス専用の弾発射処理（高い打点から巨大岩石を発射）
	void Shot(void) override;

	// ボス専用の弾インスタンス生成（ShotRockを生成）
	ShotBase* GetValidShot(void) override;
};

#pragma once
#include "EnemyBase.h"


class EnemyWizard : public EnemyBase
{
public:
    // 攻撃間隔用の定数
    static constexpr int TERM_ATTACK = 120; // 待機から攻撃へのインターバル（フレーム）
    static constexpr int ATTACK_PREP_FRAMES = 30; // 攻撃前のカウントダウンフレーム数

    EnemyWizard(void);
    ~EnemyWizard(void);

protected:
    // 待機状態の更新処理（オーバーライド）
    void UpdateStandby(void) override;

private:
    // エネミーのパラメータを設定する純粋仮想関数の実装
    void SetParam(void) override;

    // ---------- 追従円軌道用パラメータ ----------
    float orbitAngle_ = 0.0f;                     // 現在の角度 (rad)
    static constexpr float ORBIT_RADIUS = 150.0f; // プレイヤー中心からの半径
};

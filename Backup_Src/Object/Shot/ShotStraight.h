#pragma once
#include "ShotBase.h"
class ShotStraight : public ShotBase
{
public:
    // コンストラクタ
    ShotStraight(TYPE type, int baseModelId);
    virtual ~ShotStraight(void);

    // パラメータ設定（純粋仮想関数の実装）
    virtual void SetParam(void) override;

private:





};


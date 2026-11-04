#pragma once
#include "ShotBase.h"

// =============================================================================
// ボス用岩石弾クラス（ShotRock）
// 教材第42章：追加実装例（ShotBase派生クラス）
// 巨大ボスが投擲する、高威力の大型岩石弾
// =============================================================================
class ShotRock : public ShotBase
{
public:
	ShotRock(TYPE type, int baseModelId);
	virtual ~ShotRock(void);

	// 毎フレーム更新（直線移動＋飛翔中の回転演出）
	void Update(void) override;

protected:
	// 岩石弾固有のパラメータ設定（大型スケール・当たり判定）
	void SetParam(void) override;

private:
	VECTOR rotAngles_; // 飛翔中の回転角度
};

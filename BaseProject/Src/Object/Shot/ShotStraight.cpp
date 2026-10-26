#include "ShotStraight.h"

ShotStraight::ShotStraight(TYPE type, int baseModelId)
	: ShotBase(type, baseModelId) //  親クラスのコンストラクタを呼ぶ
{
	
}


ShotStraight::~ShotStraight(void)
{
}

void ShotStraight::SetParam(void)
{
	scales_ = { 0.4f, 0.4f, 0.4f }; // 大きさ
	speed_ = 8.0f;                 // 移動スピード
	isAlive_ = true;                 // 生存判定
	cntAlive_ = 120;                 // 生存期間（フレーム数）
	collisionRadius_ = 30.0f;           // 衝突判定用半径

}

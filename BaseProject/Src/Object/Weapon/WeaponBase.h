#pragma once
#include <DxLib.h>

// =============================================================================
// 武器基底クラス (すべての武器の親クラス)
// =============================================================================
class WeaponBase
{
public:
	// 武器の種類
	enum class TYPE
	{
		NONE = -1,
		PUNCH, // パンチ
		AXE,   // 斧
	};

	WeaponBase(void) {}
	virtual ~WeaponBase(void) {}

	virtual void Init(TYPE type) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;

	virtual bool IsAlive() const = 0;

	virtual VECTOR GetPos() const = 0;

	virtual float GetCollisionRadius() const = 0;


	// 攻撃を実行する関数（座標と向きを受け取る）
	virtual void Use(VECTOR pos, VECTOR dir) = 0;
};
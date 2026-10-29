#pragma once
#include <DxLib.h>

// =============================================================================
// ������N���X (���ׂĂ̕���̐e�N���X)
// =============================================================================
class WeaponBase
{
public:
	// ����̎��
	enum class TYPE
	{
		NONE = -1,
		PUNCH, // �p���`
		AXE,   // ��
	};

	WeaponBase(void) {}
	virtual ~WeaponBase(void) {}

	virtual void Init(TYPE type) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;

	virtual bool IsAlive() const = 0;

	virtual VECTOR GetPos() const = 0;

	virtual float GetCollisionRadius() const = 0;


	// �U������s����֐��i���W�ƌ�����󂯎��j
	virtual void Use(VECTOR pos, VECTOR dir) = 0;
};
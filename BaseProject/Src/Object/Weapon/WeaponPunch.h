#pragma once
#include <DxLib.h>
#include "WeaponBase.h"

// =============================================================================
// �p���`����N���X
// =============================================================================
class WeaponPunch : public WeaponBase
{
public:
	// �X�v���C�g�������i��4 �~ �c3 = 12�R�}�j
	static constexpr int NUM_SPRITE_X = 4;
	static constexpr int NUM_SPRITE_Y = 3;
	static constexpr int NUM_SPRITE_ALL = NUM_SPRITE_X * NUM_SPRITE_Y;

	// �X�v���C�g��1�R�}������̃T�C�Y
	static constexpr int SIZE_XSPRITE_X = 100;
	static constexpr int SIZE_XSPRITE_Y = 80;

	static constexpr int SIZE_YSPRITE_X = 80;
	static constexpr int SIZE_YSPRITE_Y = 100;

	// �A�j���[�V�����Đ����x�i�t���[�����̐i�s�j
	static constexpr float ANIM_SPEED = 0.5f;

	WeaponPunch(void);
	virtual ~WeaponPunch(void);

	void Init(TYPE type) override;
	void Update(void) override;
	void Draw(void) override;

	// �U���̔����i�v���C���[�̍��W�ƌ�����󂯎��j
	void Use(VECTOR pos, VECTOR dir) override;

	// �U�������ǂ����̎擾
	bool IsUse(void) const { return isUse_; }

	// �����蔻��p
  bool IsAlive(void) const override { return isUse_; }
	VECTOR GetPos(void) const { return pos_; }
	float GetCollisionRadius(void) const { return 40.0f; }

private:
	TYPE type_;
	int imgHandles_[NUM_SPRITE_ALL]; // �����ǂݍ��݂����A�j���[�V�����摜
	float animFrame_;                // ���݂̃A�j���[�V�����R�}�i���������_�ŊǗ��j
	bool isUse_;                     // �U�����t���O
	VECTOR pos_;                     // �p���`�G�t�F�N�g�̕\��3D���W
	VECTOR dir_;                     // �U���̌���


};
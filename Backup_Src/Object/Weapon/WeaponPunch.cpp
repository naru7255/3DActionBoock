#include "WeaponPunch.h"
#include "../../Utility/AsoUtility.h"

WeaponPunch::WeaponPunch(void) :
	type_(TYPE::NONE),
	animFrame_(0.0f),
	isUse_(false),
	pos_({ 0.0f, 0.0f, 0.0f }),
	dir_({ 0.0f, 0.0f, 0.0f })
{
	for (int i = 0; i < NUM_SPRITE_ALL; ++i)
	{
		imgHandles_[i] = -1;
	}
}

WeaponPunch::~WeaponPunch(void)
{
	for (int i = 0; i < NUM_SPRITE_ALL; ++i)
	{
		if (imgHandles_[i] != -1)
		{
			DeleteGraph(imgHandles_[i]);
			imgHandles_[i] = -1;
		}
	}
}

void WeaponPunch::Init(TYPE type)
{
	type_ = type;

	// �p���`�X�v���C�g�V�[�g�i��4�R�} �~ �c3�R�} = �v12�R�}�j�𕪊��ǂݍ���
	LoadDivGraph(
		"Data/Image/Punch.png",
		NUM_SPRITE_ALL,
		NUM_SPRITE_X,
		NUM_SPRITE_Y,
		SIZE_XSPRITE_X,
		SIZE_XSPRITE_Y,
		imgHandles_
	);
}

void WeaponPunch::Update(void)
{
	if (!isUse_) return;

	// �R�}�����i�߂�
	animFrame_ += ANIM_SPEED;

	// �S�R�}�Đ����I�������U���I��
	if (animFrame_ >= static_cast<float>(NUM_SPRITE_ALL))
	{
		animFrame_ = 0.0f;
		isUse_ = false;
	}
}

void WeaponPunch::Draw(void)
{
	if (!isUse_) return;

	int index = static_cast<int>(animFrame_);
	if (index >= 0 && index < NUM_SPRITE_ALL && imgHandles_[index] != -1)
	{
		// 3D��Ԃ�pos_�ʒu�ɁA�J�����ɐ��΂���r���{�[�h�Ƃ��ĕ`��
		DrawBillboard3D(
			pos_,
			0.5f, 0.5f,                    // �摜�̒��S��A���J�[�ɂ���iX:0.5, Y:0.5�j
			static_cast<float>(SIZE_XSPRITE_X), // �\����
			0.0f,                          // ��]�p
			imgHandles_[index],            // �摜�n���h��
			TRUE                           // ���߂���
		);
	}
}

void WeaponPunch::Use(VECTOR pos, VECTOR dir)
{
	// �v���C���[�̑O���������ꂽ�ʒu�i�r��L�΂�����j�ɃG�t�F�N�g�𔭐�������
	const float OFFSET_DIST = 40.0f;
	const float OFFSET_Y = 30.0f;

	pos_ = VAdd(pos, VScale(dir, OFFSET_DIST));
	pos_.y += OFFSET_Y; // ���̍���
	dir_ = dir;

	animFrame_ = 0.0f;
	isUse_ = true;
}
#include "EnemyBase.h"
#include "../Player/Player.h"
#include "../Player/AnimationController.h"
#include "../../Utility/AsoUtility.h"
#include "../Shot/ShotBase.h"
#include "../Shot/ShotStraight.h"

// �萔�̒�`�i�w�b�_�[��cpp�̏㕔�Ȃǁj
static constexpr COLOR_F COLOR_DIF_DEFAULT = { 0.4f, 0.4f, 0.4f, 1.0f };
static constexpr COLOR_F COLOR_DIF_BLINK   = { 1.0f, 0.2f, 0.2f, 1.0f };
static constexpr int TERM_BLINK = 8;
static constexpr int CNT_HIT_REACT = 40;
static constexpr int CNT_DEAD_REACT = 80;

EnemyBase::EnemyBase(void) :
	modelId_(-1),
	pos_({ 0.0f, 0.0f, 0.0f }),
	angles_({ 0.0f, 0.0f, 0.0f }),
	scales_({ 0.5f, 0.5f, 0.5f }),
	player_(nullptr),
	moveDir_({ 0.0f, 0.0f, 0.0f }),
	speed_(5.0f),
	hp_(0),
	spawnRange_(0.0f),
	animController_(nullptr),
	hitTimer_(0),
	deadTimer_(0),
	collisionRadius_(50.0f),
	state_(STATE::STANDBY),
	baseAttackEffectModelId_(-1),
	shotModelId_(-1),
	cntAttack_(0)
{
}

EnemyBase::~EnemyBase(void)
{
	Release();
}

// ���������i���� 31�� �����FbaseAttackEffectModelId �����t���j
void EnemyBase::Init(TYPE type, int baseModelId, int baseAttackEffectModelId, Player* player)
{
	type_ = type;
	player_ = player;
	baseAttackEffectModelId_ = baseAttackEffectModelId;
	shotModelId_ = baseAttackEffectModelId;

	// �x�[�X���f���𕡐�����
	modelId_ = MV1DuplicateModel(baseModelId);

	// �� �h���N���X���Ƃ̃p�����[�^�i�X�s�[�h��X�P�[���Ȃǁj��K�p����
	SetParam();

	if (modelId_ != -1)
	{
		MV1SetScale(modelId_, scales_);
		MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
		// ���Ȕ����F��ݒ肵�Ė��邭����i���� 27�� �����j
		MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
	}
	// �A �����_���ȏo���ʒu�̐ݒ�
	SetSpawnPosition();

	// �B �A�j���[�V�����R���g���[���̏������ƃA�j���[�V�����̓o�^
	animController_ = new AnimationController(modelId_);

	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::ATTACK), 30.0f, static_cast<int>(ANIM_TYPE::ATTACK));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::DEATH), 30.0f, static_cast<int>(ANIM_TYPE::DEATH));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::HIT_REACT), 30.0f, static_cast<int>(ANIM_TYPE::HIT_REACT));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 30.0f, static_cast<int>(ANIM_TYPE::IDLE));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP), 30.0f, static_cast<int>(ANIM_TYPE::JUMP));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN), 30.0f, static_cast<int>(ANIM_TYPE::RUN));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 30.0f, static_cast<int>(ANIM_TYPE::WALK));

	// �� WALK �A�j���[�V��������[�v�Đ�
	animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);

	//  ��Ԃ� STANDBY �ɐݒ�
	ChangeState(STATE::STANDBY);
}

// �݊��p������
void EnemyBase::Init(TYPE type, int baseModelId, Player* player)
{
	Init(type, baseModelId, -1, player);
}

void EnemyBase::Update(void)
{
	// �I���iEND�j��Ԃ̎��͏�����s��Ȃ�
	if (state_ == STATE::END) return;

	// �� ��Ԃ��ƂɍX�V�𕪊�
	switch (state_)
	{
	case EnemyBase::STATE::STANDBY:
		UpdateStandby();
		break;
	case EnemyBase::STATE::ATTACK:
		UpdateAttack();
		break;
	case EnemyBase::STATE::HIT_REACT:
		UpdateHitReact();
		break;
	case EnemyBase::STATE::DEAD_REACT:
		UpdateDeadReact();
		break;
	case EnemyBase::STATE::END:
		break;
	}

	// �e�̍X�V����
	UpdateShots();

	// �A�j���[�V�����̍X�V
	if (animController_ != nullptr)
	{
		animController_->Update();
	}

	// ���W�Ɖ�]�̔��f
	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		MV1SetRotationXYZ(modelId_, angles_);
	}
}

void EnemyBase::Draw(void)
{
	// �I���iEND�j��Ԃ̎��͕`���s��Ȃ�
	if (state_ == STATE::END) return;

	// �� ��Ԃ��Ƃɕ`�揈���𕪊�
	switch (state_)
	{
	case EnemyBase::STATE::STANDBY:
		DrawStandby();
		break;
	case EnemyBase::STATE::ATTACK:
		DrawAttack();
		break;
	case EnemyBase::STATE::HIT_REACT:
		DrawHitReact();
		break;
	case EnemyBase::STATE::DEAD_REACT:
		DrawDeadReact();
		break;
	case EnemyBase::STATE::END:
		break;
	}

	// �e�̕`�揈��
	DrawShots();

#ifdef _DEBUG
	// デバッグ用衝突判定の球の描画（当たり判定が有効なSTANDBYまたはATTACK時のみ描画）
	if (IsCollisionState())
	{
		DrawSphere3D(pos_, collisionRadius_, 10, 0x0000ff, 0x0000ff, false);
	}
#endif
}

void EnemyBase::Release(void)
{
	if (animController_ != nullptr)
	{
		animController_->Release();
		delete animController_;
		animController_ = nullptr;
	}

	for (auto* shot : shots_)
	{
		if (shot != nullptr)
		{
			shot->Release();
			delete shot;
		}
	}
	shots_.clear();

	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

void EnemyBase::SetSpawnPosition(void)
{
	VECTOR centerPos = { 0.0f, 0.0f, 0.0f };
	float radius = 800.0f * spawnRange_;

	float randomDeg = static_cast<float>(rand() % 360);
	float randomRad = AsoUtility::Deg2RadF(randomDeg);

	VECTOR dir = { sinf(randomRad), 0.0f, cosf(randomRad) };
	VECTOR offset = VScale(dir, radius);

	pos_ = VAdd(centerPos, offset);
	pos_.y = 50.0f;
}

void EnemyBase::LookPlayer(void)
{
	if (player_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();
	VECTOR toPlayer = VSub(playerPos, pos_);
	toPlayer.y = 0.0f;

	if (VSquareSize(toPlayer) > 0.0f)
	{
		moveDir_ = VNorm(toPlayer);
		angles_.y = atan2f(moveDir_.x, moveDir_.z) + AsoUtility::Deg2RadF(180.0f);
	}
}

void EnemyBase::Move(void)
{
	VECTOR velocity = VScale(moveDir_, speed_);
	pos_ = VAdd(pos_, velocity);
}

void EnemyBase::ChangeState(STATE state)
{
	state_ = state;

	switch (state_)
	{
	case STATE::STANDBY:
		ChangeStandby();
		break;
	case STATE::ATTACK:
		ChangeAttack();
		break;
	case STATE::HIT_REACT:
		ChangeHitReact();
		break;
	case STATE::DEAD_REACT:
		ChangeDeadReact();
		break;
	case STATE::END:
		// �I�����͓��ʂȏ����͕s�v
		break;
	}
}

void EnemyBase::ChangeStandby(void)
{
	if (modelId_ != -1)
	{
		MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
	}
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
	}
	cntAttack_ = 0;
}

void EnemyBase::ChangeAttack(void)
{
	if (modelId_ != -1)
	{
		MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
	}
	if (animController_ != nullptr)
	{
		// �U���A�j���[�V�����Đ��i���[�v�����j
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
	}

	// �e�𐶐��E���˂���i���� 31�� �����j
	if (baseAttackEffectModelId_ != -1)
	{
		Shot();
	}
}

void EnemyBase::ChangeHitReact(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::HIT_REACT), true);
	}
	hitTimer_ = CNT_HIT_REACT;
}

void EnemyBase::ChangeDeadReact(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::DEATH), false);
	}
	deadTimer_ = CNT_DEAD_REACT;
}

void EnemyBase::UpdateStandby(void)
{
	LookPlayer();
	Move();
}

void EnemyBase::UpdateAttack(void)
{
	LookPlayer();

	// �U���A�j���[�V�������I�������� STANDBY ��Ԃֈڍs�i���� 30�� �����j
	if (animController_ != nullptr && animController_->IsEnd())
	{
		ChangeState(STATE::STANDBY);
	}
}

void EnemyBase::UpdateHitReact(void)
{
	hitTimer_--;

	if (modelId_ != -1)
	{
		if ((hitTimer_ / TERM_BLINK) % 2 == 0)
		{
			MV1SetDifColorScale(modelId_, COLOR_DIF_BLINK);
		}
		else
		{
			MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
		}
	}

	if (hitTimer_ <= 0)
	{
		ChangeState(STATE::STANDBY);
	}
}

void EnemyBase::UpdateDeadReact(void)
{
	deadTimer_--;

	// �_�ŏ���
	if (modelId_ != -1)
	{
		if ((deadTimer_ / TERM_BLINK) % 2 == 0)
		{
			MV1SetDifColorScale(modelId_, COLOR_DIF_BLINK);
		}
		else
		{
			MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
		}
	}

	// �A�j���[�V�����I���A�܂��̓^�C�}�[�I���� END ��Ԃ�
	if ((animController_ != nullptr && animController_->IsEnd()) || deadTimer_ <= 0)
	{
		ChangeState(STATE::END);
	}
}

void EnemyBase::DrawStandby(void)
{
	if (modelId_ != -1) { MV1DrawModel(modelId_); }
}

void EnemyBase::DrawAttack(void)
{
	if (modelId_ != -1) { MV1DrawModel(modelId_); }
}

void EnemyBase::DrawHitReact(void)
{
	if (modelId_ != -1) { MV1DrawModel(modelId_); }
}

void EnemyBase::DrawDeadReact(void)
{
	if (modelId_ != -1) { MV1DrawModel(modelId_); }
}

void EnemyBase::UpdateShots(void)
{
	for (auto& shot : shots_)
	{
		if (shot != nullptr && shot->IsAlive())
		{
			shot->Update();
		}
	}
}

void EnemyBase::DrawShots(void)
{
	for (auto& shot : shots_)
	{
		if (shot != nullptr && shot->IsAlive())
		{
			shot->Draw();
		}
	}
}

// �e�̔��ˁi���� 31�� �����j
void EnemyBase::Shot(void)
{
	if (baseAttackEffectModelId_ == -1) return;

	// �L���Ȓe��擾����
	ShotBase* shot = GetValidShot();
	if (shot != nullptr)
	{
		VECTOR shotPos = pos_;
		shotPos.y += 40.0f; // ���E���̍������甭��
		shot->CreateShot(shotPos, moveDir_);
	}
}

// �L���Ȓe��擾
ShotBase* EnemyBase::GetValidShot(void)
{
	for (size_t i = 0; i < shots_.size(); i++)
	{
		// ���g�p�i�������Ă��Ȃ��j�e������΍ė��p
		if (!shots_[i]->IsAlive())
		{
			return shots_[i];
		}
	}

	// �V�����e�̃C���X�^���X�𐶐�����
	ShotBase* shot = new ShotStraight(ShotBase::TYPE::STRAIGHT, baseAttackEffectModelId_);
	shots_.push_back(shot);
	return shot;
}

bool EnemyBase::IsCollisionState(void) const
{
	return state_ == STATE::STANDBY || state_ == STATE::ATTACK;
}

bool EnemyBase::IsAlive(void) const
{
	return hp_ > 0 && state_ != STATE::END;
}

void EnemyBase::Damage(int damage)
{
	if (!IsCollisionState()) return;

	hp_ -= damage;

	if (hp_ > 0)
	{
		ChangeState(STATE::HIT_REACT);
	}
	else
	{
		ChangeState(STATE::DEAD_REACT);
	}
}

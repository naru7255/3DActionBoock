#include "Player.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/InputManager.h"
#include "../../Utility/AsoUtility.h"
#include "AnimationController.h"
#include "../Weapon/WeaponPunch.h"
#include "../Weapon/WeaponAxe.h"
#include <cmath>

Player::Player(void) :
	modelId_(-1),
	jumpPow_(0.0f),
	isJump_(false),
	isLanding_(false),
	hp_(MAX_HP),
	pos_(DEFAULT_POS),
	prevPos_(DEFAULT_POS),
	angles_({ 0.0f, 0.0f, 0.0f }),
	scales_(SCALES),
	state_(STATE::NONE),
	knockBackDir_({ 0.0f, 0.0f, 0.0f }),
	cntKnockBack_(0),
	animType_(ANIM_TYPE::IDLE),
	animController_(nullptr),
	useWeapon_(nullptr),
	moveDir_({ 0.0f, 0.0f, 1.0f })
{
}

Player::~Player(void)
{
	Release();
}

void Player::Init(void)
{
	modelId_ = MV1LoadModel("Data/Model/Player/Player.mv1");
	pos_ = DEFAULT_POS;
	prevPos_ = DEFAULT_POS;
	scales_ = SCALES;
	jumpPow_ = 0.0f;
	isJump_ = false;
	isLanding_ = false;

	if (modelId_ != -1)
	{
		MV1SetScale(modelId_, scales_);
		angles_ = { 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f };
		MV1SetRotationXYZ(modelId_, angles_);
		MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
		MV1SetPosition(modelId_, pos_);
	}

	animController_ = new AnimationController(modelId_);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::DEATH), 30.0f, 0);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::DUCK), 30.0f, 1);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::HIT_REACT), 30.0f, 2);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 30.0f, 3);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE_ATTACK), 30.0f, 4);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE_HOLD), 30.0f, 5);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP), 30.0f, 6);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP_IDLE), 30.0f, 7);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP_LAND), 30.0f, 8);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::NO), 30.0f, 9);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::PUNCH), 30.0f, 10);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN), 30.0f, 11);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN_ATTACK), 30.0f, 12);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN_HOLD), 30.0f, 13);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 30.0f, 14);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK_HOLD), 30.0f, 15);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WAVE), 30.0f, 16);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::YES), 30.0f, 17);

	// --- 武器生成とstd::mapへの登録 ---
	// 1. パンチ武器
	WeaponPunch* punch = new WeaponPunch();
	punch->Init(WeaponBase::TYPE::PUNCH);
	weapons_[WeaponBase::TYPE::PUNCH] = punch;

	// 2. 斧武器
	WeaponAxe* axe = new WeaponAxe();
	axe->Init(WeaponBase::TYPE::AXE);
	weapons_[WeaponBase::TYPE::AXE] = axe;

	// 初期装備はパンチ
	useWeapon_ = weapons_[WeaponBase::TYPE::PUNCH];

	ChangeState(STATE::STANDBY);
}

void Player::Update(float cameraAngleY)
{
	prevPos_ = pos_;

	switch (state_)
	{
	case STATE::STANDBY:
		UpdateStandby(cameraAngleY);
		break;
	case STATE::KNOCKBACK:
		UpdateKnockback();
		break;
	case STATE::ATTACK:
		// 攻撃アニメーション終了でSTANDBYへ復帰
		if (animController_ != nullptr && animController_->IsEnd())
		{
			ChangeState(STATE::STANDBY);
		}
		break;
	case STATE::DEAD:
		UpdateDead();
		break;
	case STATE::END:
		break;
	}

	Respawn();

	// --- 装備中武器の更新 ---
	if (useWeapon_ != nullptr)
	{
		useWeapon_->Update();
	}

	if (animController_ != nullptr)
	{
		animController_->Update();
		if (isLanding_ && animController_->IsEnd())
		{
			isLanding_ = false;
		}
	}

	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		MV1SetRotationXYZ(modelId_, angles_);
	}
}

void Player::Draw(void)
{
	if (modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}

	// --- 装備中武器の描画 ---
	if (useWeapon_ != nullptr)
	{
		useWeapon_->Draw();
	}
}

void Player::Release(void)
{
	// --- std::map に登録された全武器を解放 ---
	for (auto& pair : weapons_)
	{
		if (pair.second != nullptr)
		{
			delete pair.second;
		}
	}
	weapons_.clear();
	useWeapon_ = nullptr;

	if (animController_ != nullptr)
	{
		animController_->Release();
		delete animController_;
		animController_ = nullptr;
	}

	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

void Player::ChangeState(STATE state)
{
	state_ = state;

	switch (state_)
	{
	case STATE::STANDBY:
		ChangeStandby();
		break;
	case STATE::KNOCKBACK:
		ChangeKnockback();
		break;
	case STATE::DEAD:
		ChangeDead();
		break;
	case STATE::ATTACK:
		break;
	}
}

void Player::ChangeStandby(void)
{
	if (modelId_ != -1)
	{
		MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
	}
}

void Player::UpdateStandby(float cameraAngleY)
{
	ProcessMove(cameraAngleY);
	ProcessJump();
	ProcessAttack();       // 攻撃入力
	ProcessChangeWeapon(); // 武器切り替え入力
}

void Player::KnockBack(VECTOR dirXZ, float jumpPow)
{
	if (state_ == STATE::KNOCKBACK || state_ == STATE::DEAD || state_ == STATE::END) return;

	knockBackDir_ = dirXZ;
	jumpPow_ = jumpPow;
	ChangeState(STATE::KNOCKBACK);
}

void Player::ChangeKnockback(void)
{
	isJump_ = true;
	isLanding_ = false;
	cntKnockBack_ = 0;

	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::HIT_REACT), false);
	}
}

void Player::UpdateKnockback(void)
{
	cntKnockBack_++;

	jumpPow_ -= GRAVITY_POW;
	pos_.y += jumpPow_;

	VECTOR velocity = VScale(knockBackDir_, SPEED_KNOCKBACK);
	pos_.x += velocity.x;
	pos_.z += velocity.z;

	if (animController_ != nullptr &&
		animController_->GetPlayType() == static_cast<int>(ANIM_TYPE::HIT_REACT) &&
		animController_->IsEnd())
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP_IDLE), true);
	}

	if (modelId_ != -1)
	{
		if ((cntKnockBack_ / 5) % 2 == 0)
		{
			MV1SetDifColorScale(modelId_, { 5.0f, 0.5f, 0.5f, 1.0f });
		}
		else
		{
			MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
		}
	}
}

void Player::ProcessMove(float cameraAngleY)
{
	VECTOR inputDir = { 0.0f, 0.0f, 0.0f };

	auto& input = InputManager::GetInstance();
	if (input.IsNew(KEY_INPUT_W)) { inputDir = VAdd(inputDir, AsoUtility::DIR_F); }
	if (input.IsNew(KEY_INPUT_S)) { inputDir = VAdd(inputDir, AsoUtility::DIR_B); }
	if (input.IsNew(KEY_INPUT_A)) { inputDir = VAdd(inputDir, AsoUtility::DIR_L); }
	if (input.IsNew(KEY_INPUT_D)) { inputDir = VAdd(inputDir, AsoUtility::DIR_R); }

	if (state_ == STATE::STANDBY)
	{
		if (VSquareSize(inputDir) > 0.0f)
		{
			if (!isJump_ && !isLanding_)
			{
				animController_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
			}

			inputDir = VNorm(inputDir);

			VECTOR moveDir = { 0.0f, 0.0f, 0.0f };
			float sinY = sinf(cameraAngleY);
			float cosY = cosf(cameraAngleY);

			moveDir.x = inputDir.x * cosY + inputDir.z * sinY;
			moveDir.z = -inputDir.x * sinY + inputDir.z * cosY;

			// 移動向きを保存（攻撃の向きに使用）
			moveDir_ = moveDir;

			VECTOR velocity = VScale(moveDir, SPEED_MOVE);
			pos_ = VAdd(pos_, velocity);

			angles_.y = atan2f(moveDir.x, moveDir.z) + AsoUtility::Deg2RadF(180.0f);
		}
		else
		{
			if (!isJump_ && !isLanding_)
			{
				animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
			}
		}
	}
}

void Player::ProcessJump(void)
{
	InputManager& input = InputManager::GetInstance();

	jumpPow_ -= GRAVITY_POW;

	if (input.IsTrgDown(KEY_INPUT_SPACE) && !isJump_ && !isLanding_)
	{
		isJump_ = true;
		jumpPow_ = JUMP_POW;
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP), false);
	}

	if (isJump_ && animController_ != nullptr &&
		animController_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP) &&
		animController_->IsEnd())
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP_IDLE), true);
	}

	pos_.y += jumpPow_;
}

// 攻撃入力処理（BキーまたはJキー）
void Player::ProcessAttack(void)
{
	auto& input = InputManager::GetInstance();

	// BキーまたはJキーで攻撃。ジャンプ中でなく、現在武器が使用中でないとき
	bool isAttackKey = input.IsTrgDown(KEY_INPUT_B) || input.IsTrgDown(KEY_INPUT_J);
	bool canAttack = (useWeapon_ != nullptr && !useWeapon_->IsAlive());

	if (isAttackKey && !isJump_ && canAttack)
	{
		// 1. 攻撃状態へ遷移
		ChangeState(STATE::ATTACK);

		// 2. 現在の武器を使用
		useWeapon_->Use(pos_, moveDir_);

		// 3. アニメーション再生（パンチまたは攻撃）
		if (animController_ != nullptr)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::PUNCH), false);
		}
	}
}

// 武器切り替え入力処理（Cキー）
void Player::ProcessChangeWeapon(void)
{
	auto& input = InputManager::GetInstance();

	// Cキーが押され、かつ武器を使用中でないときに切り替え
	if (input.IsTrgDown(KEY_INPUT_C) && (useWeapon_ == nullptr || !useWeapon_->IsAlive()))
	{
		// パンチなら斧へ、斧ならパンチへトグル切り替え
		if (useWeapon_ == weapons_[WeaponBase::TYPE::PUNCH])
		{
			useWeapon_ = weapons_[WeaponBase::TYPE::AXE];
		}
		else
		{
			useWeapon_ = weapons_[WeaponBase::TYPE::PUNCH];
		}
	}
}

void Player::CollisionStage(float groundY)
{
	if (state_ == STATE::DEAD || state_ == STATE::END) return;

	bool wasJumping = isJump_;
	pos_.y = groundY;
	jumpPow_ = 0.0f;
	isJump_ = false;

	if (wasJumping && animController_ != nullptr)
	{
		isLanding_ = true;
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP_LAND), false);
	}

	if (state_ == STATE::KNOCKBACK)
	{
		ChangeState(STATE::STANDBY);
		if (modelId_ != -1)
		{
			MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
		}
	}
}

void Player::Respawn(void)
{
	if (state_ == STATE::DEAD || state_ == STATE::END) return;

	if (pos_.y < -1500.0f)
	{
		pos_.x = 0.0f;
		pos_.z = 0.0f;
		pos_.y = 150.0f;

		prevPos_ = pos_;

		if (modelId_ != -1)
		{
			MV1SetPosition(modelId_, pos_);
		}

		jumpPow_ = 0.0f;
		isJump_ = true;

		ChangeState(STATE::STANDBY);
		if (animController_ != nullptr)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
		}
	}
}

void Player::Damage(int damage)
{
	if (IsInvincible()) return;

	hp_ -= damage;
	if (hp_ <= 0)
	{
		hp_ = 0;
		ChangeState(STATE::DEAD);
	}
}

void Player::ChangeDead(void)
{
	jumpPow_ = 0.0f;
	isLanding_ = false;
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::DEATH), false);
	}
}

void Player::UpdateDead(void)
{
	if (animController_ != nullptr && animController_->IsEnd())
	{
		ChangeState(STATE::END);
	}
}

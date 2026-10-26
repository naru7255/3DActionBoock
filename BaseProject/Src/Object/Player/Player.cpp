#include "Player.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/InputManager.h"
#include "../../Utility/AsoUtility.h"
#include "AnimationController.h"
#include <cmath>

Player::Player(void) :
	modelId_(-1),
	jumpPow_(0.0f),
	isJump_(false),
	isLanding_(false),
	hp_(MAX_HP),
	pos_(DEFAULT_POS),
	prevPos_(DEFAULT_POS), // 初期化
	angles_({ 0.0f, 0.0f, 0.0f }),
	scales_(SCALES),
	state_(STATE::NONE),
	knockBackDir_({ 0.0f, 0.0f, 0.0f }),
	cntKnockBack_(0),
	animType_(ANIM_TYPE::IDLE),
	animController_(nullptr)
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
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::DEATH), 30.0f, 0);       // 死亡
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::DUCK), 30.0f, 1);        // しゃがみ
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::HIT_REACT), 30.0f, 2);   // 被弾
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 30.0f, 3);        // 待機
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE_ATTACK), 30.0f, 4); // 待機攻撃
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE_HOLD), 30.0f, 5);   // 物持ち待機
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP), 30.0f, 6);        // ジャンプ開始
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP_IDLE), 30.0f, 7);   // 空中待機
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP_LAND), 30.0f, 8);   // 着地
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::NO), 30.0f, 9);          // 否定
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::PUNCH), 30.0f, 10);      // パンチ
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN), 30.0f, 11);        // 走り
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN_ATTACK), 30.0f, 12); // 走り攻撃
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN_HOLD), 30.0f, 13);   // 物持ち走り
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 30.0f, 14);       // 歩き
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK_HOLD), 30.0f, 15);  // 物持ち歩き
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WAVE), 30.0f, 16);       // 手を振る
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::YES), 30.0f, 17);        // 肯定

	ChangeState(STATE::STANDBY);
}

void Player::Update(float cameraAngleY)
{
	// 【重要】移動する前の安全な座標を記録しておく！
	prevPos_ = pos_;

	switch (state_)
	{
	case STATE::STANDBY:
		UpdateStandby(cameraAngleY);
		break;
	case STATE::KNOCKBACK:
		UpdateKnockback();
		break;
	case STATE::DEAD:
		UpdateDead();
		break;
	case STATE::END:
		break;
	}

	Respawn();

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
}

void Player::Release(void)
{
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

			// カメラの向き（cameraAngleY）を基準に入力方向を回転
			VECTOR moveDir = { 0.0f, 0.0f, 0.0f };
			float sinY = sinf(cameraAngleY);
			float cosY = cosf(cameraAngleY);

			moveDir.x = inputDir.x * cosY + inputDir.z * sinY;
			moveDir.z = -inputDir.x * sinY + inputDir.z * cosY;

			VECTOR velocity = VScale(moveDir, SPEED_MOVE);
			pos_ = VAdd(pos_, velocity);

			// キャラクターを進行方向に向ける
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

	// ステージの下（奈落）に落下した場合
	if (pos_.y < -1500.0f)
	{
		// 1. ステージの中心（X=0, Z=0）の少し上（Y=150）にリスポーン
		pos_.x = 0.0f;
		pos_.z = 0.0f;
		pos_.y = 150.0f; // 地面の少し上から着地させる

		// 2. 【超重要】移動前の座標も真ん中に揃える（当たり判定の暴走防止）
		prevPos_ = pos_;

		// 3. 【超重要】3Dモデルも即座に真ん中へワープさせる
		if (modelId_ != -1)
		{
			MV1SetPosition(modelId_, pos_);
		}

		// 4. ジャンプ力や落下速度をゼロにリセット
		jumpPow_ = 0.0f;
		isJump_ = true;

		// 5. 待機状態に戻して待機モーションを再生
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
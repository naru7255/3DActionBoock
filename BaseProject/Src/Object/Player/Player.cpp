#include "Player.h"
#include "../../Manager/SceneManager.h"
#include "../../Manager/InputManager.h"
#include "../../Utility/AsoUtility.h"
#include "AnimationController.h"
#include "../Weapon/WeaponPunch.h"
#include "../Weapon/WeaponAxe.h"
#include "../Weapon/WeaponDrill.h"
#include <cmath>

// =============================================================================
// コンストラクタ
// =============================================================================
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
	animController_(nullptr),
	useWeapon_(nullptr),
	moveDir_({ 0.0f, 0.0f, 1.0f })
{
}

// =============================================================================
// デストラクタ
// =============================================================================
Player::~Player(void)
{
	Release();
}

// =============================================================================
// 初期化処理
// 3Dモデルのロード、アニメーションの登録、武器の生成と登録を行う
// =============================================================================
void Player::Init(void)
{
	// 1. 3Dモデルの読み込みと基本設定
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

	// 2. アニメーション制御クラスの生成と各モーションの登録（FBX内インデックス）
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

	// 3. 武器の生成と連想配列（std::map）への登録
	// パンチ武器の初期化
	WeaponPunch* punch = new WeaponPunch();
	punch->Init(WeaponBase::TYPE::PUNCH);
	weapons_[WeaponBase::TYPE::PUNCH] = punch;

	// 斧武器の初期化
	WeaponAxe* axe = new WeaponAxe();
	axe->Init(WeaponBase::TYPE::AXE);
	weapons_[WeaponBase::TYPE::AXE] = axe;

	// ドリル武器の初期化（教材第42章追加実装）
	WeaponDrill* drill = new WeaponDrill();
	drill->Init(WeaponBase::TYPE::DRILL);
	weapons_[WeaponBase::TYPE::DRILL] = drill;

	// 初期装備をパンチに設定
	useWeapon_ = weapons_[WeaponBase::TYPE::PUNCH];

	// 初期ステートを通常待機に設定
	ChangeState(STATE::STANDBY);
}

// =============================================================================
// 毎フレーム更新処理
// =============================================================================
void Player::Update(float cameraAngleY)
{
	// 1フレーム前の座標を保存（壁の押し戻し判定などで使用）
	prevPos_ = pos_;

	// 行動ステートに応じた更新処理の呼び出し
	switch (state_)
	{
	case STATE::STANDBY:
		UpdateStandby(cameraAngleY);
		break;
	case STATE::KNOCKBACK:
		UpdateKnockback();
		break;
	case STATE::ATTACK:
		// 攻撃アニメーション終了で通常待機（STANDBY）へ復帰
		if (animController_ != nullptr && animController_->IsEnd())
		{
			ChangeState(STATE::STANDBY);
		}
		break;
	case STATE::DEAD:
		UpdateDead();
		break;
	case STATE::END:
		// 死亡確定時は何もしない（GameScene側でゲームオーバー表示）
		break;
	case STATE::VICTORY:
		UpdateVictory();
		break;
	}

	// 落下リスポーン判定（勝利中はリスポーンしない）
	if (state_ != STATE::VICTORY)
	{
		Respawn();
	}

	// 装備中武器の更新処理（勝利中は武器の更新不要）
	if (useWeapon_ != nullptr && state_ != STATE::VICTORY)
	{
		useWeapon_->Update();
	}

	// アニメーション更新処理
	if (animController_ != nullptr)
	{
		animController_->Update();
		// 着地モーション終了検知
		if (isLanding_ && animController_->IsEnd())
		{
			isLanding_ = false;
		}
	}

	// 3Dモデルへの座標と回転角度の反映
	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		MV1SetRotationXYZ(modelId_, angles_);
	}
}

// =============================================================================
// 描画処理
// =============================================================================
void Player::Draw(void)
{
	// プレイヤーモデルの描画
	if (modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}

	// 装備中武器の描画（勝利中は描画しない）
	if (useWeapon_ != nullptr && state_ != STATE::VICTORY)
	{
		useWeapon_->Draw();
	}
}

// =============================================================================
// 解放処理
// =============================================================================
void Player::Release(void)
{
	// 登録されたすべての武器インスタンスを解放
	for (auto& pair : weapons_)
	{
		if (pair.second != nullptr)
		{
			delete pair.second;
		}
	}
	weapons_.clear();
	useWeapon_ = nullptr;

	// アニメーションコントローラーの解放
	if (animController_ != nullptr)
	{
		animController_->Release();
		delete animController_;
		animController_ = nullptr;
	}

	// 3Dモデルハンドルの破棄
	if (modelId_ != -1)
	{
		MV1DeleteModel(modelId_);
		modelId_ = -1;
	}
}

// =============================================================================
// ステート（状態）切り替え
// =============================================================================
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
	case STATE::VICTORY:
		ChangeVictory();
		break;
	}
}

// 通常待機状態への切り替え（マテリアルカラーを通常色へ戻す）
void Player::ChangeStandby(void)
{
	if (modelId_ != -1)
	{
		MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
	}
}

// 通常待機状態の更新処理（移動、ジャンプ、攻撃、武器切り替えの入力を受付）
void Player::UpdateStandby(float cameraAngleY)
{
	ProcessMove(cameraAngleY);
	ProcessJump();
	ProcessAttack();       // 攻撃入力
	ProcessChangeWeapon(); // 武器切り替え入力
}

// =============================================================================
// ゲームクリア勝利演出（教材第41章）
// =============================================================================
void Player::Victory(void)
{
	if (state_ == STATE::DEAD || state_ == STATE::END) return;
	ChangeState(STATE::VICTORY);
}

// 勝利ステート開始処理
void Player::ChangeVictory(void)
{
	jumpPow_ = 0.0f;
	isJump_ = false;
	isLanding_ = false;

	// カメラ側（Zマイナス方向＝手前）を向かせる（教材第41章）
	angles_.x = 0.0f;
	angles_.y = 0.0f;
	angles_.z = 0.0f;

	if (modelId_ != -1)
	{
		MV1SetRotationXYZ(modelId_, angles_);
		MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
	}

	// 手を振るアニメーション（WAVE）をループ再生（教材第41章）
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::WAVE), true);
	}
}

// 勝利ステート更新処理
void Player::UpdateVictory(void)
{
	// 勝利演出中は操作を受け付けない（アニメーション再生のみ）
}

// =============================================================================
// 被弾ノックバック処理
// =============================================================================
void Player::KnockBack(VECTOR dirXZ, float jumpPow)
{
	// すでに無敵・被弾中・死亡中・勝利中なら重ねがけしない
	if (state_ == STATE::KNOCKBACK || state_ == STATE::DEAD || state_ == STATE::END || state_ == STATE::VICTORY) return;

	knockBackDir_ = dirXZ;
	jumpPow_ = jumpPow;
	ChangeState(STATE::KNOCKBACK);
}

// ノックバック状態開始時の設定
void Player::ChangeKnockback(void)
{
	isJump_ = true;
	isLanding_ = false;
	cntKnockBack_ = 0;

	// 被弾リアクションアニメーションを単発再生
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::HIT_REACT), false);
	}
}

// ノックバック中の移動・落下・点滅更新
void Player::UpdateKnockback(void)
{
	cntKnockBack_++;

	// 重力による落下
	jumpPow_ -= GRAVITY_POW;
	pos_.y += jumpPow_;

	// 水平後退移動
	VECTOR velocity = VScale(knockBackDir_, SPEED_KNOCKBACK);
	pos_.x += velocity.x;
	pos_.z += velocity.z;

	// 被弾アニメーション終了後は空中落下モーションへ
	if (animController_ != nullptr &&
		animController_->GetPlayType() == static_cast<int>(ANIM_TYPE::HIT_REACT) &&
		animController_->IsEnd())
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP_IDLE), true);
	}

	// ダメージ点滅演出（赤と通常色を交互に切り替え）
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

// =============================================================================
// 移動入力処理（WASDキー・カメラ向き連動）
// =============================================================================
void Player::ProcessMove(float cameraAngleY)
{
	VECTOR inputDir = { 0.0f, 0.0f, 0.0f };

	// キー入力の受付
	auto& input = InputManager::GetInstance();
	if (input.IsNew(KEY_INPUT_W)) { inputDir = VAdd(inputDir, AsoUtility::DIR_F); }
	if (input.IsNew(KEY_INPUT_S)) { inputDir = VAdd(inputDir, AsoUtility::DIR_B); }
	if (input.IsNew(KEY_INPUT_A)) { inputDir = VAdd(inputDir, AsoUtility::DIR_L); }
	if (input.IsNew(KEY_INPUT_D)) { inputDir = VAdd(inputDir, AsoUtility::DIR_R); }

	if (state_ == STATE::STANDBY)
	{
		// 移動入力がある場合
		if (VSquareSize(inputDir) > 0.0f)
		{
			// ジャンプ・着地中でなければ走りモーション再生
			if (!isJump_ && !isLanding_)
			{
				animController_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
			}

			inputDir = VNorm(inputDir);

			// カメラの向き（Y軸回転角度）に合わせて移動方向を回転
			VECTOR moveDir = { 0.0f, 0.0f, 0.0f };
			float sinY = sinf(cameraAngleY);
			float cosY = cosf(cameraAngleY);

			moveDir.x = inputDir.x * cosY + inputDir.z * sinY;
			moveDir.z = -inputDir.x * sinY + inputDir.z * cosY;

			// 移動方向を保持（攻撃武器の発射方向として使用）
			moveDir_ = moveDir;

			// 速度を乗算して座標を加算
			VECTOR velocity = VScale(moveDir, SPEED_MOVE);
			pos_ = VAdd(pos_, velocity);

			// キャラクターの向きを進行方向に合わせる
			angles_.y = atan2f(moveDir.x, moveDir.z) + AsoUtility::Deg2RadF(180.0f);
		}
		else
		{
			// 移動入力がない場合は待機モーション再生
			if (!isJump_ && !isLanding_)
			{
				animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
			}
		}
	}
}

// =============================================================================
// ジャンプ処理（スペースキー・重力加速度）
// =============================================================================
void Player::ProcessJump(void)
{
	InputManager& input = InputManager::GetInstance();

	// 重力加速度を加算
	jumpPow_ -= GRAVITY_POW;

	// スペースキー押下かつ接地中であればジャンプ開始
	if (input.IsTrgDown(KEY_INPUT_SPACE) && !isJump_ && !isLanding_)
	{
		isJump_ = true;
		jumpPow_ = JUMP_POW;
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP), false);
	}

	// ジャンプ上昇モーションが終了したら空中滞空モーションへ遷移
	if (isJump_ && animController_ != nullptr &&
		animController_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP) &&
		animController_->IsEnd())
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP_IDLE), true);
	}

	// 垂直移動量の反映
	pos_.y += jumpPow_;
}

// =============================================================================
// 攻撃入力処理（BキーまたはJキー）
// =============================================================================
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

		// 2. 現在装備中の武器を使用
		useWeapon_->Use(pos_, moveDir_);

		// 3. パンチ攻撃アニメーション再生
		if (animController_ != nullptr)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::PUNCH), false);
		}
	}
}

// =============================================================================
// 武器切り替え入力処理（Cキー）
// =============================================================================
void Player::ProcessChangeWeapon(void)
{
	auto& input = InputManager::GetInstance();

	// Cキーが押され、かつ武器を使用中でないときに切り替え
	if (input.IsTrgDown(KEY_INPUT_C) && (useWeapon_ == nullptr || !useWeapon_->IsAlive()))
	{
		// パンチ → 斧 → ドリル → パンチ の3武器ローテーション切り替え
		if (useWeapon_ == weapons_[WeaponBase::TYPE::PUNCH])
		{
			useWeapon_ = weapons_[WeaponBase::TYPE::AXE];
		}
		else if (useWeapon_ == weapons_[WeaponBase::TYPE::AXE])
		{
			useWeapon_ = weapons_[WeaponBase::TYPE::DRILL];
		}
		else
		{
			useWeapon_ = weapons_[WeaponBase::TYPE::PUNCH];
		}
	}
}

// =============================================================================
// 地面との衝突・接地処理（GameSceneから接地時に呼び出される）
// =============================================================================
void Player::CollisionStage(float groundY)
{
	if (state_ == STATE::DEAD || state_ == STATE::END) return;

	bool wasJumping = isJump_;
	pos_.y = groundY;
	jumpPow_ = 0.0f;
	isJump_ = false;

	// ジャンプ着地時に着地モーションを再生
	if (wasJumping && animController_ != nullptr && state_ != STATE::VICTORY)
	{
		isLanding_ = true;
		animController_->Play(static_cast<int>(ANIM_TYPE::JUMP_LAND), false);
	}

	// ノックバック中の着地であれば通常待機状態へ復帰
	if (state_ == STATE::KNOCKBACK)
	{
		ChangeState(STATE::STANDBY);
		if (modelId_ != -1)
		{
			MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
		}
	}
}

// =============================================================================
// ステージ外落下時のリスポーン処理
// =============================================================================
void Player::Respawn(void)
{
	if (state_ == STATE::DEAD || state_ == STATE::END || state_ == STATE::VICTORY) return;

	// 一定高度以下へ落下した場合に初期地点へ復帰
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

// =============================================================================
// ダメージ処理
// =============================================================================
void Player::Damage(int damage)
{
	if (IsInvincible() || state_ == STATE::VICTORY) return;

	hp_ -= damage;
	if (hp_ <= 0)
	{
		hp_ = 0;
		ChangeState(STATE::DEAD);
	}
}

// 死亡ステート開始処理
void Player::ChangeDead(void)
{
	jumpPow_ = 0.0f;
	isLanding_ = false;
	if (animController_ != nullptr)
	{
		// 倒れるアニメーションを単発再生
		animController_->Play(static_cast<int>(ANIM_TYPE::DEATH), false);
	}
}

// 死亡ステート更新処理（モーション終了でENDへ遷移）
void Player::UpdateDead(void)
{
	if (animController_ != nullptr && animController_->IsEnd())
	{
		ChangeState(STATE::END);
	}
}

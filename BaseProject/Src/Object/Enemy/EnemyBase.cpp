#include "EnemyBase.h"
#include "../Player/Player.h"
#include "../Player/AnimationController.h"
#include "../../Utility/AsoUtility.h"
#include "../Shot/ShotBase.h"
#include "../Shot/ShotStraight.h"

// 定数の定義（ヘッダーやcppの上部など）
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

// 初期処理（教材 31章 準拠：baseAttackEffectModelId 引数付き）
void EnemyBase::Init(TYPE type, int baseModelId, int baseAttackEffectModelId, Player* player)
{
	type_ = type;
	player_ = player;
	baseAttackEffectModelId_ = baseAttackEffectModelId;
	shotModelId_ = baseAttackEffectModelId;

	// ① ベースモデルを複製する
	modelId_ = MV1DuplicateModel(baseModelId);

	// ★ 派生クラスごとのパラメータ（スピードやスケールなど）を適用する
	SetParam();

	if (modelId_ != -1)
	{
		MV1SetScale(modelId_, scales_);
		MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
		// 自己発光色を設定して明るくする（教材 27章 準拠）
		MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
	}
	// ② ランダムな出現位置の設定
	SetSpawnPosition();

	// ③ アニメーションコントローラの初期化とアニメーションの登録
	animController_ = new AnimationController(modelId_);

	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::ATTACK), 30.0f, static_cast<int>(ANIM_TYPE::ATTACK));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::DEATH), 30.0f, static_cast<int>(ANIM_TYPE::DEATH));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::HIT_REACT), 30.0f, static_cast<int>(ANIM_TYPE::HIT_REACT));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 30.0f, static_cast<int>(ANIM_TYPE::IDLE));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP), 30.0f, static_cast<int>(ANIM_TYPE::JUMP));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN), 30.0f, static_cast<int>(ANIM_TYPE::RUN));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 30.0f, static_cast<int>(ANIM_TYPE::WALK));

	// ★ WALK アニメーションをループ再生
	animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);

	// ④ 状態を STANDBY に設定
	ChangeState(STATE::STANDBY);
}

// 互換用初期化
void EnemyBase::Init(TYPE type, int baseModelId, Player* player)
{
	Init(type, baseModelId, -1, player);
}

void EnemyBase::Update(void)
{
	// 終了（END）状態の時は処理を行わない
	if (state_ == STATE::END) return;

	// ★ 状態ごとに更新を分岐
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

	// 弾の更新処理
	UpdateShots();

	// アニメーションの更新
	if (animController_ != nullptr)
	{
		animController_->Update();
	}

	// 座標と回転の反映
	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		MV1SetRotationXYZ(modelId_, angles_);
	}
}

void EnemyBase::Draw(void)
{
	// 終了（END）状態の時は描画を行わない
	if (state_ == STATE::END) return;

	// ★ 状態ごとに描画処理を分岐
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

	// 弾の描画処理
	DrawShots();

#ifdef _DEBUG
	// デバッグ用衝突判定の球の描画
	DrawSphere3D(pos_, collisionRadius_, 10, 0x0000ff, 0x0000ff, false);
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
		// 終了時は特別な処理は不要
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
		// 攻撃アニメーション再生（ループ無し）
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
	}

	// 弾を生成・発射する（教材 31章 準拠）
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

	// 攻撃アニメーションが終了したら STANDBY 状態へ移行（教材 30章 準拠）
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

	// 点滅処理
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

	// アニメーション終了、またはタイマー終了で END 状態へ
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

// 弾の発射（教材 31章 準拠）
void EnemyBase::Shot(void)
{
	if (baseAttackEffectModelId_ == -1) return;

	// 有効な弾を取得する（教材 31章：オブジェクトプーリング）
	ShotBase* shot = GetValidShot();
	if (shot != nullptr)
	{
		VECTOR shotPos = pos_;
		shotPos.y += 40.0f; // 腰・胸の高さから発射
		shot->CreateShot(shotPos, moveDir_);
	}
}

// 有効な弾を取得（教材 31章 準拠：未使用弾の再利用または新規生成）
ShotBase* EnemyBase::GetValidShot(void)
{
	for (size_t i = 0; i < shots_.size(); i++)
	{
		// 未使用（生存していない）弾があれば再利用
		if (!shots_[i]->IsAlive())
		{
			return shots_[i];
		}
	}

	// 新しい弾のインスタンスを生成する
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

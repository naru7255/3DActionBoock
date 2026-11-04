#include "EnemyBase.h"
#include "../Player/Player.h"
#include "../Player/AnimationController.h"
#include "../../Utility/AsoUtility.h"
#include "../Shot/ShotBase.h"
#include "../Shot/ShotStraight.h"

// =============================================================================
// 定数定義（教材第33章・第37章参照）
// =============================================================================
static constexpr COLOR_F COLOR_DIF_DEFAULT = { 0.4f, 0.4f, 0.4f, 1.0f }; // 通常時の拡散光色
static constexpr COLOR_F COLOR_DIF_BLINK   = { 1.0f, 0.2f, 0.2f, 1.0f }; // 被弾点滅時の拡散光色（赤）
static constexpr int TERM_BLINK = 8;                                     // 点滅間隔（フレーム数）
static constexpr int CNT_HIT_REACT = 40;                                 // 被弾怯み時間（フレーム数）
static constexpr int CNT_DEAD_REACT = 80;                                // 死亡演出時間（フレーム数）

// =============================================================================
// コンストラクタ
// =============================================================================
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

// =============================================================================
// デストラクタ
// =============================================================================
EnemyBase::~EnemyBase(void)
{
	Release();
}

// =============================================================================
// 初期化処理（弾エフェクトモデルあり版：教材第31章）
// =============================================================================
void EnemyBase::Init(TYPE type, int baseModelId, int baseAttackEffectModelId, Player* player)
{
	type_ = type;
	player_ = player;
	baseAttackEffectModelId_ = baseAttackEffectModelId;
	shotModelId_ = baseAttackEffectModelId;

	// ベースモデルからインスタンスを複製（メモリ効率化）
	modelId_ = MV1DuplicateModel(baseModelId);

	// 派生クラスごとの固有パラメータ（速度・スケール・HP・当たり判定半径）を設定
	SetParam();

	if (modelId_ != -1)
	{
		MV1SetScale(modelId_, scales_);
		MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
		// エネミー本来の色味を設定（教材第27章）
		MV1SetDifColorScale(modelId_, COLOR_EMI_DEFAULT);
	}

	// ランダムな出現位置を決定
	SetSpawnPosition();

	// アニメーション制御クラスの生成と各モーションの登録
	animController_ = new AnimationController(modelId_);

	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::ATTACK), 30.0f, static_cast<int>(ANIM_TYPE::ATTACK));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::DEATH), 30.0f, static_cast<int>(ANIM_TYPE::DEATH));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::HIT_REACT), 30.0f, static_cast<int>(ANIM_TYPE::HIT_REACT));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 30.0f, static_cast<int>(ANIM_TYPE::IDLE));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::JUMP), 30.0f, static_cast<int>(ANIM_TYPE::JUMP));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::RUN), 30.0f, static_cast<int>(ANIM_TYPE::RUN));
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 30.0f, static_cast<int>(ANIM_TYPE::WALK));

	// 初期アニメーションは歩行モーション（ループ再生）
	animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);

	// 初期ステートを通常待機に設定
	ChangeState(STATE::STANDBY);
}

// 弾なし初期化（Demonなどの近接エネミー用）
void EnemyBase::Init(TYPE type, int baseModelId, Player* player)
{
	Init(type, baseModelId, -1, player);
}

// =============================================================================
// 毎フレーム更新処理
// =============================================================================
void EnemyBase::Update(void)
{
	// 終了（消滅）状態なら更新しない
	if (state_ == STATE::END) return;

	// 行動ステートごとの更新処理呼び出し
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

	// 発射した弾の更新
	UpdateShots();

	// アニメーションの更新
	if (animController_ != nullptr)
	{
		animController_->Update();
	}

	// 3Dモデルに座標と回転角度を反映
	if (modelId_ != -1)
	{
		MV1SetPosition(modelId_, pos_);
		MV1SetRotationXYZ(modelId_, angles_);
	}
}

// =============================================================================
// 描画処理
// =============================================================================
void EnemyBase::Draw(void)
{
	// 終了（消滅）状態なら描画しない
	if (state_ == STATE::END) return;

	// ステートに応じた描画呼び出し
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

	// 弾の描画
	DrawShots();

#ifdef _DEBUG
	// デバッグ用衝突判定球体の描画（当たり判定が有効なSTANDBYまたはATTACK時のみ描画）
	if (IsCollisionState())
	{
		DrawSphere3D(pos_, collisionRadius_, 10, 0x0000ff, 0x0000ff, false);
	}
#endif
}

// =============================================================================
// 解放処理
// =============================================================================
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

// =============================================================================
// 出現座標の設定（ステージ外周からランダムな角度で配置）
// =============================================================================
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

// =============================================================================
// プレイヤーの方向を向く
// =============================================================================
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

// =============================================================================
// 移動処理（プレイヤー方向へ直進）
// =============================================================================
void EnemyBase::Move(void)
{
	VECTOR velocity = VScale(moveDir_, speed_);
	pos_ = VAdd(pos_, velocity);
}

// =============================================================================
// ステート（状態）切り替え
// =============================================================================
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
		break;
	}
}

// 通常待機状態への切り替え
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

// 攻撃状態への切り替え
void EnemyBase::ChangeAttack(void)
{
	if (modelId_ != -1)
	{
		MV1SetDifColorScale(modelId_, COLOR_DIF_DEFAULT);
	}
	if (animController_ != nullptr)
	{
		// 攻撃アニメーションを単発再生
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
	}

	// 弾を所持している敵（Wizard等）は弾を発射（教材第31章）
	if (baseAttackEffectModelId_ != -1)
	{
		Shot();
	}
}

// 被弾リアクション状態への切り替え
void EnemyBase::ChangeHitReact(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::HIT_REACT), true);
	}
	hitTimer_ = CNT_HIT_REACT;
}

// 死亡演出状態への切り替え
void EnemyBase::ChangeDeadReact(void)
{
	// 死亡時は発射済みの弾をすべて消滅・無効化する
	ClearShots();
	if (animController_ != nullptr)
	{
		// 倒れるアニメーションを単発再生
		animController_->Play(static_cast<int>(ANIM_TYPE::DEATH), false);
	}
	deadTimer_ = CNT_DEAD_REACT;
}

// 通常待機状態の更新
void EnemyBase::UpdateStandby(void)
{
	LookPlayer();
	Move();
}

// 攻撃状態の更新
void EnemyBase::UpdateAttack(void)
{
	LookPlayer();

	// 攻撃アニメーション終了で通常待機へ復帰（教材第30章）
	if (animController_ != nullptr && animController_->IsEnd())
	{
		ChangeState(STATE::STANDBY);
	}
}

// 被弾リアクション状態の更新（点滅演出とタイマーカウント）
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

// 死亡演出状態の更新（点滅と消滅遷移）
void EnemyBase::UpdateDeadReact(void)
{
	deadTimer_--;

	// 被弾点滅演出
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

	// アニメーション終了またはタイマー終了でEND状態へ遷移（消滅）
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

// 弾リストの更新
void EnemyBase::ClearShots(void)
{
	for (auto* shot : shots_)
	{
		if (shot != nullptr)
		{
			shot->Destroy();
		}
	}
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

// 弾リストの描画
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

// 弾の発射処理（教材第31章）
void EnemyBase::Shot(void)
{
	if (baseAttackEffectModelId_ == -1) return;

	// 未使用の弾インスタンスを取得して発射
	ShotBase* shot = GetValidShot();
	if (shot != nullptr)
	{
		VECTOR shotPos = pos_;
		shotPos.y += 40.0f; // 腰・胸の高さから発射
		shot->CreateShot(shotPos, moveDir_);
	}
}

// 未使用の弾を取得（オブジェクトプーリング）
ShotBase* EnemyBase::GetValidShot(void)
{
	for (size_t i = 0; i < shots_.size(); i++)
	{
		// 画面外や消滅済みの弾インスタンスを再利用
		if (!shots_[i]->IsAlive())
		{
			return shots_[i];
		}
	}

	// 空きがなければ新しい弾を生成してリストに追加
	ShotBase* shot = new ShotStraight(ShotBase::TYPE::STRAIGHT, baseAttackEffectModelId_);
	shots_.push_back(shot);
	return shot;
}

// 当たり判定が有効な状態かどうか（STANDBY または ATTACK 時のみ有効）
bool EnemyBase::IsCollisionState(void) const
{
	return state_ == STATE::STANDBY || state_ == STATE::ATTACK;
}

// 生存判定
bool EnemyBase::IsAlive(void) const
{
	return hp_ > 0 && state_ != STATE::END;
}

// 被弾ダメージ処理
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

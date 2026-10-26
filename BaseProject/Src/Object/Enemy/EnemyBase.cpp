#include "EnemyBase.h"
#include "../Player/Player.h"
#include "../Player/AnimationController.h"
#include "../../Utility/AsoUtility.h"
#include "../Shot/ShotBase.h"
#include "../Shot/ShotStraight.h"
#include <cmath>
#include <cstdlib>

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
	animController_(nullptr)
	
{
}

EnemyBase::~EnemyBase(void)
{
	Release();
}

void EnemyBase::Init(TYPE type, int baseModelId, Player* player)
{
	type_ = type;
	player_ = player;

	// ① ベースモデルを複製する
	modelId_ = MV1DuplicateModel(baseModelId);

	

	// ★ 派生クラスごとのパラメータ（スピードやスケールなど）を適用する
	SetParam();

	if (modelId_ != -1)
	{
		MV1SetScale(modelId_, scales_);
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

	// 常に WALK アニメーションをループ再生
	animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);

	// ④ 初期状態を STANDBY に設定
	ChangeState(STATE::STANDBY);
}
void EnemyBase::Update(void)
{
	// ★ 状態ごとに更新処理を分岐
	switch (state_)
	{
	case EnemyBase::STATE::STANDBY:
		UpdateStandby();
		break;
	case EnemyBase::STATE::ATTACK:
		UpdateAttack();
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
	// ★ 状態ごとに描画処理を分岐
	switch (state_)
	{
	case EnemyBase::STATE::STANDBY:
		DrawStandby();
		break;
	case EnemyBase::STATE::ATTACK:
		DrawAttack();
		break;
	}
	// 弾の描画処理
	DrawShots();
}

void EnemyBase::Release(void)
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

// ---------------------------------------------------------
// 要件：エネミーの出現位置をランダムにする
// ---------------------------------------------------------
void EnemyBase::SetSpawnPosition(void)
{
	// 1. ステージの中心座標を求める
	VECTOR centerPos = { 1000.0f, 0.0f, 1000.0f }; // ステージに合わせて調整
	float radius = 800.0f; // ステージ中央から端までの半径

	// 2. 乱数を使用し、ランダムな角度（0?360度）を取得する
	float randomDeg = static_cast<float>(rand() % 360);
	float randomRad = AsoUtility::Deg2RadF(randomDeg);

	// 3. 角度から方向（単位ベクトル）に変換する
	VECTOR dir = { sinf(randomRad), 0.0f, cosf(randomRad) };

	// 4. 方向に半径分の長さを乗算し、円の中心から円周までのベクトルを求める
	VECTOR offset = VScale(dir, radius);

	// 5. 円の中心に 4. で求めたベクトルを加算し、ランダムな位置を求める
	pos_ = VAdd(centerPos, offset);
	pos_.y = 50.0f; // 地面の高さに合わせる
}

// ---------------------------------------------------------
// 要件：エネミーの角度を常にプレイヤーに向くようにする
// ---------------------------------------------------------
void EnemyBase::LookPlayer(void)
{
	if (player_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();

	// プレイヤーへのベクトルを計算
	VECTOR toPlayer = VSub(playerPos, pos_);
	toPlayer.y = 0.0f; // 高さの差は無視して水平面で計算

	if (VSquareSize(toPlayer) > 0.0f)
	{
		// 方向ベクトル（単位ベクトル）をメンバ変数 moveDir_ に格納
		moveDir_ = VNorm(toPlayer);

		// 向き（角度）の計算
		angles_.y = atan2f(moveDir_.x, moveDir_.z) + AsoUtility::Deg2RadF(180.0f);
	}
}

// ---------------------------------------------------------
// 要件：エネミーを常にプレイヤー方向に移動させる
// ---------------------------------------------------------
void EnemyBase::Move(void)
{
	// 方向ベクトルとスピードを掛け合わせて移動量を計算
	VECTOR velocity = VScale(moveDir_, speed_);
	pos_ = VAdd(pos_, velocity);

	// ※ステージ外への移動制限が必要な場合はここに記述
}
// 状態を切り替える関数
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
	}
}

void EnemyBase::ChangeStandby(void)
{
	// 待機・移動状態に変わったときのアニメーション（WALKをループ再生）
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
	}
	else if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
	}

	// 攻撃発射までのカウントを設定（例：60フレーム後に発射）
	cntAttack_ = 60;

}

void EnemyBase::ChangeAttack(void)
{
	if (animController_ != nullptr)
	{
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK), false);
	}
	// 攻撃状態に入った瞬間にカウントを初期化する
	cntAttack_ = 60;
}

void EnemyBase::UpdateStandby(void)
{
	// 従来の移動とプレイヤーを向く処理
	LookPlayer();
	Move();
	// プレイヤーとの距離が 500.0f 未満になったら攻撃状態へ！
	if (player_ != nullptr)
	{
		VECTOR toPlayer = VSub(player_->GetPos(), pos_);
		if (VSize(toPlayer) < 500.0f)
		{
			ChangeState(STATE::ATTACK);
		}
	}

}

void EnemyBase::UpdateAttack(void)
{
	LookPlayer();

	if (cntAttack_ > 0)
	{
		cntAttack_--;
		if (cntAttack_ == 0)
		{
			Shot(); // ここで弾が発射される！
		}
	}

	// アニメーションが終了したら STANDBY に戻る
	if (animController_->IsEnd())
	{
		ChangeState(STATE::STANDBY);
	}
}
void EnemyBase::DrawStandby(void)
{
	if (modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}
}

void EnemyBase::DrawAttack(void)
{
	if (modelId_ != -1)
	{
		MV1DrawModel(modelId_);
	}
}

void EnemyBase::UpdateShots(void)
{
	for(auto& shot : shots_)
	{
		if(shot != nullptr)
		{
			shot->Update();
		}
	}
}

void EnemyBase::DrawShots(void)
{
	for(auto& shot : shots_)
	{
		if(shot != nullptr)
		{
			shot->Draw();
		}
	}
}

void EnemyBase::Shot(void)
{
	//  ShotStraight を新規作成する
	ShotStraight* newShot = new ShotStraight(ShotBase::TYPE::STRAIGHT, shotModelId_);
	VECTOR shotPos = pos_;
	shotPos.y += 40.0f; // 高さを調整
	//  弾の発生場所と進行方向を設定して初期化する
	newShot->CreateShot(shotPos, moveDir_);

	//  管理用配列
	shots_.push_back(newShot);
}

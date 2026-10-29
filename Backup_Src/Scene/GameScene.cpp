#include <DxLib.h>
#include "GameScene.h"
#include "../Application.h"
#include "../Object/Grid.h"
#include "../Object/Player/Player.h"
#include "../Object/Camera/Camera.h"
#include "../Manager/SceneManager.h"
#include "../Manager/BlockManager.h" 
#include "../Manager/EnemyManager.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Object/Shot/ShotBase.h"
#include "../Object/Enemy/EnemyDemon.h"
#include "../Manager/HpManager.h"

GameScene::GameScene(void) :
	grid_(nullptr),
	blockManager_(nullptr),
	player_(nullptr),
	enemyManager_(nullptr),
	hpManager_(nullptr),
	imgGameover_(-1)
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{
	// 3D描画の初期化
	grid_ = new Grid();
	grid_->Init();
	// ブロック管理クラスの初期化
	blockManager_ = new BlockManager();
	blockManager_->Init();
	// プレイヤーの初期化
	player_ = new Player();
	player_->Init();
	// HP管理クラスの初期化
	hpManager_ = new HpManager(player_);
	hpManager_->Init();
	// 敵管理クラスの初期化
	enemyManager_ = new EnemyManager(player_);
	enemyManager_->Init();
	// ゲームオーバー画像のロード
	imgGameover_ = LoadGraph("Data/Image/Gameover.png");
	// カメラの追従対象をプレイヤーに設定
	SceneManager::GetInstance().GetCamera()->SetFollow(player_);
}

void GameScene::Update(void)
{
	if (player_ != nullptr && player_->GetState() == Player::STATE::END)
	{
		if (InputManager::GetInstance().IsTrgDown(KEY_INPUT_SPACE))
		{
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
		}
		return;
	}

	if (grid_ != nullptr) { grid_->Update(); }
	if (blockManager_ != nullptr) { blockManager_->Update(); }

	// プレイヤー更新（カメラ角度を渡す）
	if (player_ != nullptr)
	{
		float cameraAngleY = 0.0f;
		auto camera = SceneManager::GetInstance().GetCamera();
		if (camera != nullptr)
		{
			cameraAngleY = camera->GetAngleY();
		}
		player_->Update(cameraAngleY);
	}

	if (hpManager_ != nullptr) { hpManager_->Update(); }
	if (enemyManager_ != nullptr) { enemyManager_->Update(); }

	// 衝突判定（床 -> 壁 -> 敵 の順）
	Collision();
}

void GameScene::Draw(void)
{
	if (grid_ != nullptr) { grid_->Draw(); }
	if (blockManager_ != nullptr) { blockManager_->Draw(); }
	if (player_ != nullptr) { player_->Draw(); }
	if (enemyManager_ != nullptr) { enemyManager_->Draw(); }
	if (hpManager_ != nullptr) { hpManager_->Draw(); }

	if (player_ != nullptr && player_->GetState() == Player::STATE::END && imgGameover_ != -1)
	{
		int imageWidth = 0;
		int imageHeight = 0;
		GetGraphSize(imgGameover_, &imageWidth, &imageHeight);
		DrawGraph((Application::SCREEN_SIZE_X - imageWidth) / 2,
			(Application::SCREEN_SIZE_Y - imageHeight) / 2, imgGameover_, TRUE);
	}
}

void GameScene::Release(void)
{
	if (grid_ != nullptr)
	{
		grid_->Release();
		delete grid_;
		grid_ = nullptr;
	}

	if (blockManager_ != nullptr)
	{
		blockManager_->Release();
		delete blockManager_;
		blockManager_ = nullptr;
	}

	if (player_ != nullptr)
	{
		player_->Release();
		delete player_;
		player_ = nullptr;
	}

	if (hpManager_ != nullptr)
	{
		hpManager_->Release();
		delete hpManager_;
		hpManager_ = nullptr;
	}

	if (imgGameover_ != -1)
	{
		DeleteGraph(imgGameover_);
		imgGameover_ = -1;
	}

	if (enemyManager_ != nullptr)
	{
		enemyManager_->Release();
		delete enemyManager_;
		enemyManager_ = nullptr;
	}
}

void GameScene::Collision(void)
{
	CollisionStage();
	CollisionWall(); // スイープ壁判定
	CollisionEnemy();
	CollisionWeapon();
}

// -----------------------------------------------------------------------------
// 【修正版】床（地面）との接地判定
// -----------------------------------------------------------------------------
void GameScene::CollisionStage(void)
{
	if (player_ == nullptr || blockManager_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();

	// 崖際で自然に足を踏み外して落ちるよう、足元の中心付近（狭いオフセット）でチェック
	float offset = 10.0f;
	VECTOR checkPositions[5] = {
		playerPos,
		VAdd(playerPos, VGet(offset, 0.0f,  0.0f)),
		VAdd(playerPos, VGet(-offset, 0.0f,  0.0f)),
		VAdd(playerPos, VGet(0.0f,    0.0f,  offset)),
		VAdd(playerPos, VGet(0.0f,    0.0f, -offset))
	};

	bool isHit = false;
	float highestY = -99999.0f;

	// 落下中はレイを下に伸ばして、高速落下時の地面すり抜けを防ぐ
	float checkLen = Player::COLLISION_LEN;
	float jumpPow = player_->GetJumpPow();
	if (jumpPow < 0.0f)
	{
		checkLen += fabsf(jumpPow) * 1.5f;
	}

	for (int i = 0; i < 5; i++)
	{
		// 足元より少し上（checkLen * 0.5f）から下向き（checkLen）にレイキャスト
		VECTOR topPos = checkPositions[i];
		topPos.y = playerPos.y + (checkLen * 0.5f);

		VECTOR downPos = checkPositions[i];
		downPos.y = playerPos.y - checkLen;

		if (i == 0)
		{
			lineTopPos_ = topPos;
			lineDownPos_ = downPos;
		}

		MV1_COLL_RESULT_POLY result;
		if (blockManager_->IsCollisionLine(topPos, downPos, &result))
		{
			// 【重要】上向きの面（法線 Normal.y > 0.5f：床面）のみを地面として判定
			// 側面（壁）にレイがかすめても地面扱いせず無視することで、崖際でスルッと落下させる
			if (result.Normal.y > 0.5f)
			{
				if (result.HitPosition.y > highestY)
				{
					highestY = result.HitPosition.y;
					isHit = true;
				}
			}
		}
	}

	// 床面にのみ接地・着地処理を行う
	if (isHit)
	{
		player_->CollisionStage(highestY);
	}
}

// -----------------------------------------------------------------------------
// 【修正版】壁との衝突判定（レイキャストによる壁スライド＆押し戻し）
// -----------------------------------------------------------------------------
void GameScene::CollisionWall(void)
{
	if (player_ == nullptr || blockManager_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();
	VECTOR playerPrePos = player_->GetPrevPos();

	// 水平方向の移動ベクトル
	VECTOR moveVec = VSub(playerPos, playerPrePos);
	moveVec.y = 0.0f;

	// 移動していない場合は判定不要
	float moveDistSq = VSquareSize(moveVec);
	if (moveDistSq <= 0.0001f) return;

	float moveDist = sqrtf(moveDistSq);
	VECTOR moveDir = VScale(moveVec, 1.0f / moveDist);

	const float bodyRadius = 25.0f; // プレイヤーの体感半径（ブロックサイズ100に対して適切な厚み）
	const float checkHeight[2] = { 25.0f, 60.0f }; // 足元（段差検知）と胴体（壁検知）の2段チェック

	for (int h = 0; h < 2; h++)
	{
		VECTOR startPos = playerPrePos;
		startPos.y += checkHeight[h];

		// 移動方向 ＋ 体の半径分先までレイを飛ばして壁との接触を検知
		VECTOR endPos = VAdd(startPos, VScale(moveDir, moveDist + bodyRadius));

		MV1_COLL_RESULT_POLY res;
		if (blockManager_->IsCollisionLine(startPos, endPos, &res))
		{
			// 床面ではなく垂直な面（壁：Normal.y < 0.5f）の場合のみ押し戻し
			if (fabsf(res.Normal.y) < 0.5f)
			{
				VECTOR wallNormal = VGet(res.Normal.x, 0.0f, res.Normal.z);
				if (VSquareSize(wallNormal) > 0.0001f)
				{
					wallNormal = VNorm(wallNormal);

					// 壁の表面位置から半径分外側に離した補正座標
					VECTOR correctPos = VAdd(res.HitPosition, VScale(wallNormal, bodyRadius + 0.5f));

					// 移動ベクトルから壁にめり込む方向の成分を相殺（スライドベクトルを算出）
					float dot = VDot(moveVec, wallNormal);
					if (dot < 0.0f)
					{
						// 壁に垂直な進入成分を打ち消し、壁と平行な方向へ滑らせる
						VECTOR slideVec = VSub(moveVec, VScale(wallNormal, dot));
						playerPos.x = playerPrePos.x + slideVec.x;
						playerPos.z = playerPrePos.z + slideVec.z;
					}
					else
					{
						// すでに食い込んでいる場合は壁表面の外側へ押し出す
						playerPos.x = correctPos.x;
						playerPos.z = correctPos.z;
					}
					player_->SetPos(playerPos);
					break;
				}
			}
		}
	}
}
bool CheckCollisionSphere(VECTOR pos1, float radius1, VECTOR pos2, float radius2)
{
	VECTOR vDiff = VSub(pos2, pos1);
	float distSq = (vDiff.x * vDiff.x) + (vDiff.y * vDiff.y) + (vDiff.z * vDiff.z);
	float radiusSum = radius1 + radius2;
	float radiusSumSq = radiusSum * radiusSum;

	return (distSq <= radiusSumSq);
}

void GameScene::CollisionEnemy(void)
{
	if (player_ == nullptr || enemyManager_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();
	float playerRadius = Player::COLLISION_RADIUS;

	const auto& enemys = enemyManager_->GetEnemys();

	for (EnemyBase* enemy : enemys)
	{
		if (enemy == nullptr) continue;

		if (CheckCollisionSphere(playerPos, playerRadius, enemy->GetPos(), enemy->GetCollisionRadius()))
		{
			if (!player_->IsInvincible())
			{
				VECTOR dir = VSub(playerPos, enemy->GetPos());
				dir.y = 0.0f;
				if (VSquareSize(dir) > 0.0f) { dir = VNorm(dir); }
				else { dir = { 0.0f, 0.0f, 1.0f }; }

				player_->Damage(1);
				player_->KnockBack(dir, 20.0f);
			}
		}

		const auto& shots = enemy->GetShots();
		for (ShotBase* shot : shots)
		{
			if (shot == nullptr || !shot->IsAlive()) continue;

			if (CheckCollisionSphere(playerPos, playerRadius, shot->GetPos(), shot->GetCollisionRadius()))
			{
				if (!player_->IsInvincible())
				{
					VECTOR dir = VSub(playerPos, shot->GetPos());
					dir.y = 0.0f;
					if (VSquareSize(dir) > 0.0f) { dir = VNorm(dir); }
					else { dir = { 0.0f, 0.0f, 1.0f }; }

					player_->Damage(1);
					player_->KnockBack(dir, 20.0f);
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------
// プレイヤーの武器とエネミーの衝突判定
// -----------------------------------------------------------------------------
void GameScene::CollisionWeapon(void)
{
	if (player_ == nullptr || enemyManager_ == nullptr) return;

	// 1. プレイヤーから現在使用中の武器を取得
	WeaponBase* weapon = player_->GetUseWeapon();
	if (weapon == nullptr) return;

	// 2. 武器が生存（使用中）していなければ判定しない
	if (!weapon->IsAlive()) return;

	// 3. エネミーリストを取得して1体ずつ判定
	const auto& enemys = enemyManager_->GetEnemys();
	for (EnemyBase* enemy : enemys)
	{
		if (enemy == nullptr || !enemy->IsAlive()) continue;

		// 4. 武器の球体とエネミーの球体の当たり判定
		if (CheckCollisionSphere(
			weapon->GetPos(), weapon->GetCollisionRadius(),
			enemy->GetPos(), enemy->GetCollisionRadius()))
		{
			// 5. ヒットしたらエネミーにダメージを与える
			enemy->Damage(1);
		}
	}
}
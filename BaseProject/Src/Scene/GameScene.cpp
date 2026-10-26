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
	grid_ = new Grid();
	grid_->Init();

	blockManager_ = new BlockManager();
	blockManager_->Init();

	player_ = new Player();
	player_->Init();

	hpManager_ = new HpManager(player_);
	hpManager_->Init();

	enemyManager_ = new EnemyManager(player_);
	enemyManager_->Init();

	imgGameover_ = LoadGraph("Data/Image/Gameover.png");

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
}

// -----------------------------------------------------------------------------
// 【完全復元】あなたが元々作っていた、正しくジャンプできる地面判定
// -----------------------------------------------------------------------------
void GameScene::CollisionStage(void)
{
	if (player_ == nullptr || blockManager_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();
	float offsetX = 20.0f;
	float offsetZ = 20.0f;

	VECTOR checkPositions[5] = {
		playerPos,
		VAdd(playerPos, VGet(offsetX, 0.0f,  0.0f)),
		VAdd(playerPos, VGet(-offsetX, 0.0f,  0.0f)),
		VAdd(playerPos, VGet(0.0f,    0.0f,  offsetZ)),
		VAdd(playerPos, VGet(0.0f,    0.0f, -offsetZ))
	};

	bool isHit = false;
	float highestY = -99999.0f;

	// あなたの元の設計：落下中だけ判定を伸ばし、ジャンプ中は地面に吸着させない！
	float checkLen = Player::COLLISION_LEN;
	float jumpPow = player_->GetJumpPow();
	if (jumpPow < 0.0f)
	{
		checkLen += fabsf(jumpPow) * 1.5f;
	}

	for (int i = 0; i < 5; i++)
	{
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
			if (result.HitPosition.y > highestY)
			{
				highestY = result.HitPosition.y;
				isHit = true;
			}
		}
	}

	// 衝突した一番高い地面に着地
	if (isHit)
	{
		player_->CollisionStage(highestY);
	}
}

// -----------------------------------------------------------------------------
// 【安全版】壁との衝突判定
// -----------------------------------------------------------------------------
void GameScene::CollisionWall(void)
{
	if (player_ == nullptr || blockManager_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();
	VECTOR playerPrePos = player_->GetPrevPos();

	VECTOR moveVec = VSub(playerPos, playerPrePos);
	moveVec.y = 0.0f;

	if (VSquareSize(moveVec) <= 0.0001f) return;

	// 地面と擦らない高さ（足元 + 10.0f）
	VECTOR startPos = playerPrePos;
	startPos.y += 10.0f;

	VECTOR endPos = playerPos;
	endPos.y += 10.0f;

	MV1_COLL_RESULT_POLY res;
	if (blockManager_->IsCollisionLine(startPos, endPos, &res))
	{
		// 床ではなく「壁（垂直な面）」に当たった時だけ押し戻す
		if (fabsf(res.Normal.y) < 0.5f)
		{
			playerPos.x = playerPrePos.x;
			playerPos.z = playerPrePos.z;
			player_->SetPos(playerPos);
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
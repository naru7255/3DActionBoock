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

// =============================================================================
// コンストラクタ
// =============================================================================
GameScene::GameScene(void) :
	grid_(nullptr),
	blockManager_(nullptr),
	player_(nullptr),
	enemyManager_(nullptr),
	hpManager_(nullptr),
	imgGameover_(-1)
{
}

// =============================================================================
// デストラクタ
// =============================================================================
GameScene::~GameScene(void)
{
}

// =============================================================================
// 初期化処理（各マネージャー・オブジェクトの生成とリソースロード）
// =============================================================================
void GameScene::Init(void)
{
	// 3D空間のグリッド線生成
	grid_ = new Grid();
	grid_->Init();

	// ステージブロック管理クラスの初期化
	blockManager_ = new BlockManager();
	blockManager_->Init();

	// プレイヤーの初期化
	player_ = new Player();
	player_->Init();

	// HP UI管理クラスの初期化（プレイヤーのポインタを渡す）
	hpManager_ = new HpManager(player_);
	hpManager_->Init();

	// エネミー管理クラスの初期化（プレイヤーのポインタを渡す）
	enemyManager_ = new EnemyManager(player_);
	enemyManager_->Init();

	// ゲームオーバー画像の読み込み
	imgGameover_ = LoadGraph("Data/Image/Gameover.png");

	// カメラの追従対象をプレイヤーに設定
	SceneManager::GetInstance().GetCamera()->SetFollow(player_);
}

// =============================================================================
// 毎フレーム更新処理
// =============================================================================
void GameScene::Update(void)
{
	// ゲームオーバー状態（Player::STATE::END）の時：スペースキーでタイトルへ遷移
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

	// プレイヤーの更新（カメラの水平角度Yを渡して移動向きを連動）
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

	// 総合衝突判定の実行（地面・壁・敵・武器）
	Collision();
}

// =============================================================================
// 描画処理
// =============================================================================
void GameScene::Draw(void)
{
	if (grid_ != nullptr) { grid_->Draw(); }
	if (blockManager_ != nullptr) { blockManager_->Draw(); }
	if (player_ != nullptr) { player_->Draw(); }
	if (enemyManager_ != nullptr) { enemyManager_->Draw(); }
	if (hpManager_ != nullptr) { hpManager_->Draw(); }

	// ゲームオーバー時：画面中央に「GAME OVER」画像を表示（教材第35章）
	if (player_ != nullptr && player_->GetState() == Player::STATE::END && imgGameover_ != -1)
	{
		int imageWidth = 0;
		int imageHeight = 0;
		GetGraphSize(imgGameover_, &imageWidth, &imageHeight);
		DrawGraph(
			(Application::SCREEN_SIZE_X - imageWidth) / 2,
			(Application::SCREEN_SIZE_Y - imageHeight) / 2,
			imgGameover_,
			TRUE
		);
	}
}

// =============================================================================
// 解放処理（メモリ解放と画像ハンドルの破棄）
// =============================================================================
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

// =============================================================================
// 総合当たり判定
// =============================================================================
void GameScene::Collision(void)
{
	CollisionStage();   // 地面との接地判定
	CollisionWall();    // 壁との衝突・スライド押し戻し判定
	CollisionEnemy();   // プレイヤー対エネミー本体・弾の判定
	CollisionWeapon();  // プレイヤー武器対エネミー本体の判定
}

// =============================================================================
// 地面との接地判定（教材第18章の発展形：5点マルチレイキャスト）
// 足元5点から下方へレイを飛ばし、水平面（法線Y > 0.5f）のみに接地させる
// =============================================================================
void GameScene::CollisionStage(void)
{
	if (player_ == nullptr || blockManager_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();

	// 足元中心＋前後左右の5点でチェック（崖際での自然な踏み外し落下を実現）
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

	// 高速落下時に地面をすり抜けないよう、落下速度に応じてレイを下方へ伸長
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
			// 【重要】水平な面（法線 Normal.y > 0.5f：床面）のみを地面として判定
			// 垂直な面（壁）に当たっても接地とみなさないことで、崖際で自然に滑落する
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

	// 最も高い床面に接地
	if (isHit)
	{
		player_->CollisionStage(highestY);
	}
}

// =============================================================================
// 壁との衝突・スライド判定（レイキャストによる法線反射）
// ブロックの側面にぶつかった際、壁に沿って滑らかに移動できるようにする
// =============================================================================
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

	const float bodyRadius = 25.0f;                // プレイヤーの体感衝突半径
	const float checkHeight[2] = { 25.0f, 60.0f }; // 足元と胴体の2段階でチェック

	for (int h = 0; h < 2; h++)
	{
		VECTOR startPos = playerPrePos;
		startPos.y += checkHeight[h];

		// 移動先＋体の半径分までレイを伸ばして壁との接触を検知
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

					// 壁の表面位置から半径分外側に押し出す補正座標
					VECTOR correctPos = VAdd(res.HitPosition, VScale(wallNormal, bodyRadius + 0.5f));

					// 移動ベクトルから壁にめり込む成分を相殺（スライドベクトル算出）
					float dot = VDot(moveVec, wallNormal);
					if (dot < 0.0f)
					{
						// 壁に沿って滑らせる
						VECTOR slideVec = VSub(moveVec, VScale(wallNormal, dot));
						playerPos.x = playerPrePos.x + slideVec.x;
						playerPos.z = playerPrePos.z + slideVec.z;
					}
					else
					{
						// すでに食い込んでいる場合は壁表面の外へ押し戻す
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

// =============================================================================
// 球体同士の衝突判定ヘルパー関数（距離の2乗と半径和の2乗を比較して高速化）
// =============================================================================
bool CheckCollisionSphere(VECTOR pos1, float radius1, VECTOR pos2, float radius2)
{
	VECTOR vDiff = VSub(pos2, pos1);
	float distSq = (vDiff.x * vDiff.x) + (vDiff.y * vDiff.y) + (vDiff.z * vDiff.z);
	float radiusSum = radius1 + radius2;
	float radiusSumSq = radiusSum * radiusSum;

	return (distSq <= radiusSumSq);
}

// =============================================================================
// プレイヤー対エネミー本体・弾の衝突判定（教材第32章）
// =============================================================================
void GameScene::CollisionEnemy(void)
{
	if (player_ == nullptr || enemyManager_ == nullptr) return;

	VECTOR playerPos = player_->GetPos();
	float playerRadius = Player::COLLISION_RADIUS;

	const auto& enemys = enemyManager_->GetEnemys();

	for (EnemyBase* enemy : enemys)
	{
		if (enemy == nullptr) continue;

		// ★ エネミー本体の当たり判定：有効な状態（STANDBY または ATTACK）の時のみ判定
		// 死亡演出中（DEAD_REACT）や消滅後（END）はプレイヤーにダメージを与えない（教材第37章要件）
		if (enemy->IsCollisionState())
		{
			if (CheckCollisionSphere(playerPos, playerRadius, enemy->GetPos(), enemy->GetCollisionRadius()))
			{
				if (!player_->IsInvincible())
				{
					// プレイヤーが敵の反対方向へノックバック
					VECTOR dir = VSub(playerPos, enemy->GetPos());
					dir.y = 0.0f;
					if (VSquareSize(dir) > 0.0f) { dir = VNorm(dir); }
					else { dir = { 0.0f, 0.0f, 1.0f }; }

					player_->Damage(1);
					player_->KnockBack(dir, 20.0f);
				}
			}
		}

		// エネミーの弾とプレイヤーの衝突判定（生存中の弾のみ判定）
		const auto& shots = enemy->GetShots();
		for (ShotBase* shot : shots)
		{
			if (shot == nullptr || !shot->IsAlive()) continue;

			if (CheckCollisionSphere(playerPos, playerRadius, shot->GetPos(), shot->GetCollisionRadius()))
			{
				if (!player_->IsInvincible())
				{
					// 弾の進行方向（または弾から受ける向き）へノックバック
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

// =============================================================================
// プレイヤーの武器とエネミーの衝突判定（教材第37章）
// =============================================================================
void GameScene::CollisionWeapon(void)
{
	if (player_ == nullptr || enemyManager_ == nullptr) return;

	// 1. プレイヤーから現在使用中の武器を取得
	WeaponBase* weapon = player_->GetUseWeapon();
	if (weapon == nullptr || !weapon->IsAlive()) return;

	// 2. エネミーリストを取得して1体ずつ判定
	const auto& enemys = enemyManager_->GetEnemys();
	for (EnemyBase* enemy : enemys)
	{
		// ★ エネミーが当たり判定有効（STANDBY または ATTACK）な場合のみ武器と衝突
		if (enemy == nullptr || !enemy->IsCollisionState()) continue;

		// 3. 武器の球体とエネミーの球体の当たり判定
		if (CheckCollisionSphere(
			weapon->GetPos(), weapon->GetCollisionRadius(),
			enemy->GetPos(), enemy->GetCollisionRadius()))
		{
			// 4. ヒットしたエネミーにダメージを与える
			enemy->Damage(1);
		}
	}
}

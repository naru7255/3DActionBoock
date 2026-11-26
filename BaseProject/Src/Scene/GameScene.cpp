#include <DxLib.h>
#include <cmath>
#include <cstdio>
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
	imgBg_(-1),
	imgBgBoss_(-1),
	bossBgAlpha_(0),
	imgGameover_(-1),
	imgGameClear_(-1),
	imgHealHeart_(-1),
	elapsedFrames_(0),
	bossHpDisplay_(static_cast<float>(BOSS_MAX_HP))
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

	// 背景画像の読み込み（教材第42章追加実装）
	imgBg_ = LoadGraph((Application::PATH_IMAGE + "GameBackground.png").c_str());
	imgBgBoss_ = LoadGraph((Application::PATH_IMAGE + "GameBackgroundBoss.png").c_str());
	bossBgAlpha_ = 0;

	// ゲームオーバー画像の読み込み（教材第35章）
	imgGameover_ = LoadGraph("Data/Image/Gameover.png");

	// ゲームクリア画像の読み込み（教材第41章）
	imgGameClear_ = LoadGraph("Data/Image/GameClear.png");

	// 回復アイテム画像の読み込み
	imgHealHeart_ = LoadGraph((Application::PATH_IMAGE + "Heart.png").c_str());
	healItems_.clear();
	elapsedFrames_ = 0;
	bossHpDisplay_ = static_cast<float>(BOSS_MAX_HP);

	// カメラの追従対象をプレイヤーに設定
	SceneManager::GetInstance().GetCamera()->SetFollow(player_);
}

// =============================================================================
// 毎フレーム更新処理
// =============================================================================
void GameScene::Update(void)
{
	// -------------------------------------------------------------------------
	// 1. ゲームオーバー判定（Player::STATE::END：教材第35章）
	// -------------------------------------------------------------------------
	if (player_ != nullptr && player_->GetState() == Player::STATE::END)
	{
		// スペースキー押下でタイトル画面へ遷移
		if (InputManager::GetInstance().IsTrgDown(KEY_INPUT_SPACE))
		{
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
		}
		return;
	}

	// -------------------------------------------------------------------------
	// 2. ゲームクリア判定（全ウェーブ全滅達成：教材第41章）
	// -------------------------------------------------------------------------
	if (enemyManager_ != nullptr && enemyManager_->GetWave() == EnemyManager::WAVE::END)
	{
		// プレイヤーが死亡中（DEADまたはEND）の場合はクリア処理を行わない
		if (player_ != nullptr && (player_->GetState() == Player::STATE::DEAD || player_->GetState() == Player::STATE::END))
		{
			return;
		}
		// プレイヤーを勝利演出（VICTORY）ステートへ遷移させる
		if (player_ != nullptr && player_->GetState() != Player::STATE::VICTORY)
		{
			player_->Victory();
		}

		// クリア判定中は Player のみ更新（手を振るアニメーション再生）し、他は停止させる
		if (player_ != nullptr)
		{
			player_->Update();
		}

		// スペースキー押下でタイトル画面へ遷移
		if (InputManager::GetInstance().IsTrgDown(KEY_INPUT_SPACE))
		{
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
		}
		return; // 他のオブジェクトの更新や衝突判定は行わない
	}

	// -------------------------------------------------------------------------
	// 3. 通常ゲームプレイ更新
	// -------------------------------------------------------------------------
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

	// プレイ中のみクリアタイムを計測
	if (player_ != nullptr && player_->GetState() != Player::STATE::DEAD && player_->GetState() != Player::STATE::END && player_->GetState() != Player::STATE::VICTORY)
	{
		elapsedFrames_++;
	}

	if (hpManager_ != nullptr) { hpManager_->Update(); }
	UpdateHealItems();
	if (enemyManager_ != nullptr)
	{
		enemyManager_->Update();

		// ボス戦突入時：昼背景から夕焼け背景へ滑らかにクロスフェード（約2秒）
		auto wave = enemyManager_->GetWave();
		if ((wave == EnemyManager::WAVE::BOSS || wave == EnemyManager::WAVE::END) && bossBgAlpha_ < 255)
		{
			bossBgAlpha_ += BOSS_BG_FADE_SPEED;
			if (bossBgAlpha_ > 255) { bossBgAlpha_ = 255; }
		}
	}

	// 総合衝突判定の実行（地面・壁・敵・武器）
	Collision();
}

// =============================================================================
// 描画処理
// =============================================================================
void GameScene::Draw(void)
{
	// 背景画像の描画（3Dオブジェクトより先に画面全体へ描画：教材第42章追加実装）
	if (imgBg_ != -1)
	{
		DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, imgBg_, FALSE);
	}
	// ボス戦用の夕焼け背景を透過ブレンドで重ねて描画
	if (imgBgBoss_ != -1 && bossBgAlpha_ > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, bossBgAlpha_);
		DrawExtendGraph(0, 0, Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, imgBgBoss_, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	if (grid_ != nullptr) { grid_->Draw(); }
	if (blockManager_ != nullptr) { blockManager_->Draw(); }
	if (player_ != nullptr) { player_->Draw(); }
	if (enemyManager_ != nullptr) { enemyManager_->Draw(); }
	DrawHealItems();
	if (hpManager_ != nullptr) { hpManager_->Draw(); }
	DrawStatusHud();
	DrawBossHpBar();
	DrawControlGuide();

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

	// ゲームクリア時：画面中央に「GAME CLEAR」画像を表示（教材第41章）
	// ※ゲームオーバーとゲームクリアが同時に表示されないよう排他にする
	else if (enemyManager_ != nullptr && enemyManager_->GetWave() == EnemyManager::WAVE::END && imgGameClear_ != -1)
	{
		int imageWidth = 0;
		int imageHeight = 0;
		GetGraphSize(imgGameClear_, &imageWidth, &imageHeight);
		int clearY = (Application::SCREEN_SIZE_Y - imageHeight) / 2 - 70;
		DrawGraph(
			(Application::SCREEN_SIZE_X - imageWidth) / 2,
			clearY,
			imgGameClear_,
			TRUE
		);
		DrawClearResult(clearY + imageHeight + 20);
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

	if (imgBg_ != -1)
	{
		DeleteGraph(imgBg_);
		imgBg_ = -1;
	}

	if (imgBgBoss_ != -1)
	{
		DeleteGraph(imgBgBoss_);
		imgBgBoss_ = -1;
	}

	if (imgGameover_ != -1)
	{
		DeleteGraph(imgGameover_);
		imgGameover_ = -1;
	}

	if (imgGameClear_ != -1)
	{
		DeleteGraph(imgGameClear_);
		imgGameClear_ = -1;
	}

	if (imgHealHeart_ != -1)
	{
		DeleteGraph(imgHealHeart_);
		imgHealHeart_ = -1;
	}
	healItems_.clear();

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

		// エネミー本体の当たり判定：有効な状態（STANDBY または ATTACK）の時のみ判定
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
		// エネミーが当たり判定有効（STANDBY または ATTACK）な場合のみ武器と衝突
		if (enemy == nullptr || !enemy->IsCollisionState()) continue;

		// 3. 武器の球体とエネミーの球体の当たり判定
		if (CheckCollisionSphere(
			weapon->GetPos(), weapon->GetCollisionRadius(),
			enemy->GetPos(), enemy->GetCollisionRadius()))
		{
			// 4. ヒットしたエネミーにダメージを与える
			enemy->Damage(1);

			// 撃破した瞬間に一定確率（35%）で回復ハートをドロップ
			if (!enemy->IsCollisionState() && GetRand(99) < HEAL_DROP_RATE)
			{
				SpawnHealItem(enemy->GetPos());
			}
		}
	}
}


// =============================================================================
// 回復アイテムの生成（敵撃破時にその場へドロップ）
// =============================================================================
void GameScene::SpawnHealItem(const VECTOR& pos)
{
	HealItem item;
	item.pos = pos;
	item.pos.y += 45.0f;
	item.lifeTimer = HEAL_ITEM_LIFE;
	item.animPhase = 0.0f;
	item.isAlive = true;
	healItems_.push_back(item);
}

// =============================================================================
// 回復アイテムの更新（浮遊アニメーション・寿命管理・プレイヤー取得判定）
// =============================================================================
void GameScene::UpdateHealItems(void)
{
	if (player_ == nullptr) return;
	VECTOR playerPos = player_->GetPos();

	for (auto& item : healItems_)
	{
		if (!item.isAlive) continue;

		item.animPhase += 0.12f;
		item.lifeTimer--;
		if (item.lifeTimer <= 0)
		{
			item.isAlive = false;
			continue;
		}

		// プレイヤーとの距離判定（近づくとHP回復）
		VECTOR diff = VSub(playerPos, item.pos);
		diff.y = 0.0f;
		float distSq = (diff.x * diff.x) + (diff.z * diff.z);
		const float pickupRadius = HEAL_PICKUP_RADIUS;
		if (distSq <= pickupRadius * pickupRadius && player_->GetHp() < Player::MAX_HP)
		{
			player_->Heal(1);
			item.isAlive = false;
		}
	}
}

// =============================================================================
// 回復アイテムの3Dビルボード描画
// =============================================================================
void GameScene::DrawHealItems(void)
{
	if (imgHealHeart_ == -1) return;

	for (const auto& item : healItems_)
	{
		if (!item.isAlive) continue;

		// 消滅直前（残り2秒以下）は点滅させる
		if (item.lifeTimer < HEAL_ITEM_BLINK_TIME && (item.lifeTimer / 6) % 2 == 0) continue;

		VECTOR drawPos = item.pos;
		drawPos.y += sinf(item.animPhase) * 10.0f; // ふわふわ上下浮遊
		float scale = 55.0f + sinf(item.animPhase * 2.0f) * 5.0f;

		DrawBillboard3D(drawPos, 0.5f, 0.5f, scale, 0.0f, imgHealHeart_, TRUE);
	}
}

// =============================================================================
// 画面右上のステータスHUD描画（現在ウェーブ・経過タイム・ダッシュ状態）
// =============================================================================
void GameScene::DrawStatusHud(void)
{
	const int panelX = Application::SCREEN_SIZE_X - 245;
	const int panelY = 18;
	const int panelW = 225;
	const int panelH = 86;

	// 半透明の背景パネルと枠線
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 155);
	DrawRoundRect(panelX, panelY, panelX + panelW, panelY + panelH, 10, 10, 0x111122, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawRoundRect(panelX, panelY, panelX + panelW, panelY + panelH, 10, 10, 0xddaa44, FALSE);

	// 1. 現在のウェーブ表示
	const char* waveText = "WAVE 1 / 3";
	unsigned int waveColor = 0xffffff;
	if (enemyManager_ != nullptr)
	{
		switch (enemyManager_->GetWave())
		{
		case EnemyManager::WAVE::WAVE01: waveText = "WAVE 1 / 3"; waveColor = 0xffffff; break;
		case EnemyManager::WAVE::WAVE02: waveText = "WAVE 2 / 3"; waveColor = 0x88eeff; break;
		case EnemyManager::WAVE::WAVE03: waveText = "WAVE 3 / 3"; waveColor = 0xffee66; break;
		case EnemyManager::WAVE::BOSS:   waveText = "BOSS WAVE!!"; waveColor = 0xff6644; break;
		case EnemyManager::WAVE::END:    waveText = "STAGE CLEAR"; waveColor = 0x66ff88; break;
		}
	}
	DrawString(panelX + 16, panelY + 12, waveText, waveColor);

	// 2. 経過タイム表示（MM:SS.d）
	char rawTime[32];
	FormatElapsedTime(rawTime, sizeof(rawTime));
	char timeBuf[64];
	std::snprintf(timeBuf, sizeof(timeBuf), "TIME  %s", rawTime);
	DrawString(panelX + 16, panelY + 36, timeBuf, 0xffffff);

	// 3. 回避ダッシュのクールタイム状態表示
	if (player_ != nullptr)
	{
		if (player_->GetDashCooldown() <= 0)
		{
			DrawString(panelX + 16, panelY + 60, "[SHIFT] DASH READY", 0x66ffff);
		}
		else
		{
			DrawString(panelX + 16, panelY + 60, "[SHIFT] RECHARGING...", 0xaaaaaa);
		}
	}
}

// =============================================================================
// ゲームクリア時のリザルトパネル描画（クリアタイム・残りHP・総合ランク）
// =============================================================================
void GameScene::DrawClearResult(int baseY)
{
	const int panelW = 380;
	const int panelH = 165;
	const int panelX = (Application::SCREEN_SIZE_X - panelW) / 2;
	const int panelY = baseY;

	// 半透明のリザルト背景パネル
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 185);
	DrawRoundRect(panelX, panelY, panelX + panelW, panelY + panelH, 12, 12, 0x101424, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawRoundRect(panelX, panelY, panelX + panelW, panelY + panelH, 12, 12, 0xffcc44, FALSE);

	// タイムと残りHPから総合ランク（S / A / B）を判定
	float totalSec = static_cast<float>(elapsedFrames_) / 60.0f;
	char rawTime[32];
	FormatElapsedTime(rawTime, sizeof(rawTime));
	int remainHp = (player_ != nullptr) ? player_->GetHp() : 0;

	const char* rankStr = "B";
	unsigned int rankColor = 0x66ff88;
	if (remainHp >= RANK_S_HP && totalSec <= RANK_S_TIME)
	{
		rankStr = "S";
		rankColor = 0xffdd33;
	}
	else if (remainHp >= RANK_A_HP && totalSec <= RANK_A_TIME)
	{
		rankStr = "A";
		rankColor = 0x44ddff;
	}

	char buf[64];
	std::snprintf(buf, sizeof(buf), "CLEAR TIME  :  %s", rawTime);
	DrawString(panelX + 40, panelY + 24, buf, 0xffffff);

	std::snprintf(buf, sizeof(buf), "REMAIN HP   :  %d / %d", remainHp, Player::MAX_HP);
	DrawString(panelX + 40, panelY + 52, buf, 0xffffff);

	std::snprintf(buf, sizeof(buf), "TOTAL RANK  :  %s", rankStr);
	DrawString(panelX + 40, panelY + 86, buf, rankColor);

	DrawString(panelX + 82, panelY + 130, "PRESS [SPACE] TO TITLE", 0xcccccc);
}

// =============================================================================
// ボス専用HPバー描画（ボス戦中のみ画面下部中央に表示）
// =============================================================================
void GameScene::DrawBossHpBar(void)
{
	if (enemyManager_ == nullptr || enemyManager_->GetWave() != EnemyManager::WAVE::BOSS) return;

	// 現在生存しているボス（EnemyBase::TYPE::GIANT）を検索
	EnemyBase* boss = nullptr;
	const auto& enemys = enemyManager_->GetEnemys();
	for (EnemyBase* enemy : enemys)
	{
		if (enemy != nullptr && enemy->GetType() == EnemyBase::TYPE::GIANT && enemy->IsAlive())
		{
			boss = enemy;
			break;
		}
	}
	if (boss == nullptr) return;

	const int maxHp = BOSS_MAX_HP;
	int hp = boss->GetHp();
	if (hp < 0) hp = 0;

	// 追従ダメージゲージ（黄色）を現在HPへ滑らかに近づける
	if (bossHpDisplay_ > static_cast<float>(hp))
	{
		bossHpDisplay_ -= 0.08f;
		if (bossHpDisplay_ < static_cast<float>(hp))
		{
			bossHpDisplay_ = static_cast<float>(hp);
		}
	}
	else
	{
		bossHpDisplay_ = static_cast<float>(hp);
	}

	const int barW = 520;
	const int barH = 22;
	const int barX = (Application::SCREEN_SIZE_X - barW) / 2;
	const int barY = Application::SCREEN_SIZE_Y - 76;

	// 背景プレート
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 175);
	DrawRoundRect(barX - 16, barY - 28, barX + barW + 16, barY + barH + 12, 8, 8, 0x110c14, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawRoundRect(barX - 16, barY - 28, barX + barW + 16, barY + barH + 12, 8, 8, 0xff6644, FALSE);

	// ボス名と残りHPテキスト
	DrawString(barX, barY - 22, "BOSS : GIANT GOLEM", 0xffaa66);
	char hpBuf[32];
	std::snprintf(hpBuf, sizeof(hpBuf), "%d / %d", hp, maxHp);
	DrawString(barX + barW - 56, barY - 22, hpBuf, 0xffffff);

	// ゲージ下地（暗い赤黒）
	DrawBox(barX, barY, barX + barW, barY + barH, 0x331111, TRUE);

	// 追従ダメージゲージ（黄色）
	int delayW = static_cast<int>(barW * (bossHpDisplay_ / static_cast<float>(maxHp)));
	if (delayW > 0)
	{
		DrawBox(barX, barY, barX + delayW, barY + barH, 0xffcc33, TRUE);
	}

	// 現在の残りHPゲージ（赤色：割合 = hp / maxHp）
	int currentW = static_cast<int>(barW * (static_cast<float>(hp) / static_cast<float>(maxHp)));
	if (currentW > 0)
	{
		DrawBox(barX, barY, barX + currentW, barY + barH, 0xee2233, TRUE);
	}

	// ゲージ外枠
	DrawBox(barX, barY, barX + barW, barY + barH, 0xffffff, FALSE);
}

// =============================================================================
// 画面左下のミニ操作ガイド表示
// =============================================================================
void GameScene::DrawControlGuide(void)
{
	const int guideX = 16;
	const int guideY = Application::SCREEN_SIZE_Y - 32;
	const int guideW = 585;
	const int guideH = 24;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 135);
	DrawRoundRect(guideX, guideY, guideX + guideW, guideY + guideH, 6, 6, 0x101420, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	DrawString(
		guideX + 12,
		guideY + 4,
		"[WASD] MOVE   [SPACE] JUMP   [B/J] ATTACK   [C] WEAPON   [SHIFT] DASH",
		0xddeeff
	);
}

// =============================================================================
// プレイヤー被弾・ノックバック共通処理（敵本体・弾の衝突から共通呼び出し）
// =============================================================================
void GameScene::ApplyPlayerDamage(const VECTOR& attackerPos)
{
	if (player_ == nullptr || player_->IsInvincible()) return;

	VECTOR dir = VSub(player_->GetPos(), attackerPos);
	dir.y = 0.0f;
	if (VSquareSize(dir) > 0.0f)
	{
		dir = VNorm(dir);
	}
	else
	{
		dir = { 0.0f, 0.0f, 1.0f };
	}

	player_->Damage(1);
	player_->KnockBack(dir, PLAYER_KNOCKBACK_JUMP);
}

// =============================================================================
// 経過フレーム数を "MM:SS.d" 形式の文字列へ整形する共通ヘルパー
// =============================================================================
void GameScene::FormatElapsedTime(char* outBuf, size_t bufSize) const
{
	float totalSec = static_cast<float>(elapsedFrames_) / 60.0f;
	int mins = static_cast<int>(totalSec) / 60;
	int secs = static_cast<int>(totalSec) % 60;
	int tenths = static_cast<int>(totalSec * 10.0f) % 10;
	std::snprintf(outBuf, bufSize, "%02d:%02d.%d", mins, secs, tenths);
}
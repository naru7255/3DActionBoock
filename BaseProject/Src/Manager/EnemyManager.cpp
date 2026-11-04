#include "EnemyManager.h"
#include "../Object/Enemy/EnemyDemon.h" 
#include "../Object/Enemy/EnemyWizard.h" 
#include "../Object/Player/Player.h"
#include "../Application.h"

// =============================================================================
// コンストラクタ
// =============================================================================
EnemyManager::EnemyManager(Player* player) :
	player_(player),
	wave_(WAVE::WAVE01),
	cntWave_(0),
	cntSpawn_(0)
{
}

// =============================================================================
// デストラクタ
// =============================================================================
EnemyManager::~EnemyManager(void)
{
	Release();
}

// =============================================================================
// 初期化処理（各種エネミーと弾の3Dモデルをベースとしてロード）
// =============================================================================
void EnemyManager::Init(void)
{
	Release();

	// 1. 各種エネミーの基本モデルを事前にメモリへロード
	enemyModelIds_.clear();
	enemyModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Enemy/Demon.mv1").c_str())
	);
	enemyModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Enemy/Wizard.mv1").c_str())
	);
	enemyModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Enemy/Giant.mv1").c_str())
	);

	// 2. 攻撃エフェクト用3Dモデルのロード（教材第31章）
	attackEffectModelIds_.clear();
	attackEffectModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Effect/Fireball/Fireball.mv1").c_str())
	);
	attackEffectModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Effect/Rockfall/Rock.mv1").c_str())
	);

	// 第1ウェーブからスタート
	ChangeWave(WAVE::WAVE01);
}

// =============================================================================
// 毎フレーム更新処理（ウェーブ進行と全エネミーの更新）
// =============================================================================
void EnemyManager::Update(void)
{
	// 1. 現在のウェーブに応じた更新処理（ステートパターン）
	switch (wave_)
	{
	case WAVE::WAVE01:
		UpdateWave01();
		break;
	case WAVE::WAVE02:
		UpdateWave02();
		break;
	case WAVE::WAVE03:
		UpdateWave03();
		break;
	case WAVE::END:
		UpdateWaveEnd();
		break;
	}

	// 2. 生成されている全エネミーの更新処理
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Update();
		}
	}
}

// =============================================================================
// 描画処理
// =============================================================================
void EnemyManager::Draw(void)
{
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Draw();
		}
	}

#ifdef _DEBUG
	// デバッグ表示：現在のウェーブ名と経過フレーム数を画面上に表示
	const char* waveNames[] = { "WAVE 01", "WAVE 02", "WAVE 03", "END (ALL CLEARED)" };
	DrawFormatString(10, 80, 0xffff00, "Enemy Wave: %s (Time: %d)", waveNames[static_cast<int>(wave_)], cntWave_);
#endif
}

// =============================================================================
// 解放処理
// =============================================================================
void EnemyManager::Release(void)
{
	// 全エネミーインスタンスの解放
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Release();
			delete enemy;
		}
	}
	enemys_.clear();

	// エネミーの元モデルハンドルの解放
	for (int id : enemyModelIds_)
	{
		if (id != -1)
		{
			MV1DeleteModel(id);
		}
	}
	enemyModelIds_.clear();

	// 攻撃エフェクト用モデルハンドルの解放
	for (int id : attackEffectModelIds_)
	{
		if (id != -1)
		{
			MV1DeleteModel(id);
		}
	}
	attackEffectModelIds_.clear();
}

// =============================================================================
// ウェーブ切り替え処理
// =============================================================================
void EnemyManager::ChangeWave(WAVE wave)
{
	wave_ = wave;
	cntWave_ = 0;   // ウェーブ経過タイマーリセット
	cntSpawn_ = 0;  // 敵出現タイマーリセット

	// ウェーブ終了（全滅・クリア時）は全エネミーの残留弾をすべて消去する
	if (wave_ == WAVE::END)
	{
		for (auto* enemy : enemys_)
		{
			if (enemy != nullptr)
			{
				enemy->ClearShots();
			}
		}
	}
}

// =============================================================================
// 第1ウェーブ：EnemyDemonを180フレーム間隔で生成。500フレーム経過で第2ウェーブへ
// =============================================================================
void EnemyManager::UpdateWave01(void)
{
	cntWave_++;
	cntSpawn_++;

	// 180フレーム間隔でDemonを生成（500フレームの間に合計2体出現）
	if (cntSpawn_ >= 180)
	{
		cntSpawn_ = 0;
		CreateEnemyDemon();
	}

	// 500フレームを超えたら第2ウェーブへ進む
	if (cntWave_ > 500)
	{
		ChangeWave(WAVE::WAVE02);
	}
}

// =============================================================================
// 第2ウェーブ：EnemyWizardを240フレーム間隔で生成。500フレーム経過で第3ウェーブへ
// =============================================================================
void EnemyManager::UpdateWave02(void)
{
	cntWave_++;
	cntSpawn_++;

	// 240フレーム間隔でWizardを生成（500フレームの間に合計2体出現）
	if (cntSpawn_ >= 240)
	{
		cntSpawn_ = 0;
		CreateEnemyWizard();
	}

	// 500フレームを超えたら第3ウェーブ（残敵掃討）へ進む
	if (cntWave_ > 500)
	{
		ChangeWave(WAVE::WAVE03);
	}
}

// =============================================================================
// 第3ウェーブ：全エネミーが撃破・消滅（END）したら最終ウェーブ（END）へ移行
// 最後の敵の死亡アニメーション（DEAD_REACT）が終了するまでしっかり待機する
// =============================================================================
void EnemyManager::UpdateWave03(void)
{
	cntWave_++;

	// まだ消滅（END）していない敵の数をカウント
	int remainingCount = 0;
	for (auto* enemy : enemys_)
	{
		// 死亡モーション再生中（DEAD_REACT）もカウントし、完全に消滅するまで待つ
		if (enemy != nullptr && enemy->GetState() != EnemyBase::STATE::END)
		{
			remainingCount++;
		}
	}

	// 全エネミーが完全に倒れて消滅したら、ゲームクリア（END）へ移行
	if (remainingCount == 0)
	{
		ChangeWave(WAVE::END);
	}
}

// =============================================================================
// 終了ウェーブ：全敵撃破完了
// 次の第41章でGameClear画面の表示やタイトル遷移へ連動する
// =============================================================================
void EnemyManager::UpdateWaveEnd(void)
{
}

// =============================================================================
// デーモン生成ヘルパー関数
// =============================================================================
void EnemyManager::CreateEnemyDemon(void)
{
	EnemyDemon* demon = new EnemyDemon();
	demon->Init(
		EnemyBase::TYPE::DEMON,
		enemyModelIds_[static_cast<int>(EnemyBase::TYPE::DEMON)],
		-1,
		player_
	);
	enemys_.push_back(demon);
}

// =============================================================================
// ウィザード生成ヘルパー関数
// =============================================================================
void EnemyManager::CreateEnemyWizard(void)
{
	EnemyWizard* wizard = new EnemyWizard();
	wizard->Init(
		EnemyBase::TYPE::WIZARD,
		enemyModelIds_[static_cast<int>(EnemyBase::TYPE::WIZARD)],
		attackEffectModelIds_[0], // ファイアボールのエフェクトモデルID
		player_
	);
	enemys_.push_back(wizard);
}

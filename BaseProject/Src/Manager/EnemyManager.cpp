#include "EnemyManager.h"
#include "../Object/Enemy/EnemyDemon.h" 
#include "../Object/Enemy/EnemyWizard.h" 
#include "../Object/Player/Player.h"
#include "../Application.h"

// コンストラクタ
EnemyManager::EnemyManager(Player* player) :
	player_(player),
	wave_(WAVE::WAVE01),
	cntWave_(0),
	cntSpawn_(0)
{
}

// デストラクタ
EnemyManager::~EnemyManager(void)
{
	Release();
}

// 初期化処理
void EnemyManager::Init(void)
{
	Release();

	// 1. エネミーモデルをベースとして事前にロード
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

	// 2. 攻撃エフェクト用モデルのロード（教材 31章 準拠）
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

// 毎フレームの更新処理
void EnemyManager::Update(void)
{
	// 1. 現在のウェーブごとの更新処理を実行（Stateパターン）
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

	// 2. 登録されているすべてのエネミーを更新
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Update();
		}
	}
}

// 描画処理
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
	// デバッグ表示：現在のウェーブと経過時間を画面左上に描画
	const char* waveNames[] = { "WAVE 01", "WAVE 02", "WAVE 03", "END (ALL CLEARED)" };
	DrawFormatString(10, 80, 0xffff00, "Enemy Wave: %s (Time: %d)", waveNames[static_cast<int>(wave_)], cntWave_);
#endif
}

// メモリ解放処理
void EnemyManager::Release(void)
{
	// 生成したエネミーインスタンスの解放
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Release();
			delete enemy;
		}
	}
	enemys_.clear();

	// エネミーの元モデルの解放
	for (int id : enemyModelIds_)
	{
		if (id != -1)
		{
			MV1DeleteModel(id);
		}
	}
	enemyModelIds_.clear();

	// 攻撃エフェクトモデルの解放
	for (int id : attackEffectModelIds_)
	{
		if (id != -1)
		{
			MV1DeleteModel(id);
		}
	}
	attackEffectModelIds_.clear();
}

// ウェーブ切り替え処理
void EnemyManager::ChangeWave(WAVE wave)
{
	wave_ = wave;
	cntWave_ = 0;   // ウェーブタイマーをリセット
	cntSpawn_ = 0;  // 敵の出現タイマーをリセット
}

// 第1ウェーブ：EnemyDemonを180フレーム間隔で生成。500フレームを超えたら第2ウェーブへ
void EnemyManager::UpdateWave01(void)
{
	cntWave_++;
	cntSpawn_++;

	// 180フレーム間隔でDemonを生成（500フレームの間に計2体出現）
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

// 第2ウェーブ：EnemyWizardを240フレーム間隔で生成。500フレームを超えたら第3ウェーブへ
void EnemyManager::UpdateWave02(void)
{
	cntWave_++;
	cntSpawn_++;

	// 240フレーム間隔でWizardを生成（500フレームの間に計2体出現）
	if (cntSpawn_ >= 240)
	{
		cntSpawn_ = 0;
		CreateEnemyWizard();
	}

	// 500フレームを超えたら第3ウェーブへ進む
	if (cntWave_ > 500)
	{
		ChangeWave(WAVE::WAVE03);
	}
}

// 第3ウェーブ：エネミーが全て撃破されたら最終ウェーブ（END）へ
void EnemyManager::UpdateWave03(void)
{
	cntWave_++;

	// 生きている敵の数をカウント
	int aliveCount = 0;
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr && enemy->IsAlive())
		{
			aliveCount++;
		}
	}

	// 全エネミー撃破（生きている敵が0体）でENDウェーブへ移行！
	if (aliveCount == 0)
	{
		ChangeWave(WAVE::END);
	}
}

// 終了ウェーブ：全敵撃破後の状態
void EnemyManager::UpdateWaveEnd(void)
{
	// 次章（41章）でGameClear画面への遷移処理が連携します
}

// デーモンの生成
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

// ウィザードの生成
void EnemyManager::CreateEnemyWizard(void)
{
	EnemyWizard* wizard = new EnemyWizard();
	wizard->Init(
		EnemyBase::TYPE::WIZARD,
		enemyModelIds_[static_cast<int>(EnemyBase::TYPE::WIZARD)],
		attackEffectModelIds_[0], // ファイアボールのモデルID
		player_
	);
	enemys_.push_back(wizard);
}

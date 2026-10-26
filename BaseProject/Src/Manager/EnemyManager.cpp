#include "EnemyManager.h"
#include "../Object/Enemy/EnemyDemon.h" 
#include "../Object/Enemy/EnemyWizard.h" 
#include "../Object/Player/Player.h"
#include "../Application.h"
EnemyManager::EnemyManager(Player* player) :
	player_(player),
	cntSpawn_(0)
{
}

EnemyManager::~EnemyManager(void)
{
	Release();
}

void EnemyManager::Init(void)
{
	cntSpawn_ = 0;
	enemys_.clear();

	enemyModelIds_.clear();

	// ★ エネミーモデルをベースとして事前にロードしておく
	// ※プロジェクトのパス構造に合わせてファイル名を調整してください
	enemyModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Enemy/Demon.mv1").c_str())
	);
	enemyModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Enemy/Wizard.mv1").c_str())
	);
	enemyModelIds_.emplace_back(
		MV1LoadModel((Application::PATH_MODEL + "Enemy/Giant.mv1").c_str()));

}

void EnemyManager::Update(void)
{
	// 出現カウンタを進める
	cntSpawn_++;

	// 一定間隔（60フレーム）に達したら新しいエネミーを生成する[cite: 9]
	if (cntSpawn_ >= SPAWN_INTERVAL)
	{
		cntSpawn_ = 0;

		EnemyBase* newEnemy = nullptr;

		// 例：ランダムで Demon か Wizard を切り替える場合
		if (rand() % 2 == 0)
		{
			newEnemy = new EnemyDemon();
			newEnemy->Init(EnemyBase::TYPE::DEMON,
				enemyModelIds_[static_cast<int>(EnemyBase::TYPE::DEMON)],
				player_);
		}
		else
		{
			newEnemy = new EnemyWizard();
			newEnemy->Init(EnemyBase::TYPE::WIZARD,
				enemyModelIds_[static_cast<int>(EnemyBase::TYPE::WIZARD)],
				player_);
		}

		enemys_.push_back(newEnemy);
	}
	

	// 登録されているすべてのエネミーを更新
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Update();
		}
	}
}

void EnemyManager::Draw(void)
{
	// すべてのエネミーを描画
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Draw();
		}
	}
}

void EnemyManager::Release(void)
{
	// 可変長配列に保持されているすべてのエネミーを解放して削除
	for (auto* enemy : enemys_)
	{
		if (enemy != nullptr)
		{
			enemy->Release();
			delete enemy;
		}
	}
	// ★ ベースとなるモデルデータのメモリ解放
	for (int id : enemyModelIds_)
	{
		if (id != -1)
		{
			MV1DeleteModel(id);
		}
	}
	enemys_.clear();
}
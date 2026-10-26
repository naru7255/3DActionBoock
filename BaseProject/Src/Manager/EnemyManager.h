#pragma once
#include <vector>

class EnemyBase;
class Player;

class EnemyManager
{
public:
	// 敵の出現間隔（60フレーム ＝ 約1秒）
	static constexpr int SPAWN_INTERVAL = 60;

	// コンストラクタ（プレイヤーのポインタを受け取る）
	EnemyManager(Player* player);
	~EnemyManager(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	const std::vector<EnemyBase*>& GetEnemys(void) const { return enemys_; }
private:
	// エネミーを格納する可変長配列
	std::vector<EnemyBase*> enemys_;

	// プレイヤーのポインタ
	Player* player_;

	// 出現間隔を測るカウンタ
	int cntSpawn_;

	// ★ エネミー用のベースモデルハンドルIDを保持する可変長配列
	std::vector<int> enemyModelIds_;
};
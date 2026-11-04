#pragma once
#include <vector>

class EnemyBase;
class Player;

// =============================================================================
// エネミー管理クラス（EnemyManager）
// エネミーの生成・更新・描画、およびウェーブ進行を統括管理する（教材第40章）
// =============================================================================
class EnemyManager
{
public:
	// エネミー出現ウェーブ（状態遷移）
	enum class WAVE
	{
		WAVE01, // 第1ウェーブ：Demon出現（突進型）
		WAVE02, // 第2ウェーブ：Wizard出現（魔法型）
		WAVE03, // 第3ウェーブ：残敵掃討（全滅待ち）
		END     // 終了ウェーブ（全滅達成・ゲームクリアへ）
	};

	// コンストラクタ / デストラクタ
	EnemyManager(Player* player);
	~EnemyManager(void);

	// 基本関数（初期化、更新、描画、解放）
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	// 現在生存している全エネミーリストの取得（当たり判定などで使用）
	const std::vector<EnemyBase*>& GetEnemys(void) const { return enemys_; }

	// 現在のウェーブ進行状態を取得（第41章：ゲームクリア判定で使用）
	WAVE GetWave(void) const { return wave_; }

private:
	// ウェーブ状態の切り替え
	void ChangeWave(WAVE wave);

	// 各ウェーブごとの更新処理
	void UpdateWave01(void);
	void UpdateWave02(void);
	void UpdateWave03(void);
	void UpdateWaveEnd(void);

	// 各種エネミーの生成ヘルパー関数
	void CreateEnemyDemon(void);
	void CreateEnemyWizard(void);

private:
	std::vector<EnemyBase*> enemys_;        // 生成された全エネミーのインスタンス配列
	Player* player_;                        // プレイヤーへのポインタ

	WAVE wave_;                             // 現在進行中のウェーブ
	int cntWave_;                           // ウェーブ開始からの経過時間（フレーム単位）
	int cntSpawn_;                          // 敵の出現間隔カウント（フレーム単位）

	std::vector<int> enemyModelIds_;        // エネミーの元となる3DモデルハンドルID配列
	std::vector<int> attackEffectModelIds_; // 攻撃エフェクト用3DモデルハンドルID配列
};

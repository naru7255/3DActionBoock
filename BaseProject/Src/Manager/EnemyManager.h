#pragma once
#include <vector>

class EnemyBase;
class Player;

// 複数エネミーの生成・ウェーブ管理クラス（教材 40章 準拠）
class EnemyManager
{
public:

	// 敵の生成ウェーブ（状態遷移）
	enum class WAVE
	{
		WAVE01, // 第1ウェーブ：デーモン（突進）
		WAVE02, // 第2ウェーブ：ウィザード（遠距離魔法）
		WAVE03, // 第3ウェーブ：残党全滅戦
		END     // 終了ウェーブ（ゲームクリアへ）
	};

	// コンストラクタ / デストラクタ
	EnemyManager(Player* player);
	~EnemyManager(void);

	// 初期化・更新・描画・メモリ解放
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	// エネミーリストの取得
	const std::vector<EnemyBase*>& GetEnemys(void) const { return enemys_; }

	// 現在のウェーブを取得（第41章：ゲームクリア判定で使用）
	WAVE GetWave(void) const { return wave_; }

private:

	// ウェーブ状態遷移
	void ChangeWave(WAVE wave);

	// ウェーブ別の更新処理
	void UpdateWave01(void);
	void UpdateWave02(void);
	void UpdateWave03(void);
	void UpdateWaveEnd(void);

	// 敵の生成ヘルパー関数
	void CreateEnemyDemon(void);
	void CreateEnemyWizard(void);

private:

	std::vector<EnemyBase*> enemys_;        // 生成した全エネミーの配列
	Player* player_;                        // プレイヤーのポインタ

	WAVE wave_;                             // 現在のウェーブ状態
	int cntWave_;                           // ウェーブ経過時間カウンタ（フレーム単位）
	int cntSpawn_;                          // 敵の出現間隔カウンタ（フレーム単位）

	std::vector<int> enemyModelIds_;        // エネミー用モデルハンドルID
	std::vector<int> attackEffectModelIds_; // 攻撃エフェクト用モデルハンドルID

};

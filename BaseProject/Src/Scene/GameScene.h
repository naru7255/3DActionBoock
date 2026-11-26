#pragma once
#include "SceneBase.h"
#include <DxLib.h>
#include <vector>

class Grid;
class BlockManager;
class Player;
class EnemyManager;
class HpManager;

// =============================================================================
// ゲームプレイシーンクラス（GameScene）
// プレイヤー、ステージブロック、エネミー、HP・武器UI、回復アイテム、衝突判定を統括管理する
// =============================================================================
class GameScene : public SceneBase
{
public:
	// ゲームバランス・演出用定数
	static constexpr int HEAL_DROP_RATE = 35;            // 敵撃破時の回復ハートドロップ確率（%）
	static constexpr int HEAL_ITEM_LIFE = 600;           // 回復アイテムの生存時間（600フレーム＝約10秒）
	static constexpr int HEAL_ITEM_BLINK_TIME = 120;     // 消滅前の点滅開始残りフレーム数（約2秒）
	static constexpr float HEAL_PICKUP_RADIUS = 60.0f;   // 回復アイテムの取得判定半径
	static constexpr float PLAYER_KNOCKBACK_JUMP = 20.0f;// プレイヤー被弾時のノックバック跳ね上げ力

	static constexpr int BOSS_MAX_HP = 10;               // ボス（ジャイアント）の最大HP
	static constexpr int BOSS_BG_FADE_SPEED = 2;         // ボス戦夕焼け背景のクロスフェード速度

	static constexpr float RANK_S_TIME = 60.0f;          // Sランク達成の目標タイム（秒）
	static constexpr int RANK_S_HP = 4;                  // Sランク達成の最低残りHP
	static constexpr float RANK_A_TIME = 120.0f;         // Aランク達成の目標タイム（秒）
	static constexpr int RANK_A_HP = 2;                  // Aランク達成の最低残りHP

	GameScene(void);
	virtual ~GameScene(void);

	// シーン基本関数（初期化、毎フレーム更新、描画、解放）
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:
	// 各種マネージャーおよびゲーム内オブジェクト
	Grid* grid_;                  // デバッグ用グリッド線
	BlockManager* blockManager_;  // 地面・ステージブロック管理
	Player* player_;              // プレイヤーキャラクター
	EnemyManager* enemyManager_;  // エネミー・ウェーブ管理
	HpManager* hpManager_;        // HP・武器アイコンUI管理

	// 地面衝突判定用の線分座標（デバッグ描画用）
	VECTOR lineTopPos_;
	VECTOR lineDownPos_;

	int imgBg_;                   // 背景画像のハンドルID（昼）
	int imgBgBoss_;               // ボス戦用の夕焼け背景画像ハンドルID
	int bossBgAlpha_;             // ボス戦夕焼けクロスフェード用アルファ値（0〜255）
	int imgGameover_;             // ゲームオーバー画像のハンドルID
	int imgGameClear_;            // ゲームクリア画像のハンドルID
	int imgHealHeart_;            // 回復アイテム用ハート画像ハンドルID

	// 回復ドロップアイテム構造体
	struct HealItem
	{
		VECTOR pos;
		int lifeTimer;
		float animPhase;
		bool isAlive;
	};
	std::vector<HealItem> healItems_; // フィールド上の回復アイテムリスト
	int elapsedFrames_;               // プレイ経過フレーム数（クリアタイム計測用）
	float bossHpDisplay_;             // ボスHPバーの追従ダメージ演出用表示値

	// 総合当たり判定関数
	void Collision(void);

	// 個別当たり判定関数
	void CollisionStage(void);    // プレイヤーと地面ブロックの接地判定（レイキャスト）
	void CollisionWall(void);     // プレイヤーと壁ブロックの衝突スライド判定
	void CollisionEnemyWall(void);// エネミーと壁ブロックの衝突スライド判定
	void CollisionEnemy(void);    // プレイヤーとエネミー本体・弾の衝突判定
	void CollisionWeapon(void);   // プレイヤーの武器とエネミー本体の衝突判定

	// 共通ヘルパー関数
	void ApplyPlayerDamage(const VECTOR& attackerPos);
	void FormatElapsedTime(char* outBuf, size_t bufSize) const;

	// 回復アイテム管理関数
	void SpawnHealItem(const VECTOR& pos);
	void UpdateHealItems(void);
	void DrawHealItems(void);

	// HUD・リザルト描画関数
	void DrawStatusHud(void);
	void DrawClearResult(int baseY);
	void DrawBossHpBar(void);
	void DrawControlGuide(void);
};
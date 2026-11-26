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

	int imgBg_;                   // 背景画像のハンドルID（昼：教材第42章追加実装）
	int imgBgBoss_;               // ボス戦用の夕焼け背景画像ハンドルID
	int bossBgAlpha_;             // ボス戦夕焼けクロスフェード用アルファ値（0〜255）
	int imgGameover_;             // ゲームオーバー画像のハンドルID（教材第35章）
	int imgGameClear_;            // ゲームクリア画像のハンドルID（教材第41章）
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
	void CollisionEnemy(void);    // プレイヤーとエネミー本体・弾の衝突判定
	void CollisionWeapon(void);   // プレイヤーの武器とエネミー本体の衝突判定

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
#pragma once
#include "SceneBase.h"
#include <DxLib.h>

class Grid;
class BlockManager;
class Player;
class EnemyManager; 
class HpManager;

// =============================================================================
// ゲームプレイシーンクラス（GameScene）
// プレイヤー、ステージブロック、エネミー、HP UI、衝突判定を統括管理する
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
	HpManager* hpManager_;        // HPアイコンUI管理

	// 地面衝突判定用の線分座標（デバッグ描画用）
	VECTOR lineTopPos_;
	VECTOR lineDownPos_;

	int imgGameover_;             // ゲームオーバー画像のハンドルID

	// 総合当たり判定関数
	void Collision(void);

	// 個別当たり判定関数
	void CollisionStage(void);    // プレイヤーと地面ブロックの接地判定（レイキャスト）
	void CollisionWall(void);     // プレイヤーと壁ブロックの衝突スライド判定
	void CollisionEnemy(void);    // プレイヤーとエネミー本体・弾の衝突判定
	void CollisionWeapon(void);   // プレイヤーの武器とエネミー本体の衝突判定
};

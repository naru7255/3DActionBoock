#pragma once
#include "SceneBase.h"
#include <DxLib.h>

class Grid;
class BlockManager;
class Player;
class EnemyManager; 
class HpManager;

// プレイヤー、ステージ、敵などをまとめて管理するゲームプレイ用シーン
class GameScene : public SceneBase
{
public:
	GameScene(void);
	virtual ~GameScene(void);

   // ゲーム内オブジェクトの生成、更新、描画、解放
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:
    // ゲーム画面で使うオブジェクトと各管理クラス。
	Grid* grid_;
	BlockManager* blockManager_;
	Player* player_;
	EnemyManager* enemyManager_;
	HpManager* hpManager_;
	// 地面との衝突用線分
	VECTOR lineTopPos_;
	VECTOR lineDownPos_;


	

    int imgGameover_; // ゲームオーバー画像のDxLibハンドル

 // 各オブジェクト間の衝突判定を実行する
	void Collision(void);
  // プレイヤーと床、壁、敵、武器それぞれの衝突を判定する
	void CollisionStage(void);
	void CollisionWall();
	void CollisionEnemy(void);
	void CollisionWeapon(void);
};
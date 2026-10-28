#pragma once
#include "SceneBase.h"
#include <DxLib.h>

class Grid;
class BlockManager;
class Player;
class EnemyManager; 
class HpManager;

class GameScene : public SceneBase
{
public:
	GameScene(void);
	virtual ~GameScene(void);

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

	// 衝突判定
	void Collision(void);
	void CollisionStage(void);
	void CollisionWall();
	void CollisionEnemy(void);
	void CollisionWeapon(void);
};
#pragma once
#include <DxLib.h>
#include <vector>

class Player;
class AnimationController;
class ShotBase;

class EnemyBase
{
public:
	// エネミーの状態
	enum class STATE
	{
		NONE,
		STANDBY, // 待機・移動中
		ATTACK,  // 攻撃中
	};
	// エネミー種別
	enum class TYPE
	{
		DEMON,
		WIZARD,
		GIANT,
		MAX,
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		ATTACK,
		DEATH,
		HIT_REACT,
		IDLE,
		JUMP,
		RUN,
		WALK,
		MAX,
	};

	EnemyBase(void);
	virtual ~EnemyBase(void);

	// 初期化関数（種別、ベースモデルID、プレイヤーポインタを受け取るように拡張）
	void Init(TYPE type, int baseModelId, Player* player);
	void Update(void);
	void Draw(void);
	void Release(void);

	// 座標を取得
	VECTOR GetPos(void) const { return pos_; }

	// 衝突判定用半径を取得
	float GetCollisionRadius(void) const { return collisionRadius_; }

	// 弾のリストを取得
	const std::vector<ShotBase*>& GetShots(void) const { return shots_; }


protected: // 派生クラスからアクセスできるように protected に変更
	// エネミーの出現位置をランダムに設定する関数
	void SetSpawnPosition(void);
	// プレイヤーの方向を向く関数
	void LookPlayer(void);
	// プレイヤーの方向に移動する関数
	void Move(void);
	// エネミーのパラメータを設定する純粋仮想関数
	virtual void SetParam(void) = 0;

	// 状態遷移関数
	void ChangeState(STATE state);

	// 状態別の仮想関数（派生クラスでオーバーライドできるようにする）
	virtual void ChangeStandby(void);
	virtual void ChangeAttack(void);

	virtual void UpdateStandby(void);
	virtual void UpdateAttack(void);

	virtual void DrawStandby(void);
	virtual void DrawAttack(void);

	void UpdateShots(void);

	void DrawShots(void);

	void Shot(void); 

	

protected:
	// 種別
	TYPE type_;

	// モデルハンドルID
	int modelId_;

	// 座標・角度・スケール
	VECTOR pos_;
	VECTOR angles_;
	VECTOR scales_;

	// プレイヤーのポインタ
	Player* player_;
	// 弾のポインタ配列
	std::vector<ShotBase*> shots_;
	int shotModelId_;
	// 方向ベクトル（単位ベクトル）
	VECTOR moveDir_;
	// 状態
	STATE state_;
	// 移動スピード
	float speed_;
	// HP
	int hp_;           
	// 出現範囲の比率
	float spawnRange_; 
	// 攻撃間隔
	int cntAttack_;
	// 衝突判定用半径
	float collisionRadius_;
	// アニメーションコントローラ
	AnimationController* animController_;
};
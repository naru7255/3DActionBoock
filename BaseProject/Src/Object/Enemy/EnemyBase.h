#pragma once
#include <DxLib.h>
#include <vector>

class Player;
class AnimationController;
class ShotBase;

// 敵に共通する状態、移動、攻撃、被弾処理をまとめた基底クラス
class EnemyBase
{
public:

    // 敵の行動状態
	enum class STATE
	{
		NONE,
		STANDBY,     // 待機・移動中
		ATTACK,      // 攻撃中
		HIT_REACT,   // 被弾リアクション中
		DEAD_REACT,  // 死亡演出中
		END,         // 終了（消滅）
	};

 // 敵の種類。派生クラスのパラメータ設定に使用する
	enum class TYPE
	{
		DEMON,
		WIZARD,
		GIANT,
		MAX,
	};

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

	// 標準の自己発光色（教材 27章：エネミーモデルの色調整）
	static constexpr COLOR_F COLOR_EMI_DEFAULT = { 0.6f, 0.6f, 0.6f, 1.0f };

	EnemyBase(void);
	virtual ~EnemyBase(void);

  // モデルと弾用モデルを受け取り、敵を初期化する
	void Init(TYPE type, int baseModelId, int baseAttackEffectModelId, Player* player);
  // 弾用モデルを使わない敵の初期化
	void Init(TYPE type, int baseModelId, Player* player);

  // 敵と弾の更新、描画、使用リソースの解放
	void Update(void);
	void Draw(void);
	void Release(void);

  // 位置、当たり判定半径、発射した弾一覧を取得する
	VECTOR GetPos(void) const { return pos_; }
	float GetCollisionRadius(void) const { return collisionRadius_; }
	const std::vector<ShotBase*>& GetShots(void) const { return shots_; }

  // 現在の状態が衝突対象か、生存中かを判定する
	bool IsCollisionState(void) const;
	virtual bool IsAlive() const;
    // ダメージを適用する（敵ごとの挙動に応じて派生クラスで変更可能）
	virtual void Damage(int damage);

protected:

	void SetSpawnPosition(void);
	void LookPlayer(void);
	void Move(void);
	virtual void SetParam(void) = 0;

	void ChangeState(STATE state);

	virtual void ChangeStandby(void);
	virtual void ChangeAttack(void);
	virtual void ChangeHitReact(void);
	virtual void ChangeDeadReact(void);

	virtual void UpdateStandby(void);
	virtual void UpdateAttack(void);
	virtual void UpdateHitReact(void);
	virtual void UpdateDeadReact(void);

	virtual void DrawStandby(void);
	virtual void DrawAttack(void);
	virtual void DrawHitReact(void);
	virtual void DrawDeadReact(void);

	void UpdateShots(void);
	void DrawShots(void);
	void Shot(void);

	// 有効な弾を取得（教材 31章 準拠：オブジェクトプーリング）
	ShotBase* GetValidShot(void);

protected:

	TYPE type_;
	int modelId_;
	VECTOR pos_;
	VECTOR angles_;
	VECTOR scales_;
	Player* player_;
	std::vector<ShotBase*> shots_;
	int baseAttackEffectModelId_;
	int shotModelId_;
	VECTOR moveDir_;
	STATE state_;
	float speed_;
	int hp_;
	float spawnRange_;
	int cntAttack_;
	int hitTimer_;
	int deadTimer_;
	float collisionRadius_;
	AnimationController* animController_;

};

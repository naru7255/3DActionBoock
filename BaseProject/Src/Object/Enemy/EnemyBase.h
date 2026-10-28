#pragma once
#include <DxLib.h>
#include <vector>

class Player;
class AnimationController;
class ShotBase;

class EnemyBase
{
public:
	enum class STATE
	{
		NONE,
		STANDBY,     // 待機・移動中
		ATTACK,      // 攻撃中
		HIT_REACT,   // 被弾リアクション中
		DEAD_REACT,  // 死亡演出中
		END,         // 終了（消滅）
	};

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

	EnemyBase(void);
	virtual ~EnemyBase(void);

	void Init(TYPE type, int baseModelId, Player* player);
	void Update(void);
	void Draw(void);
	void Release(void);

	VECTOR GetPos(void) const { return pos_; }
	float GetCollisionRadius(void) const { return collisionRadius_; }
	const std::vector<ShotBase*>& GetShots(void) const { return shots_; }

	bool IsCollisionState(void) const;
	virtual bool IsAlive() const;
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

protected:
	TYPE type_;
	int modelId_;
	VECTOR pos_;
	VECTOR angles_;
	VECTOR scales_;
	Player* player_;
	std::vector<ShotBase*> shots_;
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
#pragma once
#include <DxLib.h>
#include "../Weapon/WeaponPunch.h" 

class AnimationController;
class WeaponBase;

class Player
{
public:
	// 3Dモデル内のアニメーション番号に対応する列挙型
	enum class ANIM_TYPE
	{
		DEATH,
		DUCK,
		HIT_REACT,
		IDLE,
		IDLE_ATTACK,
		IDLE_HOLD,
		JUMP,
		JUMP_IDLE,
		JUMP_LAND,
		NO,
		PUNCH,
		RUN,
		RUN_ATTACK,
		RUN_HOLD,
		WALK,
		WALK_HOLD,
		WAVE,
		YES,
		MAX,
	};

	// プレイヤーの行動ステート（状態遷移）
	enum class STATE
	{
		NONE,
		STANDBY,   // 通常待機・操作可能
		KNOCKBACK, // 被弾ノックバック中
		ATTACK,    // 攻撃中[cite: 25, 37]
		DEAD,      // 死亡演出中
		END,       // 死亡確定（ゲームオーバー待ち）
		VICTORY,   // 勝利演出
	};

	// プレイヤーパラメータ定数
	static constexpr VECTOR SCALES = { 0.5f, 0.5f, 0.5f };
	static constexpr VECTOR DEFAULT_POS = { 0.0f, 80.0f, 0.0f };
	static constexpr COLOR_F COLOR_EMI_DEFAULT = { 1.0f, 1.0f, 1.0f, 1.0f };
	static constexpr float COLLISION_RADIUS = 50.0f;
	static constexpr float SPEED_MOVE = 10.0f;
	static constexpr float SPEED_KNOCKBACK = 10.0f;
	static constexpr float GRAVITY_POW = 0.8f;
	static constexpr float COLLISION_LEN = 10.0f;
	static constexpr float JUMP_POW = 20.0f;
	static constexpr int MAX_HP = 5;

	Player(void);
	~Player(void);

	void Init(void);
	void Update(float cameraAngleY = 0.0f);
	void Draw(void);
	void Release(void);

	void SetPos(const VECTOR& pos) {
		pos_ = pos;
		if (modelId_ != -1)
		{
			MV1SetPosition(modelId_, pos_);
		}
	}
	VECTOR GetPos(void) const { return pos_; }
	VECTOR GetPrevPos(void) const { return prevPos_; }
	float GetJumpPow(void) const { return jumpPow_; }

	void KnockBack(VECTOR dirXZ, float jumpPow);
	void CollisionStage(float groundY);
	void Respawn(void);

	void Damage(int damage);
	int GetHp(void) const { return hp_; }

	bool IsInvincible(void) const {
		return state_ == STATE::KNOCKBACK || state_ == STATE::DEAD || state_ == STATE::END;
	}
	STATE GetState(void) const { return state_; }

	WeaponBase* GetUseWeapon(void) const { return useWeapon_; } 

private:
	void ChangeState(STATE state);
	void ChangeStandby(void);
	void UpdateStandby(float cameraAngleY);

	void ChangeKnockback(void);
	void UpdateKnockback(void);

	void ChangeDead(void);
	void UpdateDead(void);

	void ProcessMove(float cameraAngleY);
	void ProcessJump(void);
	void ProcessAttack(void); // 【追加】攻撃受付処理[cite: 25]

private:
	int modelId_;
	float jumpPow_;
	bool isJump_;
	bool isLanding_;
	int hp_;
	VECTOR pos_;
	VECTOR prevPos_;
	VECTOR angles_;
	VECTOR scales_;
	STATE state_;
	VECTOR knockBackDir_;
	int cntKnockBack_;
	bool isFlash_;
	ANIM_TYPE animType_;
	AnimationController* animController_;

	
	WeaponBase* useWeapon_;     // 武器を切り替えるための共通の器（ポインタ）
	WeaponPunch weaponPunch_;   // パンチの実体
	VECTOR moveDir_;            // プレイヤーの移動方向（攻撃時に使用）
};
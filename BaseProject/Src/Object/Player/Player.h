#pragma once
#include <DxLib.h>
#include <map>
#include "../Weapon/WeaponBase.h"

class AnimationController;

class Player
{
public:
	// 3Dモデルのアニメーション番号に対応する列挙型
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
		STANDBY,   // 通常待機・移動・ジャンプ操作可能
		KNOCKBACK, // 被弾ノックバック中（操作不能・無敵）
		ATTACK,    // 攻撃モーション中
		DEAD,      // 死亡モーション再生中
		END,       // 死亡確定（ゲームオーバー待ち）
		VICTORY,   // クリア勝利演出
	};

	// 定数定義（大きさ、各種速度、当たり判定半径、HP）
	static constexpr VECTOR SCALES = { 0.5f, 0.5f, 0.5f };
	static constexpr VECTOR DEFAULT_POS = { 0.0f, 80.0f, 0.0f };
	static constexpr COLOR_F COLOR_EMI_DEFAULT = { 1.0f, 1.0f, 1.0f, 1.0f };
	static constexpr float COLLISION_RADIUS = 50.0f; // 衝突判定用の球体半径
	static constexpr float SPEED_MOVE = 10.0f;       // 移動速度
	static constexpr float SPEED_KNOCKBACK = 10.0f;  // ノックバック後退速度
	static constexpr float GRAVITY_POW = 0.8f;       // 重力加速度
	static constexpr float COLLISION_LEN = 10.0f;    // 地面衝突用レイキャスト長さ
	static constexpr float JUMP_POW = 20.0f;         // ジャンプ初速度
	static constexpr int MAX_HP = 5;                 // 最大体力

	Player(void);
	~Player(void);

	// 基本関数（初期化、更新、描画、解放）
	void Init(void);
	void Update(float cameraAngleY = 0.0f);
	void Draw(void);
	void Release(void);

	// 座標の設定と取得
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

	// 被弾時に指定方向（XZ平面）へ吹き飛ばす
	void KnockBack(VECTOR dirXZ, float jumpPow);

	// 地面との接地時に呼び出され、Y座標とジャンプ状態を確定する
	void CollisionStage(float groundY);

	// ステージ外へ落下した際のリスポーン処理
	void Respawn(void);

	// ダメージ処理および現在HPの取得
	void Damage(int damage);
	int GetHp(void) const { return hp_; }

	// 無敵判定（ノックバック中や死亡中はダメージを受けない）
	bool IsInvincible(void) const {
		return state_ == STATE::KNOCKBACK || state_ == STATE::DEAD || state_ == STATE::END;
	}
	STATE GetState(void) const { return state_; }

	// 現在装備している武器を取得（当たり判定などで使用）
	WeaponBase* GetUseWeapon(void) const { return useWeapon_; }

	// ゲームクリア時の勝利演出を開始する（教材第41章）
	void Victory(void);

private:
	// ステート遷移と状態ごとの更新処理
	void ChangeState(STATE state);
	void ChangeStandby(void);
	void UpdateStandby(float cameraAngleY);

	void ChangeKnockback(void);
	void UpdateKnockback(void);

	void ChangeDead(void);
	void UpdateDead(void);

	void ChangeVictory(void);
	void UpdateVictory(void);

	// 各種プレイヤー操作処理
	void ProcessMove(float cameraAngleY);
	void ProcessJump(void);
	void ProcessAttack(void);       // 攻撃入力処理（Bキー / Jキー）
	void ProcessChangeWeapon(void); // 武器切り替え入力処理（Cキー）

private:
	int modelId_;                           // 3DモデルハンドルID
	float jumpPow_;                         // 上下方向の移動速度（ジャンプ力・重力）
	bool isJump_;                           // ジャンプ中フラグ
	bool isLanding_;                        // 着地モーション再生中フラグ
	int hp_;                                // 現在体力
	VECTOR pos_;                            // 現在座標
	VECTOR prevPos_;                        // 1フレーム前の座標
	VECTOR angles_;                         // モデルの回転角度
	VECTOR scales_;                         // モデルの拡縮率
	STATE state_;                           // 現在の行動ステート
	VECTOR knockBackDir_;                   // ノックバックの方向ベクトル
	int cntKnockBack_;                      // ノックバック点滅用カウンタ
	AnimationController* animController_;   // アニメーション制御クラス

	// --- 武器管理（std::map 連想配列） ---
	WeaponBase* useWeapon_;                             // 現在装備中の武器ポインタ
	std::map<WeaponBase::TYPE, WeaponBase*> weapons_;   // 所有武器の連想配列
	VECTOR moveDir_;                                    // プレイヤーの向き（攻撃発射方向に使用）
};

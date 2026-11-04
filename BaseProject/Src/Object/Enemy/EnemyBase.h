#pragma once
#include <DxLib.h>
#include <vector>

class Player;
class AnimationController;
class ShotBase;

// =============================================================================
// エネミー基底クラス（EnemyBase）
// すべてのエネミー（Demon, Wizard等）に共通する行動・描画・当たり判定を管理
// =============================================================================
class EnemyBase
{
public:
	// エネミーの行動ステート（状態遷移）
	enum class STATE
	{
		NONE,
		STANDBY,     // 通常待機・移動・索敵中
		ATTACK,      // 攻撃モーション中
		HIT_REACT,   // 被弾リアクション中（怯み・点滅）
		DEAD_REACT,  // 死亡演出中（倒れるモーション・点滅）
		END,         // 処理終了（消滅）
	};

	// エネミーの種類
	enum class TYPE
	{
		DEMON,
		WIZARD,
		GIANT,
		MAX,
	};

	// アニメーションの種類（FBX内のインデックス対応）
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

	// 標準のマテリアル拡散光色（教材第27章：モデル本来の色で描画）
	static constexpr COLOR_F COLOR_EMI_DEFAULT = { 0.6f, 0.6f, 0.6f, 1.0f };

	EnemyBase(void);
	virtual ~EnemyBase(void);

	// 初期化処理（弾エフェクトモデルあり版：Wizardなど）
	void Init(TYPE type, int baseModelId, int baseAttackEffectModelId, Player* player);
	// 初期化処理（弾エフェクトモデルなし版：Demonなど）
	void Init(TYPE type, int baseModelId, Player* player);

	// 毎フレームの更新・描画・リソース解放
	void Update(void);
	void Draw(void);
	void Release(void);

	// 座標、当たり判定半径、保持している弾リストの取得
	VECTOR GetPos(void) const { return pos_; }
	float GetCollisionRadius(void) const { return collisionRadius_; }
	const std::vector<ShotBase*>& GetShots(void) const { return shots_; }

	// 当たり判定が有効な状態（STANDBY または ATTACK）か判定
	bool IsCollisionState(void) const;
	// 生存しているかどうか（END以外なら生存）
	virtual bool IsAlive() const;
	// 現在の行動ステートを取得（死亡演出中かどうかの判定などで使用）
	// 保持しているすべての弾を強制消滅・無効化する
	void ClearShots(void);

	STATE GetState(void) const { return state_; }
	// ダメージ処理（HP減少、被弾・死亡ステートへの遷移）
	virtual void Damage(int damage);

protected:
	// 内部処理関数
	void SetSpawnPosition(void); // 初期出現座標をランダムに決定
	void LookPlayer(void);       // プレイヤーの方向を向く
	void Move(void);             // プレイヤー方向へ移動
	virtual void SetParam(void) = 0; // 派生クラスごとの固有パラメータ設定（純粋仮想関数）

	// ステート遷移
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

	// 弾の更新・描画・発射
	void UpdateShots(void);
	void DrawShots(void);
	void Shot(void);

	// 未使用の弾インスタンスを取得（教材第31章：オブジェクトプーリング）
	ShotBase* GetValidShot(void);

protected:
	TYPE type_;                              // エネミー種別
	int modelId_;                            // 3DモデルハンドルID（MV1DuplicateModelで複製）
	VECTOR pos_;                             // 現在座標
	VECTOR angles_;                          // 回転角度
	VECTOR scales_;                          // 拡縮率
	Player* player_;                         // 追従対象のプレイヤーポインタ
	std::vector<ShotBase*> shots_;           // 発射した弾の管理配列
	int baseAttackEffectModelId_;            // 攻撃エフェクト用モデルID
	int shotModelId_;                        // 弾モデルID
	VECTOR moveDir_;                         // 移動方向ベクトル
	STATE state_;                            // 現在の行動ステート
	float speed_;                            // 移動速度
	int hp_;                                 // 現在体力
	float spawnRange_;                       // 出現範囲倍率
	int cntAttack_;                          // 攻撃間隔カウント用タイマー
	int hitTimer_;                           // 被弾怯みタイマー
	int deadTimer_;                          // 死亡演出タイマー
	float collisionRadius_;                  // 衝突判定用の球体半径
	AnimationController* animController_;    // アニメーション制御クラス
};

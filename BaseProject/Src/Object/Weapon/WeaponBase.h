#pragma once
#include <DxLib.h>

// =============================================================================
// 武器基底クラス（ポリモーフィズム用インターフェース）
// すべての武器（パンチ、斧、ドリルなど）が継承する親クラス
// =============================================================================
class WeaponBase
{
public:
	// 武器の種別
	enum class TYPE
	{
		NONE = -1,
		PUNCH, // パンチ（近接・4方向ドリル衝撃波）
		AXE,   // 斧（投擲ブーメラン）
		DRILL, // ドリル（高速回転ロケット貫通型）
	};

	WeaponBase(void) : type_(TYPE::NONE) {}
	virtual ~WeaponBase(void) {}

	// 武器種別の取得（UI表示などで使用）
	virtual TYPE GetType(void) const { return type_; }

	// 純粋仮想関数：各武器で必ずオーバーライドする
	virtual void Init(TYPE type) = 0;              // 初期化（画像・モデルのロードなど）
	virtual void Update(void) = 0;                 // 毎フレーム更新処理（移動や回転など）
	virtual void Draw(void) = 0;                   // 描画処理

	virtual bool IsAlive() const = 0;              // 武器が使用中（有効）かどうか
	virtual VECTOR GetPos() const = 0;             // 武器の現在座標を取得
	virtual float GetCollisionRadius() const = 0;  // 衝突判定用の球体半径を取得

	// 武器を使用（攻撃発射）する関数
	// pos : 発射開始座標（プレイヤー座標）
	// dir : 発射方向ベクトル（プレイヤーの向き）
	virtual void Use(VECTOR pos, VECTOR dir) = 0;

protected:
	TYPE type_;                                    // 武器種別
};
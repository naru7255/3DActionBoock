#pragma once
#include "../Object/Player/Player.h"
#include "../Object/Weapon/WeaponBase.h"

// =============================================================================
// UI管理クラス（HpManager）
// プレイヤーの現在HP（ハート）および現在装備中の武器アイコンを画面上に2D描画する
// 被弾時のハート演出および武器切り替え時のポップアニメーション演出を行う
// =============================================================================
class HpManager
{
public:
	// HPアイコンの描画開始位置（画面左上基準）
	static constexpr int START_X = 60;
	static constexpr int START_Y = 50;
	// HPアイコン1個あたりの横幅間隔
	static constexpr int HP_ICON_WIDTH = 100;

	// ハートアニメーション持続時間（30フレーム＝約0.5秒間）
	static constexpr int ANIM_DURATION = 30;

	// 武器アイコンの描画位置（ハートの下）
	static constexpr int WEAPON_ICON_X = 60;
	static constexpr int WEAPON_ICON_Y = 135;
	// 武器切り替えアニメーション持続時間（15フレーム＝約0.25秒間）
	static constexpr int WEAPON_ANIM_DURATION = 15;

	HpManager(Player* player);
	~HpManager(void);

	// 基本関数（初期化、更新、描画、解放）
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

private:
	Player* player_;                       // プレイヤーへのポインタ
	int imgHeart_;                         // 生存中のハート画像ハンドル
	int imgHeartLost_;                     // 減少した空のハート画像ハンドル

	int prevHp_;                           // 1フレーム前のプレイヤーHP
	int animTimers_[Player::MAX_HP];       // 各ハートのダメージアニメーションタイマー

	// --- 武器UI関連 ---
	int imgWeaponPunch_;                   // パンチ武器アイコン画像ハンドル (Punch.png)
	int imgWeaponAxe_;                     // 斧武器アイコン画像ハンドル (Axe.png)
	int imgWeaponDrill_;                   // ドリル武器アイコン画像ハンドル (DrillX.pngの1コマ目)

	WeaponBase::TYPE prevWeaponType_;      // 1フレーム前の装備武器種別
	int weaponAnimTimer_;                  // 武器切り替え時のポップ演出タイマー
};
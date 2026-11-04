#pragma once

class Player;

// =============================================================================
// HP UI管理クラス（HpManager）
// プレイヤーの現在HPをハートアイコン（全角/空）で画面上に2D描画する（教材第34章）
// =============================================================================
class HpManager
{
public:
	// HPアイコンの描画開始位置（画面左上基準）
	static constexpr int START_X = 60;
	static constexpr int START_Y = 50;
	// HPアイコン1個あたりの横幅間隔
	static constexpr int HP_ICON_WIDTH = 100;

	HpManager(Player* player);
	~HpManager(void);

	// 基本関数（初期化、更新、描画、解放）
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

private:
	Player* player_;   // プレイヤーへのポインタ
	int imgHeart_;     // 生存中のハート画像ハンドル
	int imgHeartLost_; // 減少した空のハート画像ハンドル
};

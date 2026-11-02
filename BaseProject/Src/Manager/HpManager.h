#pragma once

class Player;

// プレイヤーのHPをハート画像として画面に表示する
class HpManager
{
public:
  // HPアイコンの描画開始位置（画面座標）
	static constexpr int START_X = 60;
	static constexpr int START_Y = 50;
   // HPアイコン1個あたりの横幅
	static constexpr int HP_ICON_WIDTH = 100;

	HpManager(Player* player);
	~HpManager(void);

    // 画像の読み込み、毎フレームの表示更新、画像ハンドルの解放
	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

private:
	Player* player_;       // プレイヤー情報へのポインタ
	int imgHeart_;         // 通常のハート画像
	int imgHeartLost_;     // 失ったハート画像
};
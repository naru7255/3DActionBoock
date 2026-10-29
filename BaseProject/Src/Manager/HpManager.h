#pragma once

class Player;

class HpManager
{
public:
	// HP描画の開始座標(X, Y)
	static constexpr int START_X = 60;
	static constexpr int START_Y = 50;
	// HPアイコンの描画幅
	static constexpr int HP_ICON_WIDTH = 100;

	HpManager(Player* player);
	~HpManager(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

private:
	Player* player_;       // プレイヤー情報へのポインタ
	int imgHeart_;         // 通常のハート画像
	int imgHeartLost_;     // 失ったハート画像
};
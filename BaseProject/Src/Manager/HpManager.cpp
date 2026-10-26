#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Object/Player/Player.h" 
#include "HpManager.h"

HpManager::HpManager(Player* player) :
	player_(player),
	imgHeart_(-1),
	imgHeartLost_(-1)
{
}

HpManager::~HpManager(void)
{
}

void HpManager::Init(void)
{
	std::string path;

	// ハート画像のロード
	path = Application::PATH_IMAGE + "Heart.png";
	imgHeart_ = LoadGraph(path.c_str());

	path = Application::PATH_IMAGE + "HeartLost.png";
	imgHeartLost_ = LoadGraph(path.c_str());
}

void HpManager::Update(void)
{
	// 特に処理なし
}

void HpManager::Draw(void)
{
	if (player_ == nullptr) return;

	int maxHp = Player::MAX_HP;
	int playerHp = player_->GetHp();

	for (int i = 0; i < maxHp; i++)
	{
		int x = START_X + (HP_ICON_WIDTH * i);
		int y = START_Y;

		// 残りHPに応じて画像を切り替える（残っていればハート、失っていればロスト画像）
		if (i < playerHp)
		{
			DrawRotaGraph(x, y, 1.0f, 0.0f, imgHeart_, TRUE);
		}
		else
		{
			DrawRotaGraph(x, y, 1.0f, 0.0f, imgHeartLost_, TRUE);
		}
	}
}

void HpManager::Release(void)
{
	if (imgHeart_ != -1)
	{
		DeleteGraph(imgHeart_);
		imgHeart_ = -1;
	}

	if (imgHeartLost_ != -1)
	{
		DeleteGraph(imgHeartLost_);
		imgHeartLost_ = -1;
	}
}
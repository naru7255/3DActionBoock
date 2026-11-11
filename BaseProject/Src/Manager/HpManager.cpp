#include <string>
#include <cmath>
#include <DxLib.h>
#include "../Application.h"
#include "../Object/Player/Player.h" 
#include "HpManager.h"

// =============================================================================
// コンストラクタ
// =============================================================================
HpManager::HpManager(Player* player) :
	player_(player),
	imgHeart_(-1),
	imgHeartLost_(-1),
	prevHp_(Player::MAX_HP)
{
	for (int i = 0; i < Player::MAX_HP; ++i)
	{
		animTimers_[i] = 0;
	}
}

// =============================================================================
// デストラクタ
// =============================================================================
HpManager::~HpManager(void)
{
}

// =============================================================================
// 初期化処理
// =============================================================================
void HpManager::Init(void)
{
	std::string path;

	// ハート画像のロード
	path = Application::PATH_IMAGE + "Heart.png";
	imgHeart_ = LoadGraph(path.c_str());

	path = Application::PATH_IMAGE + "HeartLost.png";
	imgHeartLost_ = LoadGraph(path.c_str());

	prevHp_ = player_ != nullptr ? player_->GetHp() : Player::MAX_HP;
	for (int i = 0; i < Player::MAX_HP; ++i)
	{
		animTimers_[i] = 0;
	}
}

// =============================================================================
// 毎フレーム更新処理（ダメージ被弾検知とアニメーションタイマー更新）
// =============================================================================
void HpManager::Update(void)
{
	if (player_ == nullptr) return;

	int currentHp = player_->GetHp();

	// ① HPが減った瞬間を検知
	if (currentHp < prevHp_)
	{
		// 減少したハートのアニメーションタイマーを開始（30フレーム＝約0.5秒）
		for (int i = currentHp; i < prevHp_; ++i)
		{
			if (i >= 0 && i < Player::MAX_HP)
			{
				animTimers_[i] = ANIM_DURATION;
			}
		}
	}
	prevHp_ = currentHp;

	// アニメーションタイマーのカウントダウン
	for (int i = 0; i < Player::MAX_HP; ++i)
	{
		if (animTimers_[i] > 0)
		{
			animTimers_[i]--;
		}
	}
}

// =============================================================================
// 描画処理（ポヨンと拡大＋ブルブル震えるシェイク演出）
// =============================================================================
void HpManager::Draw(void)
{
	if (player_ == nullptr) return;

	int maxHp = Player::MAX_HP;
	int playerHp = player_->GetHp();

	for (int i = 0; i < maxHp; i++)
	{
		int baseX = START_X + (HP_ICON_WIDTH * i);
		int baseY = START_Y;

		float scale = 1.0f;
		float rot = 0.0f;
		int offsetX = 0;
		int offsetY = 0;

		// 被弾アニメーション中のハートの演出計算
		if (animTimers_[i] > 0)
		{
			float ratio = static_cast<float>(animTimers_[i]) / static_cast<float>(ANIM_DURATION);

			// ② ポヨンと最大1.5倍に拡大！
			scale = 1.0f + (0.5f * ratio);

			// ③ sin関数による左右プルプル振動
			offsetX = static_cast<int>(sinf(animTimers_[i] * 1.5f) * (8.0f * ratio));

			// ふわりと上に少し跳ねる演出
			offsetY = static_cast<int>(-sinf(ratio * 3.14159f) * 14.0f);

			// 少し斜めに傾く回転演出
			rot = sinf(animTimers_[i] * 1.2f) * (0.2f * ratio);
		}

		int drawX = baseX + offsetX;
		int drawY = baseY + offsetY;

		// アニメーション中のハートは赤いハートで大きく跳ね、終了後に空ハートに変化！
		if (i < playerHp || animTimers_[i] > 0)
		{
			DrawRotaGraph(drawX, drawY, scale, rot, imgHeart_, TRUE);
		}
		else
		{
			DrawRotaGraph(drawX, drawY, scale, rot, imgHeartLost_, TRUE);
		}
	}
}

// =============================================================================
// 解放処理
// =============================================================================
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
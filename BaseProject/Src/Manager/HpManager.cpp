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
	prevHp_(Player::MAX_HP),
	imgWeaponPunch_(-1),
	imgWeaponAxe_(-1),
	imgWeaponDrill_(-1),
	prevWeaponType_(WeaponBase::TYPE::NONE),
	weaponAnimTimer_(0)
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

	// 1. ハート画像のロード
	path = Application::PATH_IMAGE + "Heart.png";
	imgHeart_ = LoadGraph(path.c_str());

	path = Application::PATH_IMAGE + "HeartLost.png";
	imgHeartLost_ = LoadGraph(path.c_str());

	prevHp_ = player_ != nullptr ? player_->GetHp() : Player::MAX_HP;
	for (int i = 0; i < Player::MAX_HP; ++i)
	{
		animTimers_[i] = 0;
	}

	// 2. 武器アイコン画像のロード（教材第39章）
	path = Application::PATH_IMAGE + "Punch.png";
	imgWeaponPunch_ = LoadGraph(path.c_str());

	path = Application::PATH_IMAGE + "Axe.png";
	imgWeaponAxe_ = LoadGraph(path.c_str());

	// ドリル用アイコン：DrillX.png の1コマ目（100x80）を読み込む
	int drillHandles[1];
	LoadDivGraph(
		(Application::PATH_IMAGE + "DrillX.png").c_str(),
		1, 1, 1, 100, 80, drillHandles
	);
	imgWeaponDrill_ = drillHandles[0];

	prevWeaponType_ = player_ != nullptr ? player_->GetUseWeaponType() : WeaponBase::TYPE::PUNCH;
	weaponAnimTimer_ = 0;
}

// =============================================================================
// 毎フレーム更新処理（HPダメージ演出 ＆ 武器切り替え演出更新）
// =============================================================================
void HpManager::Update(void)
{
	if (player_ == nullptr) return;

	// --- HPハート更新 ---
	int currentHp = player_->GetHp();
	if (currentHp < prevHp_)
	{
		for (int i = currentHp; i < prevHp_; ++i)
		{
			if (i >= 0 && i < Player::MAX_HP)
			{
				animTimers_[i] = ANIM_DURATION;
			}
		}
	}
	prevHp_ = currentHp;

	for (int i = 0; i < Player::MAX_HP; ++i)
	{
		if (animTimers_[i] > 0)
		{
			animTimers_[i]--;
		}
	}

	// --- 武器切り替え検知（クイズ第1問正解ロジック！） ---
	WeaponBase::TYPE currentWeaponType = player_->GetUseWeaponType();
	if (currentWeaponType != prevWeaponType_)
	{
		// 武器が切り替わった瞬間、ポップアニメーション開始！
		weaponAnimTimer_ = WEAPON_ANIM_DURATION;
		prevWeaponType_ = currentWeaponType;
	}

	if (weaponAnimTimer_ > 0)
	{
		weaponAnimTimer_--;
	}
}

// =============================================================================
// 描画処理（HPハート ＆ 武器アイコンUI）
// =============================================================================
void HpManager::Draw(void)
{
	if (player_ == nullptr) return;

	// -------------------------------------------------------------------------
	// 1. HPハートの描画
	// -------------------------------------------------------------------------
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

		if (animTimers_[i] > 0)
		{
			float ratio = static_cast<float>(animTimers_[i]) / static_cast<float>(ANIM_DURATION);
			scale = 1.0f + (0.5f * ratio);
			offsetX = static_cast<int>(sinf(animTimers_[i] * 1.5f) * (8.0f * ratio));
			offsetY = static_cast<int>(-sinf(ratio * 3.14159f) * 14.0f);
			rot = sinf(animTimers_[i] * 1.2f) * (0.2f * ratio);
		}

		int drawX = baseX + offsetX;
		int drawY = baseY + offsetY;

		if (i < playerHp || animTimers_[i] > 0)
		{
			DrawRotaGraph(drawX, drawY, scale, rot, imgHeart_, TRUE);
		}
		else
		{
			DrawRotaGraph(drawX, drawY, scale, rot, imgHeartLost_, TRUE);
		}
	}

	// -------------------------------------------------------------------------
	// 2. 武器アイコンUIの描画（教材第39章）
	// -------------------------------------------------------------------------
	WeaponBase::TYPE currentType = player_->GetUseWeaponType();
	int imgWeapon = -1;
	switch (currentType)
	{
	case WeaponBase::TYPE::PUNCH:
		imgWeapon = imgWeaponPunch_;
		break;
	case WeaponBase::TYPE::AXE:
		imgWeapon = imgWeaponAxe_;
		break;
	case WeaponBase::TYPE::DRILL:
		imgWeapon = imgWeaponDrill_;
		break;
	default:
		break;
	}

	if (imgWeapon != -1)
	{
		// 切り替え時のポヨン拡大アニメーション計算
		float weaponScale = 0.85f;
		if (weaponAnimTimer_ > 0)
		{
			float ratio = static_cast<float>(weaponAnimTimer_) / static_cast<float>(WEAPON_ANIM_DURATION);
			weaponScale += 0.35f * sinf(ratio * 3.14159f); // ポヨンと膨らんで戻る
		}

		// アイコンの背景枠を描画（半透明の黒い角丸四角形）
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
		DrawRoundRect(
			WEAPON_ICON_X - 45, WEAPON_ICON_Y - 42,
			WEAPON_ICON_X + 45, WEAPON_ICON_Y + 42,
			10, 10,
			0x111122, TRUE
		);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// 外枠線（金色の枠）
		DrawRoundRect(
			WEAPON_ICON_X - 45, WEAPON_ICON_Y - 42,
			WEAPON_ICON_X + 45, WEAPON_ICON_Y + 42,
			10, 10,
			0xddaa44, FALSE
		);

		// 武器アイコン本体の描画
		DrawRotaGraph(WEAPON_ICON_X, WEAPON_ICON_Y, weaponScale, 0.0f, imgWeapon, TRUE);
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

	if (imgWeaponPunch_ != -1)
	{
		DeleteGraph(imgWeaponPunch_);
		imgWeaponPunch_ = -1;
	}

	if (imgWeaponAxe_ != -1)
	{
		DeleteGraph(imgWeaponAxe_);
		imgWeaponAxe_ = -1;
	}

	if (imgWeaponDrill_ != -1)
	{
		DeleteGraph(imgWeaponDrill_);
		imgWeaponDrill_ = -1;
	}
}
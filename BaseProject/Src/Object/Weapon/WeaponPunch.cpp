#include "WeaponPunch.h"
#include "../../Utility/AsoUtility.h"

WeaponPunch::WeaponPunch(void) :
	type_(TYPE::NONE),
	animFrame_(0.0f),
	isUse_(false),
	pos_({ 0.0f, 0.0f, 0.0f }),
	dir_({ 0.0f, 0.0f, 0.0f })
{
	for (int i = 0; i < NUM_SPRITE_ALL; ++i)
	{
		imgHandles_[i] = -1;
	}
}

WeaponPunch::~WeaponPunch(void)
{
	for (int i = 0; i < NUM_SPRITE_ALL; ++i)
	{
		if (imgHandles_[i] != -1)
		{
			DeleteGraph(imgHandles_[i]);
			imgHandles_[i] = -1;
		}
	}
}

void WeaponPunch::Init(TYPE type)
{
	type_ = type;

	// パンチスプライトシート（横4コマ × 縦3コマ = 計12コマ）を分割読み込み
	LoadDivGraph(
		"Data/Image/Punch.png",
		NUM_SPRITE_ALL,
		NUM_SPRITE_X,
		NUM_SPRITE_Y,
		SIZE_XSPRITE_X,
		SIZE_XSPRITE_Y,
		imgHandles_
	);
}

void WeaponPunch::Update(void)
{
	if (!isUse_) return;

	// コマ送りを進める
	animFrame_ += ANIM_SPEED;

	// 全コマ再生が終わったら攻撃終了
	if (animFrame_ >= static_cast<float>(NUM_SPRITE_ALL))
	{
		animFrame_ = 0.0f;
		isUse_ = false;
	}
}

void WeaponPunch::Draw(void)
{
	if (!isUse_) return;

	int index = static_cast<int>(animFrame_);
	if (index >= 0 && index < NUM_SPRITE_ALL && imgHandles_[index] != -1)
	{
		// 3D空間のpos_位置に、カメラに正対するビルボードとして描画
		DrawBillboard3D(
			pos_,
			0.5f, 0.5f,                    // 画像の中心をアンカーにする（X:0.5, Y:0.5）
			static_cast<float>(SIZE_XSPRITE_X), // 表示幅
			0.0f,                          // 回転角
			imgHandles_[index],            // 画像ハンドル
			TRUE                           // 透過あり
		);
	}
}

void WeaponPunch::Use(VECTOR pos, VECTOR dir)
{
	// プレイヤーの前方少し離れた位置（腕を伸ばした先）にエフェクトを発生させる
	const float OFFSET_DIST = 40.0f;
	const float OFFSET_Y = 30.0f;

	pos_ = VAdd(pos, VScale(dir, OFFSET_DIST));
	pos_.y += OFFSET_Y; // 胸の高さ
	dir_ = dir;

	animFrame_ = 0.0f;
	isUse_ = true;
}
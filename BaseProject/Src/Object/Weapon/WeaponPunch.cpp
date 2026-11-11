#include <cmath>
#include "WeaponPunch.h"
#include "../../Utility/AsoUtility.h"

// =============================================================================
// コンストラクタ
// =============================================================================
WeaponPunch::WeaponPunch(void) :
	type_(TYPE::NONE),
	animFrame_(0.0f),
	isUse_(false),
	pos_({ 0.0f, 0.0f, 0.0f }),
	dir_({ 0.0f, 0.0f, 0.0f })
{
	for (int i = 0; i < NUM_SPRITE_ALL; ++i)
	{
		imgHandlesX_[i] = -1;
		imgHandlesY_[i] = -1;
	}
}

// =============================================================================
// デストラクタ（ロードした画像ハンドルのメモリ解放）
// =============================================================================
WeaponPunch::~WeaponPunch(void)
{
	for (int i = 0; i < NUM_SPRITE_ALL; ++i)
	{
		if (imgHandlesX_[i] != -1)
		{
			DeleteGraph(imgHandlesX_[i]);
			imgHandlesX_[i] = -1;
		}
		if (imgHandlesY_[i] != -1)
		{
			DeleteGraph(imgHandlesY_[i]);
			imgHandlesY_[i] = -1;
		}
	}
}

// =============================================================================
// 初期化処理（DrillX.png と DrillY.png の連番スプライト読み込み）
// =============================================================================
void WeaponPunch::Init(TYPE type)
{
	type_ = type;

	// 1. 横画像 (DrillX.png: 4コマ × 3コマ = 全12コマ)
	LoadDivGraph(
		"Data/Image/DrillX.png",
		NUM_SPRITE_ALL,
		NUM_SPRITE_X,
		NUM_SPRITE_Y,
		SIZE_XSPRITE_X,
		SIZE_XSPRITE_Y,
		imgHandlesX_
	);

	// 2. 縦画像 (DrillY.png: 4コマ × 3コマ = 全12コマ)
	LoadDivGraph(
		"Data/Image/DrillY.png",
		NUM_SPRITE_ALL,
		NUM_SPRITE_X,
		NUM_SPRITE_Y,
		SIZE_YSPRITE_X,
		SIZE_YSPRITE_Y,
		imgHandlesY_
	);
}

// =============================================================================
// 更新処理（前進移動 ＆ アニメーションコマ送り）
// =============================================================================
void WeaponPunch::Update(void)
{
	if (!isUse_) return;

	// 向いている方向へゆっくり衝撃波を前進させる
	pos_ = VAdd(pos_, VScale(dir_, FLY_SPEED));

	// コマ数を進める
	animFrame_ += ANIM_SPEED;

	// 全コマの再生が終了したら攻撃終了
	if (animFrame_ >= static_cast<float>(NUM_SPRITE_ALL))
	{
		animFrame_ = 0.0f;
		isUse_ = false;
	}
}

// =============================================================================
// 描画処理（プレイヤーの向きに応じたDrillX/DrillY選択 ＆ 反転ビルボード描画）
// =============================================================================
void WeaponPunch::Draw(void)
{
	if (!isUse_) return;

	int index = static_cast<int>(animFrame_);
	if (index < 0 || index >= NUM_SPRITE_ALL) return;

	bool isXRev = false;
	bool isYRev = false;
	int img = -1;

	// 【第1問正解！】左右向きか、手前／奥向きかを fabsf(dir_.x) と fabsf(dir_.z) で比較
	if (fabsf(dir_.x) > fabsf(dir_.z))
	{
		// 左右向き：横画像 DrillX.png を使用
		img = imgHandlesX_[index];

		// 右向きの場合はX反転（左右反転）
		if (dir_.x > 0.0f)
		{
			isXRev = true;
		}
	}
	else
	{
		// 手前／奥向き：縦画像 DrillY.png を使用
		img = imgHandlesY_[index];

		// 【第2問正解！】手前向き（dir_.z < 0.0f）の場合はY反転（上下反転）
		if (dir_.z < 0.0f)
		{
			isYRev = true;
		}
	}

	if (img != -1)
	{
		// 3D空間にビルボード描画（X反転・Y反転フラグを適用）
		DrawBillboard3D(
			pos_,
			0.5f, 0.5f,                         // 描画中心（中央揃え）
			100.0f,                             // 表示サイズ
			0.0f,                               // 回転角
			img,                                // 画像ハンドル
			TRUE,                               // 透過あり
			isXRev ? TRUE : FALSE,              // 左右反転フラグ
			isYRev ? TRUE : FALSE               // 上下反転フラグ
		);
	}

#ifdef _DEBUG
	// デバッグ用：当たり判定球体の描画
	DrawSphere3D(pos_, COLLISION_RADIUS, 10, 0x0000ff, 0x0000ff, false);
#endif
}

// =============================================================================
// 武器使用（攻撃開始）
// プレイヤーの少し前方、胸の高さから前方に発射する
// =============================================================================
void WeaponPunch::Use(VECTOR pos, VECTOR dir)
{
	const float OFFSET_DIST = 40.0f; // プレイヤー中心からの前方オフセット距離
	const float OFFSET_Y = 35.0f;    // 高さオフセット

	pos_ = VAdd(pos, VScale(dir, OFFSET_DIST));
	pos_.y += OFFSET_Y;
	dir_ = dir;

	animFrame_ = 0.0f;
	isUse_ = true;
}
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
		imgHandles_[i] = -1;
	}
}

// =============================================================================
// デストラクタ（ロードした画像ハンドルの解放）
// =============================================================================
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

// =============================================================================
// 初期化処理（連番スプライト画像のロード）
// =============================================================================
void WeaponPunch::Init(TYPE type)
{
	type_ = type;

	// パンチスプライトシート（横4コマ × 縦3コマ = 全12コマ）を分割読み込み
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

// =============================================================================
// 更新処理（コマ送りアニメーション制御）
// =============================================================================
void WeaponPunch::Update(void)
{
	if (!isUse_) return;

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
// 描画処理（ビルボード3Dスプライト）
// =============================================================================
void WeaponPunch::Draw(void)
{
	if (!isUse_) return;

	int index = static_cast<int>(animFrame_);
	if (index >= 0 && index < NUM_SPRITE_ALL && imgHandles_[index] != -1)
	{
		// 3D空間のpos_位置に、カメラを常に向くビルボードとして描画
		DrawBillboard3D(
			pos_,
			0.5f, 0.5f,                         // 描画中心アンカー（X:0.5, Y:0.5で中央揃え）
			static_cast<float>(SIZE_XSPRITE_X), // 表示サイズ
			0.0f,                               // 回転角
			imgHandles_[index],                 // 画像ハンドル
			TRUE                                // 透過あり
		);
	}
}

// =============================================================================
// 武器使用（攻撃開始）
// プレイヤーの少し前方、腰の高さにエフェクトを発生させる
// =============================================================================
void WeaponPunch::Use(VECTOR pos, VECTOR dir)
{
	const float OFFSET_DIST = 40.0f; // プレイヤー中心からの前方オフセット距離
	const float OFFSET_Y = 30.0f;    // 高さオフセット

	pos_ = VAdd(pos, VScale(dir, OFFSET_DIST));
	pos_.y += OFFSET_Y;
	dir_ = dir;

	animFrame_ = 0.0f;
	isUse_ = true;
}

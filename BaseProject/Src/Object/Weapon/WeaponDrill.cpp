#include "WeaponDrill.h"
#include "../../Utility/AsoUtility.h"

// =============================================================================
// コンストラクタ
// =============================================================================
WeaponDrill::WeaponDrill(void) :
	type_(TYPE::NONE),
	animFrame_(0.0f),
	isUse_(false),
	lifeTimer_(0),
	pos_({ 0.0f, 0.0f, 0.0f }),
	dir_({ 0.0f, 0.0f, 1.0f })
{
	for (int i = 0; i < NUM_SPRITE_ALL; ++i)
	{
		imgHandles_[i] = -1;
	}
}

// =============================================================================
// デストラクタ（ロードした画像ハンドルの解放）
// =============================================================================
WeaponDrill::~WeaponDrill(void)
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
void WeaponDrill::Init(TYPE type)
{
	type_ = type;

	// ドリルスプライトシート（横4コマ × 縦3コマ = 全12コマ）を分割読み込み
	LoadDivGraph(
		"Data/Image/DrillX.png",
		NUM_SPRITE_ALL,
		NUM_SPRITE_X,
		NUM_SPRITE_Y,
		SIZE_SPRITE_X,
		SIZE_SPRITE_Y,
		imgHandles_
	);

	animFrame_ = 0.0f;
	isUse_ = false;
	lifeTimer_ = 0;
}

// =============================================================================
// 更新処理（直線ロケット前進＋高速回転アニメーション）
// =============================================================================
void WeaponDrill::Update(void)
{
	if (!isUse_) return;

	// 1. 直線前進移動（速度 FLY_SPEED を向いている方向へ加算）
	pos_ = VAdd(pos_, VScale(dir_, FLY_SPEED));

	// 2. 超高速コマ送り（激しく回転する演出）
	animFrame_ += ANIM_SPEED;
	if (animFrame_ >= static_cast<float>(NUM_SPRITE_ALL))
	{
		animFrame_ = 0.0f; // ループしてギュルルルと回転し続ける
	}

	// 3. 飛翔タイマー減少（時間経過で消滅）
	lifeTimer_--;
	if (lifeTimer_ <= 0)
	{
		isUse_ = false;
	}
}

// =============================================================================
// 描画処理（ビルボード3Dスプライト）
// =============================================================================
void WeaponDrill::Draw(void)
{
	if (!isUse_) return;

	int index = static_cast<int>(animFrame_);
	if (index >= 0 && index < NUM_SPRITE_ALL && imgHandles_[index] != -1)
	{
		// 3D空間のpos_位置に、カメラを常に向くビルボードとして描画
		DrawBillboard3D(
			pos_,
			0.5f, 0.5f,                         // 描画中心アンカー（中央揃え）
			static_cast<float>(SIZE_SPRITE_X), // 表示サイズ
			0.0f,                               // 回転角
			imgHandles_[index],                 // 画像ハンドル
			TRUE                                // 透過あり
		);
	}

#ifdef _DEBUG
	// デバッグ用：当たり判定球体の描画
	DrawSphere3D(pos_, COLLISION_RADIUS, 10, 0x00ffff, 0x00ffff, false);
#endif
}

// =============================================================================
// 武器使用（攻撃開始）
// プレイヤーの少し前方、胸の高さから前方に発射する
// =============================================================================
void WeaponDrill::Use(VECTOR pos, VECTOR dir)
{
	const float OFFSET_DIST = 50.0f; // プレイヤー中心からの前方オフセット距離
	const float OFFSET_Y = 40.0f;    // 高さオフセット（胸の高さ）

	pos_ = VAdd(pos, VScale(dir, OFFSET_DIST));
	pos_.y += OFFSET_Y;
	dir_ = dir;

	animFrame_ = 0.0f;
	lifeTimer_ = LIFE_TIME;
	isUse_ = true;
}
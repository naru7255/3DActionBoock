#pragma once
#include "SceneBase.h"
class SceneManager;

// =============================================================================
// タイトルシーンクラス（TitleScene）
// タイトル画面の画像描画と、スペースキー入力によるゲーム開始を管理
// =============================================================================
class TitleScene : public SceneBase
{
public:
	TitleScene(void);
	virtual ~TitleScene(void);

	// シーン基本関数
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:
	int imgTitle_; // タイトルロゴ・背景画像のハンドルID
};

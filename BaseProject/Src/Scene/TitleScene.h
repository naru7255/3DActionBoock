#pragma once
#include "SceneBase.h"
class SceneManager;

// タイトル画像を表示し、開始入力を受け付けるシーン
class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void);

   // タイトル画像を読み込み、表示・入力処理を行い、終了時に画像を解放する
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:

	// 画像
	int imgTitle_;

};

#pragma once
#include <string>
#include <map>

class AnimationController
{
public:
	// アニメーションデータ
	struct Animation
	{
		int model = -1;       // 外部モデルハンドル（同FBX内は -1）
		int attachNo = -1;    // アタッチ番号
		int animIndex = 0;    // アニメーションインデックス
		float speed = 0.0f;   // 再生速度
		float totalTime = 0.0f; // 総再生時間
		float step = 0.0f;    // 現在の再生時間
	};

	// コンストラクタ / デストラクタ
	AnimationController(int modelId);
	~AnimationController(void);

	// 同じFBX内のアニメーションを準備
	void AddInFbx(int type, float speed, int animIndex);

	// 外部FBXからアニメーション追加（資料14の⑥で追加する機能）
	void Add(int type, float speed, const std::string path);

	// アニメーション再生（ループフラグ対応）
	void Play(int type, bool isLoop = true);

	// 更新処理
	void Update(void);

	// リソース解放
	void Release(void);

	// 再生終了判定
	bool IsEnd(void) const;

	// 再生中のアニメーション種別を取得
	int GetPlayType(void) const;

private:
	// アニメーション追加の共通処理
	void Add(int type, float speed, Animation& animation);

	int modelId_;                           // 対象モデルのハンドルID
	std::map<int, Animation> animations_;   // 種類別アニメーションデータ
	int playType_;                          // 再生中のアニメーション種別
	Animation playAnim_;                    // 再生中のアニメーションデータ
	bool isLoop_;                           // ループ再生フラグ
};
#pragma once
#include <chrono>
class SceneBase;
class Fader;
class Camera;

// 現在のシーンとカメラを管理し、シーン遷移を制御する
class SceneManager
{

public:

   // アプリケーションで使用するシーンの識別子
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		GAME,
		
	};
	
	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

  // カメラと初期シーンを準備する
	void Init(void);

   // シーンとフェードを更新する
	void Update(void);

   // カメラ設定後にシーンとフェードを描画する
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

 // 指定したシーンへの遷移を開始する
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

    // 前回の更新から経過した時間（秒）を取得する
	float GetDeltaTime(void) const;
  // DxLibの3D描画に必要な初期設定を行う
	void Init3D(void);
    // シーンで共有するカメラを取得する
	Camera* GetCamera(void) { return camera_; }

private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// フェード
	Fader* fader_;

	// カメラ
	Camera* camera_;

	// 各種シーン
	SceneBase* scene_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;
	
	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);

	// コピーコンストラクタも同様
	SceneManager(const SceneManager& instance) = default;

	// デストラクタも同様
	~SceneManager(void) = default;

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	// フェード
	void Fade(void);
	


};
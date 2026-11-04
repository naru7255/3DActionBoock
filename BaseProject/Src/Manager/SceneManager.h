#pragma once
#include <chrono>
class SceneBase;
class Fader;
class Camera;

// =============================================================================
// シーン管理クラス（SceneManager：シングルトン）
// タイトル、ゲームプレイ等のシーン遷移、フェード、カメラ、デルタタイムを統括
// =============================================================================
class SceneManager
{
public:
	// シーン識別子
	enum class SCENE_ID
	{
		NONE,
		TITLE, // タイトル画面
		GAME,  // ゲーム本編
	};

	// インスタンス生成と取得
	static void CreateInstance(void);
	static SceneManager& GetInstance(void);

	// シーンの初期化・更新・描画・破棄
	void Init(void);
	void Update(void);
	void Draw(void);
	void Destroy(void);

	// シーン切り替え要求（フェードアウト後に切り替わる）
	void ChangeScene(SCENE_ID nextId);

	// 現在のシーンIDを取得
	SCENE_ID GetSceneID(void);

	// 前フレームからの経過時間（秒単位）を取得
	float GetDeltaTime(void) const;

	// 3D描画環境の初期化
	void Init3D(void);

	// 追従カメラのポインタを取得
	Camera* GetCamera(void) { return camera_; }

private:
	// シングルトンインスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	Fader* fader_;   // 画面フェード制御
	Camera* camera_; // 3Dカメラ
	SceneBase* scene_; // 現在実行中のシーン

	bool isSceneChanging_; // シーン遷移中フラグ

	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	// コンストラクタ / デストラクタはシングルトンのためprivate
	SceneManager(void);
	SceneManager(const SceneManager& instance) = default;
	~SceneManager(void) = default;

	void ResetDeltaTime(void);
	void DoChangeScene(SCENE_ID sceneId);
	void Fade(void);
};

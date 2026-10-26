#include <DxLib.h>
#include "../../Manager/SceneManager.h"
#include "AnimationController.h"

AnimationController::AnimationController(int modelId)
{
 // 再生対象のモデルを保存し、未再生状態にする。
	modelId_ = modelId;
	playType_ = -1;
	isLoop_ = true;
}

AnimationController::~AnimationController(void)
{
}

// 外部FBXからのアニメーション追加
void AnimationController::Add(int type, float speed, const std::string path)
{
	Animation animation;
	animation.model = MV1LoadModel(path.c_str());
	animation.animIndex = -1;
	Add(type, speed, animation);
}

// 同じFBX内のアニメーション追加
void AnimationController::AddInFbx(int type, float speed, int animIndex)
{
	Animation animation;
	animation.model = -1;
	animation.animIndex = animIndex;
	Add(type, speed, animation);
}

// 共通の追加処理
void AnimationController::Add(int type, float speed, Animation& animation)
{
	animation.speed = speed;
	if (animations_.count(type) == 0)
	{
		animations_.emplace(type, animation);
	}
}

void AnimationController::Play(int type, bool isLoop)
{
   // 不正なモデルや未登録の種類では再生しない。
  if (modelId_ == -1)
	{
		return;
	}

	auto animationIt = animations_.find(type);
	if (animationIt == animations_.end())
	{
		return;
	}

	if (playType_ == type && isLoop_ == isLoop)
	{
     // 同じ設定で再生中なら、毎フレーム再スタートしない。
		return;
	}

    // 切り替え前のアニメーションをモデルから外す。
	if (playType_ != -1 && playAnim_.attachNo != -1)
	{
		MV1DetachAnim(modelId_, playAnim_.attachNo);
		playAnim_.attachNo = -1;
	}

	playType_ = type;
  playAnim_ = animationIt->second;
	isLoop_ = isLoop;
	playAnim_.step = 0.0f;

	// 同一FBX内か外部モデルかによってアタッチ方法を切り替える
	if (playAnim_.model == -1)
	{
		playAnim_.attachNo = MV1AttachAnim(modelId_, playAnim_.animIndex);
	}
	else
	{
		// 外部モデルからアタッチする場合
		int animIdx = 0; // 必要に応じて調整
		playAnim_.attachNo = MV1AttachAnim(modelId_, animIdx, playAnim_.model);
	}

   if (playAnim_.attachNo == -1)
	{
     // アタッチに失敗した場合は未再生状態に戻す。
		playType_ = -1;
       return;
	}

	playAnim_.totalTime = MV1GetAttachAnimTotalTime(modelId_, playAnim_.attachNo);
}

void AnimationController::Update(void)
{
	if (playType_ == -1 || playAnim_.attachNo == -1) return;

   // 経過時間に応じて再生位置を進め、ループまたは最終フレームで停止する。
	float deltaTime = SceneManager::GetInstance().GetDeltaTime();
	playAnim_.step += (deltaTime * playAnim_.speed);

	if (playAnim_.step >= playAnim_.totalTime)
	{
		if (isLoop_)
		{
			while (playAnim_.step >= playAnim_.totalTime && playAnim_.totalTime > 0.0f)
			{
				playAnim_.step -= playAnim_.totalTime;
			}
		}
		else
		{
			playAnim_.step = playAnim_.totalTime;
		}
	}

	MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);
}

void AnimationController::Release(void)
{
  // このコントローラーがロードした外部アニメーションモデルを解放する。
	// 外部FBXモデルの解放
	for (const auto& pair : animations_)
	{
		if (pair.second.model != -1)
		{
			MV1DeleteModel(pair.second.model);
		}
	}
	animations_.clear();
}

bool AnimationController::IsEnd(void) const
{
 // ループ再生中、または有効なアニメーションがない場合は終了扱いにしない。
    if (isLoop_ || playType_ == -1 || playAnim_.attachNo == -1)
	{
		return false;
	}
	return playAnim_.step >= playAnim_.totalTime;
}

int AnimationController::GetPlayType(void) const
{
	return playType_;
}
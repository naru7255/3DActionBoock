#pragma once
#include <DxLib.h>

class Block
{
public:
	// ブロックモデルの自己発光色
	static constexpr COLOR_F COLOR_EMI = { 0.5f, 0.5f, 0.5f, 1.0f };
	// ブロックモデルの大きさ
	static constexpr VECTOR SCALES = { 0.5f, 0.5f, 0.5f };

	// ブロック種別
	enum class TYPE
	{
		NONE = -1,
		GRASS,
		METAL,
		ICE
	};

	Block(void);
	~Block(void);

	// ブロックの生成
	void Create(TYPE type, int baseModelId, int mapX, int mapZ);
	void Update(void);
	void Draw(void);
	void Release(void);



	// モデルのハンドルIDを取得
	int GetModelId(void);
	VECTOR GetPos(void) const { return pos_; }

private:
	TYPE type_;
	int modelId_;
	VECTOR pos_;
	VECTOR scales_;
};
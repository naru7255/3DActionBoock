#pragma once
#include <DxLib.h>

class Grid
{
public:
	// ü‚Ì’·‚³
	static constexpr float LEN = 3000.0f;
	// ü‚Ì’·‚³‚Ì”¼•ª
	static constexpr float HLEN = LEN / 2.0f;
	// ü‚ÌŠÔŠu
	static constexpr float TERM = 100.0f;
	// ü‚Ì”
	static const int NUM = static_cast<int>(LEN / TERM);
	// ü‚Ì”‚Ì”¼•ª
	static const int HNUM = NUM / 2;

	Grid(void);
	~Grid(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);
};
#pragma once

const int NUM_KEYS = 256;

class TheApp
{
public:
	TheApp();
	TheApp(float width, float height);
	void AdvanceFrameNumber();
	void SetKeyDownStatus(unsigned char keyCode, bool isDown);
	bool IsKeyDown(unsigned char keyCode);
	bool WasKeyJustPressed(unsigned char keyCode);

	void SetWindowWidth(float width);
	void SetWindowHeight(float height);
	float GetWindowWidth();
	float GetWindowHeight();

	static TheApp* instance;

private:
	bool m_isKeyDown[NUM_KEYS];
	int m_frameNumberKeyLastChanged[NUM_KEYS];
	int m_frameCounter;
	float m_windowWidth;
	float m_windowHeight;
};

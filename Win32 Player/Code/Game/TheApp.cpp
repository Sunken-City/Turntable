#include "Game/TheApp.hpp"

TheApp* TheApp::instance = nullptr;

bool TheApp::IsKeyDown(unsigned char keyCode)
{
	return m_isKeyDown[keyCode];
}

bool TheApp::WasKeyJustPressed(unsigned char keyCode)
{
	return (m_isKeyDown[keyCode] && (m_frameNumberKeyLastChanged[keyCode] == m_frameCounter));
}

void TheApp::SetWindowWidth(float width)
{
	m_windowWidth = width;
}

void TheApp::SetWindowHeight(float height)
{
	m_windowHeight = height;
}

float TheApp::GetWindowWidth()
{
	return m_windowWidth;
}

float TheApp::GetWindowHeight()
{
	return m_windowHeight;
}

TheApp::TheApp() : m_frameCounter(0)
{
	//Initialize all keys to up
	for (int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex)
	{
		m_isKeyDown[keyIndex] = false;
		m_frameNumberKeyLastChanged[keyIndex] = m_frameCounter;
	}
}

TheApp::TheApp(float width, float height) : m_windowWidth(width), m_windowHeight(height) , m_frameCounter(0)
{
	//Initialize all keys to up
	for (int keyIndex = 0; keyIndex < NUM_KEYS; ++keyIndex)
	{
		m_isKeyDown[keyIndex] = false;
		m_frameNumberKeyLastChanged[keyIndex] = m_frameCounter;
	}
}

void TheApp::AdvanceFrameNumber()
{
	m_frameCounter++;
}

void TheApp::SetKeyDownStatus(unsigned char keyCode, bool isNowDown)
{
	//If we are getting a keyboard repeat, ignore it when updating "just pressed" values.
	if (m_isKeyDown[keyCode] != isNowDown)
	{
		m_frameNumberKeyLastChanged[keyCode] = m_frameCounter;
	}
	m_isKeyDown[keyCode] = isNowDown;
}

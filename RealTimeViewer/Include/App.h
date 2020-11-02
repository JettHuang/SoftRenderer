#pragma once

#include <cstdio>
#define _USE_MATH_DEFINES
#include <cmath>

#include "SDL.h"
#include "SR_Headers.h"
#include "DemoScene.h"


class FApp
{
public:
	FApp()
		: _SDLWindow(NULL)
		, _SDLRenderer(NULL)
		, _SDLRenderTexture(NULL)
		, _Width(1024u)
		, _Height(768u)
		, _Camera()
		, _Keydown_W(false)
		, _Keydown_S(false)
		, _Keydown_A(false)
		, _Keydown_D(false)
		, _bMousePressed(false)
	{}

	bool Initialize(const char* InCaption, int32_t InWidth, int32_t InHeight);
	void Uninitialize();

	void MainLoop();
	void Tick(float InDeltaSeconds);
protected:
	// process sdl event
	void ProcessEvent(const SDL_Event& InEvent);

	// Window Message
	void OnKeyDown(const SDL_Event& InEvent);
	void OnKeyUp(const SDL_Event& InEvent);
	void OnMouseButtonDown(const SDL_Event& InEvent);
	void OnMouseButtonUp(const SDL_Event& InEvent);
	void OnMouseWheel(const SDL_Event& InEvent);
	void OnMouseMove(const SDL_Event& InEvent);
	void OnWndClosed();

	// canvas operations
	void SwapChain(const FSR_Buffer2D& InBuffer2D);
	void Present();

protected:
	SDL_Window* _SDLWindow;
	SDL_Renderer* _SDLRenderer;
	SDL_Texture* _SDLRenderTexture;
	uint32_t _Width;
	uint32_t _Height;

	FSR_Context	_SR_Ctx;
	std::shared_ptr<FDemoScene> _DemoScene;
	FCamera		_Camera;

	bool _Keydown_W;
	bool _Keydown_S;
	bool _Keydown_A;
	bool _Keydown_D;

	bool _bMousePressed;
};

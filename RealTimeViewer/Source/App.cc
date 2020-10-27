// App.cc
//

#include "App.h"


bool FApp::Initialize(const char* InCaption, int32_t InWidth, int32_t InHeight)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "Could not initialize SDL.Error: " << SDL_GetError() << std::endl;
		return false;
	}

	_Width = InWidth;
	_Height = InHeight;
	_SDLWindow = SDL_CreateWindow(InCaption, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _Width, _Height, SDL_WINDOW_SHOWN);
	_SDLRenderer = SDL_CreateRenderer(_SDLWindow, -1, SDL_RENDERER_ACCELERATED);

	SDL_RendererInfo info;
	SDL_GetRendererInfo(_SDLRenderer, &info);
	printf("Renderer name: %s\n", info.name);
	printf("Texture formats: \n");
	for (uint32_t i = 0; i < info.num_texture_formats; i++)
	{
		printf("\t%s\n", SDL_GetPixelFormatName(info.texture_formats[i]));
	}

	_SDLRenderTexture = SDL_CreateTexture(_SDLRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, _Width, _Height);
	_ColorBuffer.resize(_Width * _Height * 4);

	_DemoScene = std::make_shared<FDemoScene_Cubes>();
	return true;
}

void FApp::Uninitialize()
{
	SDL_DestroyTexture(_SDLRenderTexture);
	SDL_DestroyRenderer(_SDLRenderer);
	SDL_DestroyWindow(_SDLWindow);
	SDL_Quit();
}

void FApp::ProcessEvent(const SDL_Event& InEvent)
{
	switch (InEvent.type)
	{
	case SDL_KEYDOWN:
		OnKeyDown(InEvent);
		break;
	case SDL_KEYUP:
		OnKeyUp(InEvent);
		break;

	case SDL_MOUSEBUTTONDOWN:
		OnMouseButtonDown(InEvent);
		break;
	case SDL_MOUSEBUTTONUP:
		OnMouseButtonUp(InEvent);
		break;

	case SDL_MOUSEWHEEL:
		OnMouseWheel(InEvent);
		break;

	case SDL_MOUSEMOTION:
		OnMouseMove(InEvent);
		break;
	case SDL_WINDOWEVENT:
	{
		switch (InEvent.window.event)
		{
		case SDL_WINDOWEVENT_CLOSE:
			OnWndClosed();
			break;
		default:
			break;
		}
	}
	break;
	default:
		break;
	}
}
	
// Window Message
void FApp::OnKeyDown(const SDL_Event& InEvent)
{

}

void FApp::OnKeyUp(const SDL_Event& InEvent)
{

}

void FApp::OnMouseButtonDown(const SDL_Event& InEvent)
{

}

void FApp::OnMouseButtonUp(const SDL_Event& InEvent)
{

}
	
void FApp::OnMouseWheel(const SDL_Event& InEvent)
{

}

void FApp::OnMouseMove(const SDL_Event& InEvent)
{

}

void FApp::OnWndClosed()
{
	SDL_Event event;

	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
}

void FApp::ClearCanvas()
{
	SDL_SetRenderDrawColor(_SDLRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(_SDLRenderer);
}

void FApp::Present()
{
	SDL_UpdateTexture(_SDLRenderTexture, nullptr, _ColorBuffer.data(), _Width * 4);

	SDL_RenderCopy(_SDLRenderer, _SDLRenderTexture, nullptr, nullptr);
	SDL_RenderPresent(_SDLRenderer);
}
////////////////////////////////////////////////////////

void FApp::MainLoop()
{
	bool bRequestQuit = false;

	Uint32 LastTicks = SDL_GetTicks();
	while (!bRequestQuit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			/* handle your event here */
			switch (event.type)
			{
			case SDL_QUIT:
				bRequestQuit = true;
				break;
			}

			ProcessEvent(event);
		} // end while

		Uint32 CurTicks = SDL_GetTicks();
		float ElapseSeconds = (CurTicks + 1 - LastTicks) / 1000.f;
		LastTicks = CurTicks;

		Tick(ElapseSeconds);
	} // end while
}

void FApp::Tick(float InDeltaSeconds)
{
	ClearCanvas();

	Present();
}

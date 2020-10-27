// App.cc
//

#include "App.h"
#include "svpng.inc"


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
	if (_DemoScene)
	{
		_DemoScene->Init();
	}

	_SR_Ctx.SetRenderTarget(_Width, _Height, 1, false);
	_SR_Ctx.SetViewport(0, 0, _Width, _Height);
	_SR_Ctx.SetCullFaceMode(EFrontFace::FACE_CCW);

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

void FApp::SwapChain(const std::shared_ptr<FSR_Buffer2D>& InBuffer2D)
{
	uint32_t image_width = InBuffer2D->Width();
	uint32_t image_height = InBuffer2D->Height();
	
	assert(image_width == _Width && image_height == _Height);

	uint8_t *ptr = _ColorBuffer.data();
	for (int32_t j = image_height - 1; j >= 0; --j)
	{
		for (int32_t i = 0; i < image_width; ++i)
		{
			float R, G, B, A;
			InBuffer2D->Read(i, j, R, G, B, A);

			// Write the translated [0,255] value of each color component.
			*ptr++ = static_cast<int>(255 * R);
			*ptr++ = static_cast<int>(255 * G);
			*ptr++ = static_cast<int>(255 * B);
			*ptr++ = 255;
		}
	} // end j
}

////////////////////////////////////////////////////////
void FApp::MainLoop()
{
	char szCaption[256];

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

		sprintf(szCaption, "RealTimeViewer  fps:%.2f", 1.f / ElapseSeconds);
		SDL_SetWindowTitle(_SDLWindow, szCaption);
	} // end while
}

void FApp::Tick(float InDeltaSeconds)
{
	_SR_Ctx.BeginFrame();

	_SR_Ctx.ClearRenderTarget(glm::vec4(0, 0, 0, 0));
	if (_DemoScene)
	{
		// Build view & projection matrices (right-handed system)
		const float nearPlane = 0.1f;
		const float farPlane = 100.f;
		const glm::vec3 eye(0, 3.75, 6.5);
		const glm::vec3 lookat(0, 0, 0);
		const glm::vec3 up(0, 1, 0);

		const glm::mat4 view = glm::lookAt(eye, lookat, up);
		const glm::mat4 proj = glm::perspective(glm::radians(60.f), static_cast<float>(_Width) / static_cast<float>(_Height), nearPlane, farPlane);
		_SR_Ctx.SetProjectionMatrix(proj);

		_DemoScene->DrawScene(_SR_Ctx, view, InDeltaSeconds);
	}
	_SR_Ctx.EndFrame();

	// update color buffer
	const std::shared_ptr<FSR_Buffer2D>& Buffer2d = _SR_Ctx.GetColorBuffer(0);
	assert(Buffer2d);

	SwapChain(Buffer2d);
	Present();
}


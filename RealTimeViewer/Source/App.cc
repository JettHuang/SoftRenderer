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

	_SDLRenderTexture = SDL_CreateTexture(_SDLRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, _Width, _Height);

	_DemoScene = std::make_shared<FDemoScene_Cubes>();
	if (_DemoScene)
	{
		_DemoScene->Init();
	}

	_SR_Ctx.SetRenderTarget(_Width, _Height, 1, false);
	_SR_Ctx.SetViewport(0, 0, _Width, _Height);
	_SR_Ctx.SetCullFaceMode(EFrontFace::FACE_CCW);

	// TO REMOVE FROM HERE
	// Build view & projection matrices (right-handed system)
	const float nearPlane = 0.1f;
	const float farPlane = 100.f;
	const glm::vec3 eye(0, 3.75, 6.5);
	const glm::vec3 lookat(0, 0, 0);
	const glm::vec3 up(0, 1, 0);

	const glm::mat4 proj = glm::perspective(glm::radians(60.f), static_cast<float>(_Width) / static_cast<float>(_Height), nearPlane, farPlane);
	_viewMat = glm::lookAt(eye, lookat, up);
	_SR_Ctx.SetProjectionMatrix(proj);

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

void FApp::Present()
{
	SDL_RenderCopy(_SDLRenderer, _SDLRenderTexture, nullptr, nullptr);
	SDL_RenderPresent(_SDLRenderer);
}

void FApp::SwapChain(const FSR_Buffer2D& InBuffer2D)
{
	const uint32_t image_width = InBuffer2D.Width();
	const uint32_t image_height = InBuffer2D.Height();
	const uint32_t bytes_per_row = InBuffer2D.BytesPerLine();

	assert(image_width == _Width && image_height == _Height);
	assert(InBuffer2D.Format() == EPixelFormat::PIXEL_FORMAT_RGBA8888);
	
	uint8_t* pBuffer = NULL;
	int32_t pitch = 0;
	SDL_LockTexture(_SDLRenderTexture, NULL, (void**)&pBuffer, &pitch);
	assert(bytes_per_row <= pitch);

	if (bytes_per_row == pitch)
	{
		memcpy(pBuffer, InBuffer2D.Data(), InBuffer2D.Length());
	}
	else
	{
		for (int32_t j = image_height - 1; j >= 0; --j, pBuffer += pitch)
		{
			const uint8_t* src = InBuffer2D.GetRowData(j);
			uint8_t* dst = pBuffer;

			memcpy(dst, src, bytes_per_row);
		} // end j
	}


	SDL_UnlockTexture(_SDLRenderTexture);
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
		_DemoScene->DrawScene(_SR_Ctx, _viewMat, InDeltaSeconds);
	}
	_SR_Ctx.EndFrame();

	// update color buffer
	const std::shared_ptr<FSR_Buffer2D>& Buffer2d = _SR_Ctx.GetColorBuffer(0);
	assert(Buffer2d);

	SwapChain(*Buffer2d);
	Present();
}


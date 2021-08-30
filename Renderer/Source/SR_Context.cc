// \brief
//		soft renderer context
//

#include "SR_Context.h"
#include "SR_Renderer.h"
#include "SR_SSE.h"


struct FPixelFormatDesc 
{
	EPixelFormat	_format;
	uint32_t		_bytes_per_pixel;
};

static const FPixelFormatDesc sPixelFormatTable[] = 
{
	{ EPixelFormat::PIXEL_FORMAT_U16,		 2},
	{ EPixelFormat::PIXEL_FORMAT_F32,		 4},
	{ EPixelFormat::PIXEL_FORMAT_RGB888,     3},
	{ EPixelFormat::PIXEL_FORMAT_RGBA8888,   4},
	{ EPixelFormat::PIXEL_FORMAT_RGBF32,     3*4},
	{ EPixelFormat::PIXEL_FORMAT_RGBAF32,    4*4},
};

uint32_t LookupPixelFormatBytes(EPixelFormat InFormat)
{
	uint32_t idx = static_cast<uint32_t>(InFormat);
	const FPixelFormatDesc& desc = sPixelFormatTable[idx];

	assert(desc._format == InFormat);
	return desc._bytes_per_pixel;
}

FSR_Context::FSR_Context()
	: _bEnableMultiThreads(false)
	, _viewport_rect(0, 0, 1, 1)
	, _front_face(EFrontFace::FACE_CW)
	, _bEnableMSAA(false)
	, _MSAASamplesNum(MSAA_SAMPLES)
{
	memset(&_pointers_shadow, 0, sizeof(_pointers_shadow));

	_stats = std::make_shared<FSR_Performance>();
	UpdateMVP();
}

FSR_Context::~FSR_Context()
{
}

void FSR_Context::EnableMultiThreads()
{
	_bEnableMultiThreads = FSR_Renderer::EnableMultiThreads();
}

// set render target
void FSR_Context::SetRenderTarget(uint32_t w, uint32_t h, uint32_t nCount, bool InbEnableMSAA)
{
	_rt_depth = FSR_Buffer2D_Helper::CreateBuffer2D(w, h, EPixelFormat::PIXEL_FORMAT_F32);
	_pointers_shadow._rt_depth = _rt_depth.get();

	nCount = std::min<uint32_t>(nCount, MAX_MRT_COUNT);
	for (uint32_t i=0; i<nCount; ++i)
	{
		_rt_colors[i] = FSR_Buffer2D_Helper::CreateBuffer2D(w, h, EPixelFormat::PIXEL_FORMAT_RGBA8888);
		_pointers_shadow._rt_colors[i] = _rt_colors[i].get();
	}

	_bEnableMSAA = InbEnableMSAA;
	if (_bEnableMSAA)
	{
		_rt_depth_msaa = FSR_Buffer2D_Helper::CreateBuffer2D(w * _MSAASamplesNum, h, EPixelFormat::PIXEL_FORMAT_F32);
		_pointers_shadow._rt_depth_msaa = _rt_depth_msaa.get();
		for (uint32_t i = 0; i < nCount; ++i)
		{
			_rt_colors_msaa[i] = FSR_Buffer2D_Helper::CreateBuffer2D(w * _MSAASamplesNum, h, EPixelFormat::PIXEL_FORMAT_RGBA8888);
			_pointers_shadow._rt_colors_msaa[i] = _rt_colors_msaa[i].get();
		}
	}
}

// clear render target
static const float kFloat4One[4] = { 1.f, 1.f, 1.f, 1.f };
void FSR_Context::ClearRenderTarget(const glm::vec4& InColor)
{
	if (_pointers_shadow._rt_depth)
	{
		_pointers_shadow._rt_depth->Clear(kFloat4One);
	}
	for (uint32_t i = 0; i < MAX_MRT_COUNT; ++i)
	{
		if (_pointers_shadow._rt_colors[i])
		{
			_pointers_shadow._rt_colors[i]->Clear(&InColor.r);
		}
	} // end for i

	if (_bEnableMSAA)
	{
		_pointers_shadow._rt_depth_msaa->Clear(kFloat4One);
		for (uint32_t i = 0; i < MAX_MRT_COUNT; ++i)
		{
			if (_pointers_shadow._rt_colors_msaa[i])
			{
				_pointers_shadow._rt_colors_msaa[i]->Clear(&InColor.r);
			}
		} // end for i
	}
}

// set cull face mode
void FSR_Context::SetCullFaceMode(EFrontFace InMode)
{
	_front_face = InMode;
}

void FSR_Context::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	_viewport_rect._minx = static_cast<float>(x);
	_viewport_rect._miny = static_cast<float>(y);
	_viewport_rect._maxx = static_cast<float>(x + w);
	_viewport_rect._maxy = static_cast<float>(y + h);
}

void FSR_Context::SetModelViewMatrix(const glm::mat4x4& InModelView)
{
	_mvps._modelview = InModelView;
	_mvps._modelview_inv = glm::inverse(InModelView);
	_mvps._modelview_inv_t = glm::transpose(_mvps._modelview_inv);

	UpdateMVP();
}

// set projection matrix
void FSR_Context::SetProjectionMatrix(const glm::mat4x4& InProj)
{
	_mvps._projection = InProj;
	_mvps._projection_inv = glm::inverse(InProj);

	UpdateMVP();
}

void FSR_Context::UpdateMVP()
{
	_mvps._mvp = _mvps._projection * _mvps._modelview;
	_mvps._mvp_inv = glm::inverse(_mvps._mvp);
}

// set material
void FSR_Context::SetMaterial(const std::shared_ptr<FSR_Material>& InMaterial) 
{ 
	_material = InMaterial; 
	_pointers_shadow._material = _material.get();
}

//set pipeline
void FSR_Context::SetShader(const std::shared_ptr<FSR_VertexShader>& InVs, const std::shared_ptr<FSR_PixelShader>& InPs)
{
	_vs = InVs;
	_ps = InPs;

	_pointers_shadow._vs = _vs.get();
	_pointers_shadow._ps = _ps.get();
}

std::shared_ptr<FSR_Buffer2D> FSR_Context::GetDepthBuffer() const
{
	return _rt_depth;
}

std::shared_ptr<FSR_Buffer2D> FSR_Context::GetColorBuffer(uint32_t InIndex) const
{
	if (InIndex < MAX_MRT_COUNT)
	{
		return _rt_colors[InIndex];
	}
	
	return nullptr;
}

std::shared_ptr<FSR_Buffer2D> FSR_Context::GetMSAAColorBuffer(uint32_t InIndex) const
{
	if (InIndex < MAX_MRT_COUNT)
	{
		return _rt_colors_msaa[InIndex];
	}

	return nullptr;
}

void FSR_Context::BeginFrame()
{
#if SR_ENABLE_PERFORMACE_STAT
	if (_stats) {
		_stats->Reset();
	}
#endif
}

void FSR_Context::EndFrame()
{
	FSR_Renderer::Flush(*this);
	ResolveMSAABuffer();
}

void FSR_Context::ResolveMSAABuffer()
{
	if (!_bEnableMSAA)
	{
		return;
	}

	assert(_pointers_shadow._rt_depth && _pointers_shadow._rt_depth_msaa);
	const uint32_t w = _pointers_shadow._rt_depth->Width();
	const uint32_t h = _pointers_shadow._rt_depth->Height();
	const float factor = 1.f / _MSAASamplesNum;

	for (uint32_t cy = 0; cy < h; ++cy)
	{
		for (uint32_t cx = 0, msaa_cx = 0; cx < w; ++cx, msaa_cx += _MSAASamplesNum)
		{
			float sum = 0.f, d = 0.f;
			for (int32_t i = 0; i < _MSAASamplesNum; ++i)
			{
				_pointers_shadow._rt_depth_msaa->Read(msaa_cx + i, cy, d);
				sum += d;
			}

			_pointers_shadow._rt_depth->Write(cx, cy, sum * factor);
		} // end for cx
	} // end for cy

	for (uint32_t rt = 0; rt < MAX_MRT_COUNT; ++rt)
	{
		if (_pointers_shadow._rt_colors[rt] && _pointers_shadow._rt_colors_msaa[rt])
		{
			FSR_Texture2D* rt_color = _pointers_shadow._rt_colors[rt];
			FSR_Texture2D* rt_color_msaa = _pointers_shadow._rt_colors_msaa[rt];

			for (uint32_t cy = 0; cy < h; ++cy)
			{
				for (uint32_t cx = 0, msaa_cx = 0; cx < w; ++cx, msaa_cx += _MSAASamplesNum)
				{
					float RGBA[4];
					float rgba[4];
					for (int32_t i = 0; i < _MSAASamplesNum; ++i)
					{
						rt_color_msaa->Read(msaa_cx + i, cy, rgba);
						RGBA[0] += rgba[0];
						RGBA[1] += rgba[1];
						RGBA[2] += rgba[2];
						RGBA[3] += rgba[3];
					} // end for i

					RGBA[0] *= factor;
					RGBA[1] *= factor;
					RGBA[2] *= factor;
					RGBA[3] *= factor;
					rt_color->Write(cx, cy, RGBA);
				} // end for cx
			} // end for cy
		}
	} // end for rt
}

bool FSR_Context::DepthTestAndOverride(uint32_t cx, uint32_t cy, float InDepth) const
{
	if (_pointers_shadow._rt_depth)
	{
		float PrevDepth = 0.f;
		_pointers_shadow._rt_depth->Read(cx, cy, PrevDepth);
		if (InDepth <= PrevDepth)
		{
			_pointers_shadow._rt_depth->Write(cx, cy, InDepth);
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool FSR_Context::DepthTestAndOverrideMSAA(uint32_t cx, uint32_t cy, float InDepth, int32_t InSampleIndex) const
{
	assert(_bEnableMSAA && InSampleIndex < _MSAASamplesNum);

	if (_pointers_shadow._rt_depth_msaa)
	{
		float PrevDepth = 0.f;
		uint32_t cx_msaa = cx * _MSAASamplesNum + InSampleIndex;
		_pointers_shadow._rt_depth_msaa->Read(cx_msaa, cy, PrevDepth);
		if (InDepth <= PrevDepth)
		{
			_pointers_shadow._rt_depth_msaa->Write(cx_msaa, cy, InDepth);
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void FSR_Context::OutputAndMergeColor(int32_t cx, int32_t cy, FSRPixelShaderOutput& InPixelOutput) const
{
	for (uint32_t k = 0; k < InPixelOutput._color_cnt; ++k)
	{
		FSR_Texture2D* rt = _pointers_shadow._rt_colors[k];
		if (rt)
		{
			const glm::vec4& color = InPixelOutput._colors[k];
			rt->Write(cx, cy, &color.r);
		}
	} // end for k
}

void FSR_Context::OutputAndMergeColorMSAA(int32_t cx, int32_t cy, FSRPixelShaderOutput& InPixelOutput, int32_t InBitMask) const
{
	const uint32_t cx_msaa = cx * _MSAASamplesNum;

	for (uint32_t k = 0; k < InPixelOutput._color_cnt; ++k)
	{
		FSR_Texture2D* rt = _pointers_shadow._rt_colors_msaa[k];
		if (rt)
		{
			const glm::vec4& color = InPixelOutput._colors[k];
			for (int32_t index = 0; index < _MSAASamplesNum; ++index)
			{
				if (InBitMask & (0x01 << index))
				{
					rt->Write(cx_msaa + index, cy, &color.r);
				}
			} // end for index
		}
	} // end for k
}

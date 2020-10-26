// \brief
//		soft renderer context
//

#include "SR_Context.h"


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
	: _viewport_rect(0, 0, 1, 1)
	, _modelview(1.f)
	, _modelview_inv(1.f)
	, _modelview_inv_t(1.f)
	, _projection(1.f)
	, _projection_inv(1.f)
	, _front_face(EFrontFace::FACE_CW)
	, _bEnableMSAA(false)
	, _MSAASamplesNum(MSAA_SAMPLES)
{
	_stats = std::make_shared<FSR_Performance>();
	UpdateMVP();
}

FSR_Context::~FSR_Context()
{
}

// set render target
void FSR_Context::SetRenderTarget(uint32_t w, uint32_t h, uint32_t nCount, bool InbEnableMSAA)
{
	_rt_depth = FSR_Buffer2D_Helper::CreateBuffer2D(w, h, EPixelFormat::PIXEL_FORMAT_F32);
	nCount = std::min<uint32_t>(nCount, MAX_MRT_COUNT);
	for (uint32_t i=0; i<nCount; ++i)
	{
		_rt_colors[i] = FSR_Buffer2D_Helper::CreateBuffer2D(w, h, EPixelFormat::PIXEL_FORMAT_RGBAF32);
	}

	_bEnableMSAA = InbEnableMSAA;
	if (_bEnableMSAA)
	{
		_rt_depth_msaa = FSR_Buffer2D_Helper::CreateBuffer2D(w * _MSAASamplesNum, h, EPixelFormat::PIXEL_FORMAT_F32);
		for (uint32_t i = 0; i < nCount; ++i)
		{
			_rt_colors_msaa[i] = FSR_Buffer2D_Helper::CreateBuffer2D(w * _MSAASamplesNum, h, EPixelFormat::PIXEL_FORMAT_RGBAF32);
		}
	}
}

// clear render target
void FSR_Context::ClearRenderTarget(const glm::vec4& InColor)
{
	if (_rt_depth)
	{
		_rt_depth->Clear(1.f, 1.f, 1.f, 1.f);
	}
	for (uint32_t i = 0; i < MAX_MRT_COUNT; ++i)
	{
		if (_rt_colors[i])
		{
			_rt_colors[i]->Clear(InColor.r, InColor.g, InColor.b, InColor.a);
		}
	} // end for i

	if (_bEnableMSAA)
	{
		_rt_depth_msaa->Clear(1.f, 1.f, 1.f, 1.f);
		for (uint32_t i = 0; i < MAX_MRT_COUNT; ++i)
		{
			if (_rt_colors_msaa[i])
			{
				_rt_colors_msaa[i]->Clear(InColor.r, InColor.g, InColor.b, InColor.a);
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
	_modelview = InModelView;
	_modelview_inv = glm::inverse(InModelView);
	_modelview_inv_t = glm::transpose(_modelview_inv);

	UpdateMVP();
}

// set projection matrix
void FSR_Context::SetProjectionMatrix(const glm::mat4x4& InProj)
{
	_projection = InProj;
	_projection_inv = glm::inverse(InProj);

	UpdateMVP();
}

void FSR_Context::UpdateMVP()
{
	_mvp = _projection * _modelview;
	_mvp_inv = glm::inverse(_mvp);
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

void FSR_Context::ResolveMSAABuffer()
{
	if (!_bEnableMSAA)
	{
		return;
	}

	assert(_rt_depth && _rt_depth_msaa);
	const uint32_t w = _rt_depth->Width();
	const uint32_t h = _rt_depth->Height();
	const float factor = 1.f / _MSAASamplesNum;

	for (uint32_t cy = 0; cy < h; ++cy)
	{
		for (uint32_t cx = 0, msaa_cx = 0; cx < w; ++cx, msaa_cx += _MSAASamplesNum)
		{
			float sum = 0.f, d = 0.f;
			for (int32_t i = 0; i < _MSAASamplesNum; ++i)
			{
				_rt_depth_msaa->Read(msaa_cx + i, cy, d);
				sum += d;
			}

			_rt_depth->Write(cx, cy, sum * factor);
		} // end for cx
	} // end for cy

	for (uint32_t rt = 0; rt < MAX_MRT_COUNT; ++rt)
	{
		if (_rt_colors[rt] && _rt_colors_msaa[rt])
		{
			for (uint32_t cy = 0; cy < h; ++cy)
			{
				for (uint32_t cx = 0, msaa_cx = 0; cx < w; ++cx, msaa_cx += _MSAASamplesNum)
				{
					float R = 0.f, G = 0.f, B = 0.f, A = 0.f;
					float r, g, b, a;
					for (int32_t i = 0; i < _MSAASamplesNum; ++i)
					{
						_rt_colors_msaa[rt]->Read(msaa_cx + i, cy, r, g, b, a);
						R += r;
						G += g;
						B += b;
						A += a;
					} // end for i

					R *= factor; G *= factor; B *= factor; A *= factor;
					_rt_colors[rt]->Write(cx, cy, R, G, B, A);
				} // end for cx
			} // end for cy
		}
	} // end for rt
}

glm::vec3 FSR_Context::NDCToScreenPostion(const glm::vec3& ndc) const
{
	glm::vec3 screen_pos;

	screen_pos.x = glm::mix(_viewport_rect._minx, _viewport_rect._maxx, (ndc.x + 1.0f) * 0.5f);
	screen_pos.y = glm::mix(_viewport_rect._miny, _viewport_rect._maxy, (ndc.y + 1.0f) * 0.5f);
	screen_pos.z = (ndc.z + 1.0f) * 0.5f;
	return screen_pos;
}

bool FSR_Context::DepthTestAndOverride(uint32_t cx, uint32_t cy, float InDepth) const
{
	if (_rt_depth)
	{
		float PrevDepth = 0.f;
		_rt_depth->Read(cx, cy, PrevDepth);
		if (InDepth <= PrevDepth)
		{
			_rt_depth->Write(cx, cy, InDepth);
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

	if (_rt_depth_msaa)
	{
		float PrevDepth = 0.f;
		uint32_t cx_msaa = cx * _MSAASamplesNum + InSampleIndex;
		_rt_depth_msaa->Read(cx_msaa, cy, PrevDepth);
		if (InDepth <= PrevDepth)
		{
			_rt_depth_msaa->Write(cx_msaa, cy, InDepth);
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
		const std::shared_ptr<FSR_Texture2D>& rt = _rt_colors[k];
		if (rt)
		{
			const glm::vec4& color = InPixelOutput._colors[k];
			rt->Write(cx, cy, color.r, color.g, color.b, color.a);
		}
	} // end for k
}

void FSR_Context::OutputAndMergeColorMSAA(int32_t cx, int32_t cy, FSRPixelShaderOutput& InPixelOutput, int32_t InBitMask) const
{
	const uint32_t cx_msaa = cx * _MSAASamplesNum;

	for (uint32_t k = 0; k < InPixelOutput._color_cnt; ++k)
	{
		const std::shared_ptr<FSR_Texture2D>& rt = _rt_colors_msaa[k];
		if (rt)
		{
			const glm::vec4& color = InPixelOutput._colors[k];
			for (int32_t index = 0; index < _MSAASamplesNum; ++index)
			{
				if (InBitMask & (0x01 << index))
				{
					rt->Write(cx_msaa + index, cy, color.r, color.g, color.b, color.a);
				}
			} // end for index
		}
	} // end for k
}

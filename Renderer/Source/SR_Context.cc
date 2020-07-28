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
	{ EPixelFormat::PIXEL_FORMAT_RGBA8888,   4*4},
};

uint32_t LookupPixelFormatBytes(EPixelFormat InFormat)
{
	uint32_t idx = static_cast<uint32_t>(InFormat);
	const FPixelFormatDesc& desc = sPixelFormatTable[idx];

	assert(desc._format == InFormat);
	return desc._bytes_per_pixel;
}

void CopyVertexShaderOutput(FSRVertexShaderOutput& Dst, const FSRVertexShaderOutput& Src)
{
	Dst._attri_cnt = Src._attri_cnt;
	size_t Bytes = ((uint8_t *)&(Dst._attributes[Dst._attri_cnt])) - ((uint8_t*)&Dst);
	memcpy(&Dst, &Src, Bytes);
}


FSR_Context::FSR_Context()
	: _viewport_rect(glm::vec2(0,0), glm::vec2(1,1))
	, _modelview(1.f)
	, _modelview_inv(1.f)
	, _projection(1.f)
	, _projection_inv(1.f)
	, _front_face(EFrontFace::FACE_CW)
{
}

FSR_Context::~FSR_Context()
{
}

// set render target
void FSR_Context::SetRenderTarget(uint32_t w, uint32_t h, uint32_t nCount)
{
	_rt_depth = std::make_shared<FSR_DepthBuffer>(w, h, EPixelFormat::PIXEL_FORMAT_F32);
	nCount = std::min<uint32_t>(nCount, MAX_MRT_COUNT);
	for (uint32_t i=0; i<nCount; ++i)
	{
		_rt_colors[i] = std::make_shared<FSR_Texture2D>(w, h, EPixelFormat::PIXEL_FORMAT_RGBAF32);
	}
}

// clear render target
void FSR_Context::ClearRenderTarget(const glm::vec4& InColor)
{
	if (_rt_depth)
	{
		_rt_depth->Clear(InColor.r, InColor.g, InColor.b, InColor.a);
	}
	for (uint32_t i = 0; i < MAX_MRT_COUNT; ++i)
	{
		if (_rt_colors[i])
		{
			_rt_colors[i]->Clear(InColor.r, InColor.g, InColor.b, InColor.a);
		}
	} // end for i
}

// set cull face mode
void FSR_Context::SetCullFaceMode(EFrontFace InMode)
{
	_front_face = InMode;
}

void FSR_Context::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	_viewport_rect._min.x = static_cast<float>(x);
	_viewport_rect._min.y = static_cast<float>(y);
	_viewport_rect._max.x = static_cast<float>(x + w);
	_viewport_rect._max.y = static_cast<float>(y + h);
}

void FSR_Context::SetModelViewMatrix(const glm::mat4x4& InModelView)
{
	_modelview = InModelView;
	_modelview_inv = glm::inverse(InModelView);
}

// set projection matrix
void FSR_Context::SetProjectionMatrix(const glm::mat4x4& InProj)
{
	_projection = InProj;
	_projection_inv = glm::inverse(InProj);
}

glm::vec3 FSR_Context::NdcToScreenPostion(const glm::vec3& ndc) const
{
	glm::vec3 screen_pos;

	screen_pos.x = glm::mix(_viewport_rect._min.x, _viewport_rect._max.x, (ndc.x + 1.0f) * 0.5f);
	screen_pos.y = glm::mix(_viewport_rect._min.y, _viewport_rect._max.y, (ndc.x + 1.0f) * 0.5f);
	screen_pos.z = (ndc.z + 1.0f) * 0.5f;
	return screen_pos;
}

bool FSR_Context::DepthTestAndOverride(uint32_t cx, uint32_t cy, float InDepth) const
{
	if (_rt_depth && _rt_depth->IsValid())
	{
		float PrevDepth = 0.f;
		_rt_depth->Read(cx, cy, PrevDepth);
		if (InDepth < PrevDepth)
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

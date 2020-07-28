// \brief
//		buffer 2d.
//

#include "SR_Buffer2D.h"


FSR_Buffer2D::FSR_Buffer2D(uint32_t width, uint32_t height, EPixelFormat pixelformat)
	: _w(width)
	, _h(height)
	, _format(pixelformat)
{
	_bytes_per_pixel = LookupPixelFormatBytes(_format);

	uint32_t bytes_cnt = _w * _h * _bytes_per_pixel;
	_buffer = std::make_shared<FSR_Buffer>(bytes_cnt);
}

FSR_Buffer2D::~FSR_Buffer2D()
{
	// do nothing
}

// read a element, (cx,cy) is element coordination
bool FSR_Buffer2D::Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
{
	R = G = B = A = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	const uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		R = (uint8_t)(*(reinterpret_cast<const uint16_t*>(pData)));
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
	{
		float Val = *(reinterpret_cast<const float*>(pData));
		R = glm::clamp<uint8_t>(uint8_t(Val * 255), 0, 255);
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
	{
		R = *pData;  G = *(pData + 1); B = *(pData + 2);
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
	{
		R = *pData;  G = *(pData + 1); B = *(pData + 2); A = *(pData + 3);
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	{
		const float* pFloat = reinterpret_cast<const float*>(pData);
		R = glm::clamp<uint8_t>(uint8_t(*pFloat * 255), 0, 255);
		G = glm::clamp<uint8_t>(uint8_t(*(pFloat + 1) * 255), 0, 255);
		B = glm::clamp<uint8_t>(uint8_t(*(pFloat + 2) * 255), 0, 255);
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		const float* pFloat = reinterpret_cast<const float*>(pData);
		R = glm::clamp<uint8_t>(uint8_t(*pFloat * 255), 0, 255);
		G = glm::clamp<uint8_t>(uint8_t(*(pFloat + 1) * 255), 0, 255);
		B = glm::clamp<uint8_t>(uint8_t(*(pFloat + 2) * 255), 0, 255);
		A = glm::clamp<uint8_t>(uint8_t(*(pFloat + 3) * 255), 0, 255);
	}
		break;
	default:
		assert(0);
		break;
	}

	return true;
}

bool FSR_Buffer2D::Read(uint32_t cx, uint32_t cy, uint16_t& Value) const
{
	Value = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	const uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		Value = (uint8_t)(*(reinterpret_cast<const uint16_t*>(pData)));
		break;
	default:
		return false;
	}

	return true;
}


// maybe normalized [0, 1] before return.
bool FSR_Buffer2D::Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const
{
	R = G = B = A = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	const uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		R = *(reinterpret_cast<const uint16_t*>(pData)) / 65535.f;
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
	{
		R = *(reinterpret_cast<const float*>(pData));
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
	{
		R = (*pData)/255.f;  G = *(pData + 1)/255.f; B = *(pData + 2)/255.f;
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
	{
		R = *pData/255.f;  G = *(pData + 1)/255.f; B = *(pData + 2)/255.f; A = *(pData + 3)/255.f;
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	{
		const float* pFloat = reinterpret_cast<const float*>(pData);
		R = *pFloat;
		G = *(pFloat + 1);
		B = *(pFloat + 2);
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		const float* pFloat = reinterpret_cast<const float*>(pData);
		R = *pFloat;
		G = *(pFloat + 1);
		B = *(pFloat + 2);
		A = *(pFloat + 3);
	}
		break;
	default:
		assert(0);
		break;
	}

	return true;
}

bool FSR_Buffer2D::Read(uint32_t cx, uint32_t cy, float& Value)
{
	Value = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	const uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		Value = *(reinterpret_cast<const uint16_t*>(pData)) / 65535.f;
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
		Value = *(reinterpret_cast<const float*>(pData));
		break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
		Value = (*pData) / 255.f;
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
		Value = (*pData) / 255.f;
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	{
		const float* pFloat = reinterpret_cast<const float*>(pData);
		Value = *pFloat;
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		const float* pFloat = reinterpret_cast<const float*>(pData);
		Value = *pFloat;
	}
		break;
	default:
		assert(0);
		break;
	}

	return true;
}

bool FSR_Buffer2D::Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		*(reinterpret_cast<uint16_t*>(pData)) = R;
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
	{
		*(reinterpret_cast<float*>(pData)) = R / 255.f;
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
	{
		*pData = R;  *(pData + 1) = G; *(pData + 2) = B;
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
	{
		*pData = R;  *(pData + 1) = G; *(pData + 2) = B; *(pData + 3) = A;
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	{
		float* pFloat = reinterpret_cast<float*>(pData);
		*pFloat = R / 255.f;
		*(pFloat + 1) = G / 255.f;
		*(pFloat + 2) = B / 255.f;
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		float* pFloat = reinterpret_cast<float*>(pData);
		*pFloat = R / 255.f;
		*(pFloat + 1) = G / 255.f;
		*(pFloat + 2) = B / 255.f;
		*(pFloat + 3) = A / 255.f;
	}
	break;
	default:
		assert(0);
		break;
	}

	return true;
}

bool FSR_Buffer2D::Write(uint32_t cx, uint32_t cy, uint16_t& Value)
{
	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		*(reinterpret_cast<uint16_t*>(pData)) = Value;
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		*(reinterpret_cast<float*>(pData)) = Value / 65535.f;
	}
		break;
	default:
		assert(0);
		break;
	}

	return true;
}

bool FSR_Buffer2D::Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		*(reinterpret_cast<uint16_t*>(pData)) = glm::clamp<uint16_t>(uint16_t(R * 65535.f), 0, 65535);
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
	{
		*(reinterpret_cast<float*>(pData)) = R;
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
	{
		*pData = glm::clamp<uint8_t>(uint8_t(R*255), 0, 255);  
		*(pData + 1) = glm::clamp<uint8_t>(uint8_t(G*255), 0, 255); 
		*(pData + 2) = glm::clamp<uint8_t>(uint8_t(B*255), 0, 255);
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
	{
		*pData = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
		*(pData + 1) = glm::clamp<uint8_t>(uint8_t(G * 255), 0, 255);
		*(pData + 2) = glm::clamp<uint8_t>(uint8_t(B * 255), 0, 255);
		*(pData + 2) = glm::clamp<uint8_t>(uint8_t(A * 255), 0, 255);
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	{
		float* pFloat = reinterpret_cast<float*>(pData);
		*pFloat = R;
		*(pFloat + 1) = G;
		*(pFloat + 2) = B;
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		float* pFloat = reinterpret_cast<float*>(pData);
		*pFloat = R;
		*(pFloat + 1) = G;
		*(pFloat + 2) = B;
		*(pFloat + 3) = A;
	}
	break;
	default:
		assert(0);
		break;
	}

	return true;
}

bool FSR_Buffer2D::Write(uint32_t cx, uint32_t cy, float R)
{
	uint32_t offset = GetElementOffset(cx, cy);
	if (!IsValid() || offset == SR_INVALID_INDEX)
	{
		return false;
	}

	uint8_t* pData = _buffer->data();
	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		*(reinterpret_cast<uint16_t*>(pData)) = glm::clamp<uint16_t>(uint16_t(R * 65535.f), 0, 65535);
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
	{
		*(reinterpret_cast<float*>(pData)) = R;
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
	{
		*pData = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		float* pFloat = reinterpret_cast<float*>(pData);
		*pFloat = R;
	}
		break;
	default:
		assert(0);
		break;
	}

	return true;
}

void FSR_Buffer2D::Clear(float R, float G, float B, float A)
{
	if (!IsValid())
	{
		return;
	}

	switch (_format)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
	{
		uint16_t R = glm::clamp<uint16_t>(uint16_t(R * 65535.f), 0, 65535);
		uint16_t* pData = reinterpret_cast<uint16_t*>(_buffer->data());
		for (uint32_t k = 0; k < (_w * _h); ++k)
		{
			*pData++ = R;
		}
	}
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
	{
		float* pData = reinterpret_cast<float*>(_buffer->data());
		for (uint32_t k = 0; k < (_w * _h); ++k)
		{
			*pData++ = R;
		}
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
	{
		uint8_t R8 = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
		uint8_t G8 = glm::clamp<uint8_t>(uint8_t(G * 255), 0, 255);
		uint8_t B8 = glm::clamp<uint8_t>(uint8_t(B * 255), 0, 255);
		uint8_t* pData = reinterpret_cast<uint8_t*>(_buffer->data());
		for (uint32_t k = 0; k < (_w * _h); ++k)
		{
			*pData++ = R8;
			*pData++ = G8;
			*pData++ = B8;
		}
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
	{
		uint8_t R8 = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
		uint8_t G8 = glm::clamp<uint8_t>(uint8_t(G * 255), 0, 255);
		uint8_t B8 = glm::clamp<uint8_t>(uint8_t(B * 255), 0, 255);
		uint8_t A8 = glm::clamp<uint8_t>(uint8_t(A * 255), 0, 255);
		uint8_t* pData = reinterpret_cast<uint8_t*>(_buffer->data());
		for (uint32_t k = 0; k < (_w * _h); ++k)
		{
			*pData++ = R8;
			*pData++ = G8;
			*pData++ = B8;
			*pData++ = A8;
		}
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
	{
		float* pData = reinterpret_cast<float*>(_buffer->data());
		for (uint32_t k = 0; k < (_w * _h); ++k)
		{
			*pData++ = R;
			*pData++ = G;
			*pData++ = B;
		}
	}
	break;
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
	{
		float* pData = reinterpret_cast<float*>(_buffer->data());
		for (uint32_t k = 0; k < (_w * _h); ++k)
		{
			*pData++ = R;
			*pData++ = G;
			*pData++ = B;
			*pData++ = A;
		}
	}
	break;
	default:
		assert(0);
		break;
	}
}

// sample element
bool FSR_Buffer2D::Sample2DNearest(float u, float v, float& R, float& G, float& B, float& A)
{
	u = glm::clamp<float>(u, 0.f, 1.f);
	v = glm::clamp<float>(v, 0.f, 1.f);
	uint32_t cx = glm::clamp<int32_t>(int32_t(_w * u), 0, _w-1);
	uint32_t cy = glm::clamp<int32_t>(int32_t(_h * v), 0, _h-1);

	return Read(cx, cy, R, G, B, A);
}

bool FSR_Buffer2D::Sample2DLinear(float u, float v, float& R, float& G, float& B, float& A)
{
	u = glm::clamp<float>(u, 0.f, 1.f);
	v = glm::clamp<float>(v, 0.f, 1.f);
	
	float cx0 = _w * u;
	if (cx0 >= _w) { cx0 -= _w; }
	
	float cx1 = cx0 + 1.f;
	if (cx1 >= _w) { cx1 -= _w; }

	float cy0 = _h * v;
	if (cy0 >= _h) { cy0 -= _h; }

	float cy1 = cy0 + 1.f;
	if (cy1 >= _h) { cy1 -= _h; }

	float tu = 1.f - glm::fract(cx0);
	float tv = 1.f - glm::fract(cy0);

	/*  c2|c3
	  ----+-----
	    c0|c1
	*/
	
	uint32_t icx0 = static_cast<uint32_t>(cx0);
	uint32_t icx1 = static_cast<uint32_t>(cx1);
	uint32_t icy0 = static_cast<uint32_t>(cy0);
	uint32_t icy1 = static_cast<uint32_t>(cy1);

	glm::vec4 c0, c1, c2, c3;
	Read(icx0, icy0, c0.r, c0.g, c0.b, c0.a);
	Read(icx1, icy0, c1.r, c1.g, c1.b, c1.a);
	Read(icx0, icy1, c2.r, c2.g, c2.b, c2.a);
	Read(icx1, icy1, c3.r, c3.g, c3.b, c3.a);

	glm::vec4 color0 = glm::mix(c0, c1, tu);
	glm::vec4 color1 = glm::mix(c2, c3, tu);
	glm::vec4 result = glm::mix(color0, color1, tv);

	R = result.r;
	G = result.g;
	B = result.b;
	A = result.a;

	return true;
}


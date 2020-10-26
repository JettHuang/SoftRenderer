// \brief
//		buffer 2d.
//

#include "stb_image.h"
#include "stb_image_write.h"
#include "SR_Buffer2D.h"


const float FSR_Buffer2D::ONE_OVER_255 = (1.f / 255.f);
const float FSR_Buffer2D::ONE_OVER_65535 = (1.f / 65535.f);

FSR_Buffer2D::FSR_Buffer2D(uint32_t width, uint32_t height, EPixelFormat pixelformat)
	: _w(width)
	, _h(height)
	, _format(pixelformat)
{
	_bytes_per_pixel = LookupPixelFormatBytes(_format);

	uint32_t bytes_cnt = _w * _h * _bytes_per_pixel;
	_buffer.resize(bytes_cnt);
}

// sample element
bool FSR_Buffer2D::Sample2DNearest(float u, float v, float& R, float& G, float& B, float& A) const
{
	u = u - floor(u);
	v = v - floor(v);

	uint32_t cx = glm::clamp<int32_t>(int32_t(_w * u), 0, _w - 1);
	uint32_t cy = glm::clamp<int32_t>(int32_t(_h * v), 0, _h - 1);

	return Read(cx, cy, R, G, B, A);
}

bool FSR_Buffer2D::Sample2DLinear(float u, float v, float& R, float& G, float& B, float& A) const
{
	u = u - floor(u);
	v = v - floor(v);

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

//////////////////////////////////////////////////////////////////////////
// PIXEL_FORMAT_U16

bool FSR_Buffer2D_U16::Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
{
	R = G = B = A = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	R = (uint8_t)(*(reinterpret_cast<const uint16_t*>(pData)));
	return true;
}

bool FSR_Buffer2D_U16::Read(uint32_t cx, uint32_t cy, uint16_t& Value) const
{
	Value = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	Value = (uint8_t)(*(reinterpret_cast<const uint16_t*>(pData)));

	return true;
}

// maybe normalized [0, 1] before return.
bool FSR_Buffer2D_U16::Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const
{
	R = G = B = A = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	R = *(reinterpret_cast<const uint16_t*>(pData)) * ONE_OVER_65535;
	return true;
}

bool FSR_Buffer2D_U16::Read(uint32_t cx, uint32_t cy, float& Value) const
{
	Value = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	Value = *(reinterpret_cast<const uint16_t*>(pData)) * ONE_OVER_65535;
	return true;
}

bool FSR_Buffer2D_U16::Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<uint16_t*>(pData)) = R;
	return true;
}

bool FSR_Buffer2D_U16::Write(uint32_t cx, uint32_t cy, uint16_t& Value)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<uint16_t*>(pData)) = Value;
	return true;
}

bool FSR_Buffer2D_U16::Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<uint16_t*>(pData)) = glm::clamp<uint16_t>(uint16_t(R * 65535.f), 0, 65535);
	return true;
}

bool FSR_Buffer2D_U16::Write(uint32_t cx, uint32_t cy, float R)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<uint16_t*>(pData)) = glm::clamp<uint16_t>(uint16_t(R * 65535.f), 0, 65535);
	return true;
}

void FSR_Buffer2D_U16::Clear(float R, float G, float B, float A)
{
	uint16_t R16 = glm::clamp<uint16_t>(uint16_t(R * 65535.f), 0, 65535);
	uint16_t* pData = reinterpret_cast<uint16_t*>(_buffer.data());
	for (uint32_t k = 0; k < (_w * _h); ++k)
	{
		*pData++ = R16;
	}
}

//////////////////////////////////////////////////////////////////////////
// PIXEL_FORMAT_F32

bool FSR_Buffer2D_F32::Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
{
	R = G = B = A = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	float Val = *(reinterpret_cast<const float*>(pData));
	R = glm::clamp<uint8_t>(uint8_t(Val * 255), 0, 255);
	return true;
}

bool FSR_Buffer2D_F32::Read(uint32_t cx, uint32_t cy, uint16_t& Value) const
{
	Value = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	return false;
}

// maybe normalized [0, 1] before return.
bool FSR_Buffer2D_F32::Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const
{
	R = G = B = A = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	R = *(reinterpret_cast<const float*>(pData));
	return true;
}

bool FSR_Buffer2D_F32::Read(uint32_t cx, uint32_t cy, float& Value) const
{
	Value = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	Value = *(reinterpret_cast<const float*>(pData));
	return true;
}

bool FSR_Buffer2D_F32::Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<float*>(pData)) = R * ONE_OVER_255;
	return true;
}

bool FSR_Buffer2D_F32::Write(uint32_t cx, uint32_t cy, uint16_t& Value)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<float*>(pData)) = Value * ONE_OVER_65535;
	return true;
}

bool FSR_Buffer2D_F32::Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<float*>(pData)) = R;

	return true;
}

bool FSR_Buffer2D_F32::Write(uint32_t cx, uint32_t cy, float R)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<float*>(pData)) = R;

	return true;
}

void FSR_Buffer2D_F32::Clear(float R, float G, float B, float A)
{
	float* pData = reinterpret_cast<float*>(_buffer.data());
	for (uint32_t k = 0; k < (_w * _h); ++k)
	{
		*pData++ = R;
	}
}

//////////////////////////////////////////////////////////////////////////
// FSR_Buffer2D_RGB888

bool FSR_Buffer2D_RGB888::Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
{
	R = G = B = A = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	R = *pData;  G = *(pData + 1); B = *(pData + 2);

	return true;
}

bool FSR_Buffer2D_RGB888::Read(uint32_t cx, uint32_t cy, uint16_t& Value) const
{
	Value = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	return false;
}

// maybe normalized [0, 1] before return.
bool FSR_Buffer2D_RGB888::Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	R = *(pData);
	G = *(pData + 1);
	B = *(pData + 2);

	R *= ONE_OVER_255;
	G *= ONE_OVER_255;
	B *= ONE_OVER_255;
	A = 0.f;
	return true;
}

bool FSR_Buffer2D_RGB888::Read(uint32_t cx, uint32_t cy, float& Value) const
{
	Value = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	Value = (*pData) * ONE_OVER_255;

	return true;
}

bool FSR_Buffer2D_RGB888::Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*pData = R;  *(pData + 1) = G; *(pData + 2) = B;

	return true;
}

bool FSR_Buffer2D_RGB888::Write(uint32_t cx, uint32_t cy, uint16_t& Value)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	return false;
}

bool FSR_Buffer2D_RGB888::Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*pData = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
	*(pData + 1) = glm::clamp<uint8_t>(uint8_t(G * 255), 0, 255);
	*(pData + 2) = glm::clamp<uint8_t>(uint8_t(B * 255), 0, 255);

	return true;
}

bool FSR_Buffer2D_RGB888::Write(uint32_t cx, uint32_t cy, float R)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*pData = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);

	return true;
}

void FSR_Buffer2D_RGB888::Clear(float R, float G, float B, float A)
{
	uint8_t R8 = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
	uint8_t G8 = glm::clamp<uint8_t>(uint8_t(G * 255), 0, 255);
	uint8_t B8 = glm::clamp<uint8_t>(uint8_t(B * 255), 0, 255);
	uint8_t* pData = reinterpret_cast<uint8_t*>(_buffer.data());
	for (uint32_t k = 0; k < (_w * _h); ++k)
	{
		*pData++ = R8;
		*pData++ = G8;
		*pData++ = B8;
	}
}

//////////////////////////////////////////////////////////////////////////
// PIXEL_FORMAT_RGBA8888

bool FSR_Buffer2D_RGBA8888::Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
{
	R = G = B = A = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	R = *pData;  G = *(pData + 1); B = *(pData + 2); A = *(pData + 3);

	return true;
}

bool FSR_Buffer2D_RGBA8888::Read(uint32_t cx, uint32_t cy, uint16_t& Value) const
{
	Value = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	return false;
}

// maybe normalized [0, 1] before return.
bool FSR_Buffer2D_RGBA8888::Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const
{
	R = G = B = A = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	R = *pData * ONE_OVER_255;  
	G = *(pData + 1) * ONE_OVER_255; 
	B = *(pData + 2) * ONE_OVER_255; 
	A = *(pData + 3) * ONE_OVER_255;

	return true;
}

bool FSR_Buffer2D_RGBA8888::Read(uint32_t cx, uint32_t cy, float& Value) const
{
	Value = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	Value = (*pData) * ONE_OVER_255;

	return true;
}

bool FSR_Buffer2D_RGBA8888::Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*pData = R;  *(pData + 1) = G; *(pData + 2) = B; *(pData + 3) = A;

	return true;
}

bool FSR_Buffer2D_RGBA8888::Write(uint32_t cx, uint32_t cy, uint16_t& Value)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	return false;
}

bool FSR_Buffer2D_RGBA8888::Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*pData = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
	*(pData + 1) = glm::clamp<uint8_t>(uint8_t(G * 255), 0, 255);
	*(pData + 2) = glm::clamp<uint8_t>(uint8_t(B * 255), 0, 255);
	*(pData + 2) = glm::clamp<uint8_t>(uint8_t(A * 255), 0, 255);

	return true;
}

bool FSR_Buffer2D_RGBA8888::Write(uint32_t cx, uint32_t cy, float R)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*pData = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);

	return true;
}

void FSR_Buffer2D_RGBA8888::Clear(float R, float G, float B, float A)
{
	uint8_t R8 = glm::clamp<uint8_t>(uint8_t(R * 255), 0, 255);
	uint8_t G8 = glm::clamp<uint8_t>(uint8_t(G * 255), 0, 255);
	uint8_t B8 = glm::clamp<uint8_t>(uint8_t(B * 255), 0, 255);
	uint8_t A8 = glm::clamp<uint8_t>(uint8_t(A * 255), 0, 255);
	uint8_t* pData = reinterpret_cast<uint8_t*>(_buffer.data());
	for (uint32_t k = 0; k < (_w * _h); ++k)
	{
		*pData++ = R8;
		*pData++ = G8;
		*pData++ = B8;
		*pData++ = A8;
	} // end for k
}

//////////////////////////////////////////////////////////////////////////
// FSR_Buffer2D_RGBF32

bool FSR_Buffer2D_RGBF32::Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
{
	R = G = B = A = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	const float* pFloat = reinterpret_cast<const float*>(pData);
	R = glm::clamp<uint8_t>(uint8_t(*pFloat * 255), 0, 255);
	G = glm::clamp<uint8_t>(uint8_t(*(pFloat + 1) * 255), 0, 255);
	B = glm::clamp<uint8_t>(uint8_t(*(pFloat + 2) * 255), 0, 255);

	return true;
}

bool FSR_Buffer2D_RGBF32::Read(uint32_t cx, uint32_t cy, uint16_t& Value) const
{
	Value = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	return false;
}

// maybe normalized [0, 1] before return.
bool FSR_Buffer2D_RGBF32::Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const
{
	R = G = B = A = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	const float* pFloat = reinterpret_cast<const float*>(pData);
	R = *pFloat;
	G = *(pFloat + 1);
	B = *(pFloat + 2);

	return true;
}

bool FSR_Buffer2D_RGBF32::Read(uint32_t cx, uint32_t cy, float& Value) const
{
	Value = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	const float* pFloat = reinterpret_cast<const float*>(pData);
	Value = *pFloat;

	return true;
}

bool FSR_Buffer2D_RGBF32::Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	float* pFloat = reinterpret_cast<float*>(pData);
	*pFloat = R * ONE_OVER_255;
	*(pFloat + 1) = G * ONE_OVER_255;
	*(pFloat + 2) = B * ONE_OVER_255;

	return true;
}

bool FSR_Buffer2D_RGBF32::Write(uint32_t cx, uint32_t cy, uint16_t& Value)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<float*>(pData)) = Value * ONE_OVER_65535;

	return true;
}

bool FSR_Buffer2D_RGBF32::Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	float* pFloat = reinterpret_cast<float*>(pData);
	*pFloat = R;
	*(pFloat + 1) = G;
	*(pFloat + 2) = B;

	return true;
}

bool FSR_Buffer2D_RGBF32::Write(uint32_t cx, uint32_t cy, float R)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	float* pFloat = reinterpret_cast<float*>(pData);
	*pFloat = R;

	return true;
}

void FSR_Buffer2D_RGBF32::Clear(float R, float G, float B, float A)
{
	float* pData = reinterpret_cast<float*>(_buffer.data());
	for (uint32_t k = 0; k < (_w * _h); ++k)
	{
		*pData++ = R;
		*pData++ = G;
		*pData++ = B;
	}
}

//////////////////////////////////////////////////////////////////////////
// PIXEL_FORMAT_RGBAF32

bool FSR_Buffer2D_RGBAF32::Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const
{
	R = G = B = A = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	const float* pFloat = reinterpret_cast<const float*>(pData);
	R = glm::clamp<uint8_t>(uint8_t(*pFloat * 255), 0, 255);
	G = glm::clamp<uint8_t>(uint8_t(*(pFloat + 1) * 255), 0, 255);
	B = glm::clamp<uint8_t>(uint8_t(*(pFloat + 2) * 255), 0, 255);
	A = glm::clamp<uint8_t>(uint8_t(*(pFloat + 3) * 255), 0, 255);

	return true;
}

bool FSR_Buffer2D_RGBAF32::Read(uint32_t cx, uint32_t cy, uint16_t& Value) const
{
	Value = 0;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	return false;
}

// maybe normalized [0, 1] before return.
bool FSR_Buffer2D_RGBAF32::Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const
{
	R = G = B = A = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	const float* pFloat = reinterpret_cast<const float*>(pData);
	R = *pFloat;
	G = *(pFloat + 1);
	B = *(pFloat + 2);
	A = *(pFloat + 3);

	return true;
}

bool FSR_Buffer2D_RGBAF32::Read(uint32_t cx, uint32_t cy, float& Value) const
{
	Value = 0.f;

	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	const uint8_t* pData = _buffer.data() + offset;
	const float* pFloat = reinterpret_cast<const float*>(pData);
	Value = *pFloat;

	return true;
}

bool FSR_Buffer2D_RGBAF32::Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	float* pFloat = reinterpret_cast<float*>(pData);
	*pFloat = R * ONE_OVER_255;
	*(pFloat + 1) = G * ONE_OVER_255;
	*(pFloat + 2) = B * ONE_OVER_255;
	*(pFloat + 3) = A * ONE_OVER_255;

	return true;
}

bool FSR_Buffer2D_RGBAF32::Write(uint32_t cx, uint32_t cy, uint16_t& Value)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	*(reinterpret_cast<float*>(pData)) = Value * ONE_OVER_65535;

	return true;
}

bool FSR_Buffer2D_RGBAF32::Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	float* pFloat = reinterpret_cast<float*>(pData);
	*pFloat = R;
	*(pFloat + 1) = G;
	*(pFloat + 2) = B;
	*(pFloat + 3) = A;

	return true;
}

bool FSR_Buffer2D_RGBAF32::Write(uint32_t cx, uint32_t cy, float R)
{
	uint32_t offset = GetElementOffset(cx, cy);
	assert(offset != SR_INVALID_INDEX);

	uint8_t* pData = _buffer.data() + offset;
	float* pFloat = reinterpret_cast<float*>(pData);
	*pFloat = R;

	return true;
}

void FSR_Buffer2D_RGBAF32::Clear(float R, float G, float B, float A)
{
	float* pData = reinterpret_cast<float*>(_buffer.data());
	for (uint32_t k = 0; k < (_w * _h); ++k)
	{
		*pData++ = R;
		*pData++ = G;
		*pData++ = B;
		*pData++ = A;
	}
}

//////////////////////////////////////////////////////////////////////////
// Helpers

std::shared_ptr<FSR_Buffer2D> FSR_Buffer2D_Helper::CreateBuffer2D(uint32_t width, uint32_t height, EPixelFormat pixelformat)
{
	std::shared_ptr<FSR_Buffer2D> buffer2d;

	switch (pixelformat)
	{
	case EPixelFormat::PIXEL_FORMAT_U16:
		buffer2d = std::make_shared<FSR_Buffer2D_U16>(width, height);
		break;
	case EPixelFormat::PIXEL_FORMAT_F32:
		buffer2d = std::make_shared<FSR_Buffer2D_F32>(width, height);
		break;
	case EPixelFormat::PIXEL_FORMAT_RGB888:
		buffer2d = std::make_shared<FSR_Buffer2D_RGB888>(width, height);
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBA8888:
		buffer2d = std::make_shared<FSR_Buffer2D_RGBA8888>(width, height);
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBF32:
		buffer2d = std::make_shared<FSR_Buffer2D_RGBF32>(width, height);
		break;
	case EPixelFormat::PIXEL_FORMAT_RGBAF32:
		buffer2d = std::make_shared<FSR_Buffer2D_RGBAF32>(width, height);
		break;
	default:
		assert(0 && "unknown pixel format....");
		break;
	}

	return buffer2d;
}

// load & save
std::shared_ptr<FSR_Buffer2D> FSR_Buffer2D_Helper::LoadImageFile(const char* InFileName)
{
	int w, h, channel;

	stbi_uc* pData = stbi_load(InFileName, &w, &h, &channel, 0);
	if (!pData) {
		return nullptr;
	}

	EPixelFormat PixelFormat;
	if (channel == 3) {
		PixelFormat = EPixelFormat::PIXEL_FORMAT_RGB888;
	}
	else if (channel == 4) {
		PixelFormat = EPixelFormat::PIXEL_FORMAT_RGBA8888;
	}
	else {
		return nullptr;
	}

	std::shared_ptr<FSR_Buffer2D> Buffer2d = CreateBuffer2D(w, h, PixelFormat);
	assert(Buffer2d);

	uint8_t* pDst = Buffer2d->Data();
	uint32_t nBytes = Buffer2d->Length();
	memcpy(pDst, pData, nBytes);

	return Buffer2d;
}

bool FSR_Buffer2D_Helper::SaveImageFile(const std::shared_ptr<FSR_Buffer2D> InTexture, const char* InFileName)
{
	return false;
}

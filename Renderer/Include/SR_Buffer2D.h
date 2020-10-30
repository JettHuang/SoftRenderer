// \brief
//	render target.
//

#pragma once

#include "SR_Common.h"
#include <vector>


class FSR_Buffer2D
{
public:
	virtual ~FSR_Buffer2D() {}

	uint32_t Width() const { return _w; }
	uint32_t Height() const { return _h; }
	EPixelFormat Format() const { return _format; }
	uint32_t Length() const { return _buffer.size(); }
	const uint8_t* Data() const { return _buffer.data(); }
	uint8_t* Data() { return _buffer.data(); }

	// read a element, (cx,cy) is element coordination
	virtual bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const { return false; }
	virtual bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const { return false; }
	// maybe normalized [0, 1] before return.
	virtual bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const { return false; }
	virtual bool Read(uint32_t cx, uint32_t cy, float& Value) const { return false; }

	virtual bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A) { return false; }
	virtual bool Write(uint32_t cx, uint32_t cy, uint16_t& Value) { return false; }
	virtual bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A) { return false; }
	virtual bool Write(uint32_t cx, uint32_t cy, float Value) { return false; }
	virtual void Clear(float R, float G, float B, float A) {}

	// get row pointer
	const uint8_t* GetRowData(uint32_t cy) const { return _buffer.data() + (cy * _bytes_per_line); }
	uint32_t BytesPerLine() const { return _bytes_per_line; }

	// sample element
	virtual bool Sample2DNearest(float u, float v, float& R, float& G, float& B, float& A) const;
	virtual bool Sample2DLinear(float u, float v, float& R, float& G, float& B, float& A) const;

protected:
	FSR_Buffer2D(uint32_t width, uint32_t height, EPixelFormat pixelformat);

	inline uint32_t GetElementOffset(uint32_t cx, uint32_t cy) const
	{
		return (cx >= _w || cy >= _h) ? SR_INVALID_INDEX : ((_w * cy + cx) * _bytes_per_pixel);
	}

	

	static const float ONE_OVER_255;
	static const float ONE_OVER_65535;
protected:
	uint32_t _w, _h;
	uint32_t _bytes_per_pixel;
	uint32_t _bytes_per_line;
	EPixelFormat _format;

	std::vector<uint8_t> _buffer;
};


using FSR_DepthBuffer = FSR_Buffer2D;
using FSR_Texture2D = FSR_Buffer2D;


// detail buffers
class FSR_Buffer2D_U16 : public FSR_Buffer2D
{
public:
	FSR_Buffer2D_U16(uint32_t width, uint32_t height)
		: FSR_DepthBuffer(width, height, EPixelFormat::PIXEL_FORMAT_U16)
	{}

	// read a element, (cx,cy) is element coordination
	virtual bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const override;
	// maybe normalized [0, 1] before return.
	virtual bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, float& Value) const override;

	virtual bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, uint16_t& Value) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float Value) override;
	virtual void Clear(float R, float G, float B, float A) override;
};

class FSR_Buffer2D_F32 : public FSR_Buffer2D
{
public:
	FSR_Buffer2D_F32(uint32_t width, uint32_t height)
		: FSR_DepthBuffer(width, height, EPixelFormat::PIXEL_FORMAT_F32)
	{}

	// read a element, (cx,cy) is element coordination
	virtual bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const override;
	// maybe normalized [0, 1] before return.
	virtual bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, float& Value) const override;

	virtual bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, uint16_t& Value) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float Value) override;
	virtual void Clear(float R, float G, float B, float A) override;
};

class FSR_Buffer2D_RGB888 : public FSR_Buffer2D
{
public:
	FSR_Buffer2D_RGB888(uint32_t width, uint32_t height)
		: FSR_DepthBuffer(width, height, EPixelFormat::PIXEL_FORMAT_RGB888)
	{}

	// read a element, (cx,cy) is element coordination
	virtual bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const override;
	// maybe normalized [0, 1] before return.
	virtual bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, float& Value) const override;

	virtual bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, uint16_t& Value) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float Value) override;
	virtual void Clear(float R, float G, float B, float A) override;
};

class FSR_Buffer2D_RGBA8888 : public FSR_Buffer2D
{
public:
	FSR_Buffer2D_RGBA8888(uint32_t width, uint32_t height)
		: FSR_DepthBuffer(width, height, EPixelFormat::PIXEL_FORMAT_RGBA8888)
	{}

	// read a element, (cx,cy) is element coordination
	virtual bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const override;
	// maybe normalized [0, 1] before return.
	virtual bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, float& Value) const override;

	virtual bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, uint16_t& Value) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float Value) override;
	virtual void Clear(float R, float G, float B, float A) override;
};

class FSR_Buffer2D_RGBF32 : public FSR_Buffer2D
{
public:
	FSR_Buffer2D_RGBF32(uint32_t width, uint32_t height)
		: FSR_DepthBuffer(width, height, EPixelFormat::PIXEL_FORMAT_RGBF32)
	{}

	// read a element, (cx,cy) is element coordination
	virtual bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const override;
	// maybe normalized [0, 1] before return.
	virtual bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, float& Value) const override;

	virtual bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, uint16_t& Value) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float Value) override;
	virtual void Clear(float R, float G, float B, float A) override;
};

class FSR_Buffer2D_RGBAF32 : public FSR_Buffer2D
{
public:
	FSR_Buffer2D_RGBAF32(uint32_t width, uint32_t height)
		: FSR_DepthBuffer(width, height, EPixelFormat::PIXEL_FORMAT_RGBAF32)
	{}

	// read a element, (cx,cy) is element coordination
	virtual bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const override;
	// maybe normalized [0, 1] before return.
	virtual bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const override;
	virtual bool Read(uint32_t cx, uint32_t cy, float& Value) const override;

	virtual bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, uint16_t& Value) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A) override;
	virtual bool Write(uint32_t cx, uint32_t cy, float Value) override;
	virtual void Clear(float R, float G, float B, float A) override;
};


// Helper Class
class FSR_Buffer2D_Helper
{
public:
	static std::shared_ptr<FSR_Buffer2D> CreateBuffer2D(uint32_t width, uint32_t height, EPixelFormat pixelformat);

	// load & save
	static std::shared_ptr<FSR_Buffer2D> LoadImageFile(const char* InFileName);
	static bool SaveImageFile(const std::shared_ptr<FSR_Buffer2D> InTexture, const char* InFileName);

};
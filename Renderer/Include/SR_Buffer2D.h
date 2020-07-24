// \brief
//	render target.
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <memory>

#include "SR_Buffer.h"


#define SR_INVALID_INDEX		(-1)

class FSR_Buffer2D
{
public:
	FSR_Buffer2D(uint32_t width, uint32_t height, uint32_t pixelformat);
	virtual ~FSR_Buffer2D();


	uint32_t Width() const { return _w; }
	uint32_t Height() const { return _h; }
	uint32_t Format() const { return _format; }
	bool IsValid() const { return _buffer && (_buffer->data() != nullptr); }

	// read a element, (cx,cy) is element coordination
	bool Read(uint32_t cx, uint32_t cy, uint8_t& R, uint8_t& G, uint8_t& B, uint8_t& A) const;
	bool Read(uint32_t cx, uint32_t cy, uint16_t& Value) const;

	// maybe normalized [0, 1] before return.
	bool Read(uint32_t cx, uint32_t cy, float& R, float& G, float& B, float& A) const;
	bool Read(uint32_t cx, uint32_t cy, float& Value);

	bool Write(uint32_t cx, uint32_t cy, uint8_t R, uint8_t G, uint8_t B, uint8_t A);
	bool Write(uint32_t cx, uint32_t cy, uint16_t& Value);
	bool Write(uint32_t cx, uint32_t cy, float R, float G, float B, float A);
	bool Write(uint32_t cx, uint32_t cy, float Value);

	// sample element
	bool Sample2DNearest(float u, float v, float& R, float& G, float& B, float& A);
	bool Sample2DLinear(float u, float v, float& R, float& G, float& B, float& A);

protected:
	uint32_t GetElementOffset(uint32_t cx, uint32_t cy) const;

protected:
	uint32_t _w, _h;
	uint32_t _bytes_perpixel;
	uint32_t _format;

	std::shared_ptr<FSR_Buffer> _buffer;
};


using FSR_DepthBuffer = FSR_Buffer2D;
using FSR_Texture2D = FSR_Buffer2D;


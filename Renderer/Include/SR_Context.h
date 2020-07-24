// \brief
//	renderer context.
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <memory>

#include <glm.hpp>
#include "SR_Buffer2D.h"


// target or texture format
enum EPixelFormat
{
	PIXEL_FORMAT_U16 = 0,  // uint16
	PIXEL_FORMAT_RGB888,   // r8-g8-b8
	PIXEL_FORMAT_RGBA8888, // r8-g8-b8-a8
	PIXEL_FORMAT_MAX
};

// max color render-target
#define MAX_MRT_COUNT	4


// viewport
struct FViewport
{
	FViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
		: _x(x), _y(y), _w(w), _h(h)
	{}

	uint32_t	_x, _y, _w, _h;
};

class FSR_Context
{
public:
	FSR_Context();
	virtual ~FSR_Context();

	
protected:
	FViewport	_viewport;
	
	glm::mat4x4 _modelview;
	glm::mat4x4 _modelview_inv;
	glm::mat4x4 _projection;
	glm::mat4x4 _projection_inv;
	
	std::shared_ptr<FSR_DepthBuffer>	_rt_depth;
	std::shared_ptr<FSR_Texture2D>		_rt_colors[MAX_MRT_COUNT];
};

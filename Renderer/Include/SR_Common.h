// \brief
//	sr_common types
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <memory>
#include <algorithm>

#include <glm.hpp>


#define SR_INVALID_INDEX		(-1)

// target or texture format
enum class EPixelFormat
{
	PIXEL_FORMAT_U16 = 0,  // uint16
	PIXEL_FORMAT_F32,      // float
	PIXEL_FORMAT_RGB888,   // r8-g8-b8
	PIXEL_FORMAT_RGBA8888, // r8-g8-b8-a8
	PIXEL_FORMAT_RGBF32,   // r-g-b (float3)
	PIXEL_FORMAT_RGBAF32,  // r-g-b-a (float4)
	PIXEL_FORMAT_MAX
};

// max color render-target
#define MAX_MRT_COUNT	4

// max attributes count
#define MAX_ATTRIBUTES_COUNT  4

// FRONT FACE
enum class EFrontFace
{
	FACE_CW = 0,
	FACE_CCW
};

// rectangle
struct FSR_Rectangle
{
	glm::vec2	_min;
	glm::vec2	_max;

	FSR_Rectangle()
		: _min(0,0)
		, _max(0,0)
	{}

	FSR_Rectangle(const glm::vec2& InMin, const glm::vec2& InMax)
		: _min(InMin)
		, _max(InMax)
	{}
};

// VS INPUT
struct FSRVertexShaderInput
{
	glm::vec3		_vertex;
	glm::vec3		_attributes[MAX_ATTRIBUTES_COUNT];
	uint32_t		_attri_cnt;
};

using FSRVertex = FSRVertexShaderInput;

// VS OUTPUT
struct FSRVertexShaderOutput
{
	glm::vec4		_vertex; // projected vertex
	glm::vec3		_attributes[MAX_ATTRIBUTES_COUNT];
	uint32_t		_attri_cnt;
};

// PS INPUT
struct FSRPixelShaderInput
{
	glm::vec3  _attributes[MAX_ATTRIBUTES_COUNT];
	uint32_t   _attri_cnt;
};

// PS OUTPUT
struct FSRPixelShaderOutput
{
	glm::vec4	_colors[MAX_MRT_COUNT];
	uint32_t	_color_cnt;
};


// look up bytes of a format
uint32_t LookupPixelFormatBytes(EPixelFormat InFormat);
void CopyVertexShaderOutput(FSRVertexShaderOutput& Dst, const FSRVertexShaderOutput& Src);


// \brief
//	sr_common types
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <glm.hpp>


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

// max attributes count
#define MAX_ATTRIBUTES_COUNT  4


// VS INPUT
struct FVertexShaderInput
{
	glm::vec3		_vertex;
	glm::vec3		_attributes[MAX_ATTRIBUTES_COUNT];
	uint32_t		_attri_cnt;
};


// VS OUTPUT
struct FVertexShaderOutput
{
	glm::vec4		_vertex;
	glm::vec3		_attributes[MAX_ATTRIBUTES_COUNT];
	uint32_t		_attri_cnt;
};

// PS INPUT
struct FPixelShaderInput
{
	float	_view_z;	// z in view coordination
	float	_inv_z;		// one over z
	glm::vec3  _attributes[MAX_ATTRIBUTES_COUNT];
	uint32_t   _attri_cnt;
};

// PS OUTPUT
struct FPixelShaderOutput
{
	glm::vec4	_colors[MAX_MRT_COUNT];
	uint32_t	_color_cnt;
};


// \brief
//		lights
//

#pragma once

#include "SR_Common.h"


// light types
enum class ESR_LightType
{
	LightType_Point,
	LightType_Directional,
	LightType_Max
};


// class light
class FSR_Light
{
public:
	FSR_Light() 
		: _type(ESR_LightType::LightType_Point)
		, _color(1,1,1)
		, _position(0,0,0,1)
	{}

public:
	ESR_LightType	_type;
	glm::vec3		_color;
	glm::vec4		_position;
};

// \brief
//	abstract material
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <memory>

#include "SR_Buffer2D.h"


// base material
class FSR_Material
{
public:
	FSR_Material() {}
	virtual ~FSR_Material() {}

public:
	std::shared_ptr<FSR_Texture2D>	_diffuse_tex;
	std::shared_ptr<FSR_Texture2D>	_normal_tex;
};


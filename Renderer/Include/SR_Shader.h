// \brief
//	shader class to perform as vs, ps.
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <memory>

#include "SR_Common.h"


class FSR_Context;

// vs shader
class FSR_VertexShader
{
public:
	virtual ~FSR_VertexShader() {}

	virtual void Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output) = 0;

};

// ps shader
class FSR_PixelShader
{
public:
	virtual ~FSR_PixelShader() {}

	virtual void Process(const FSR_Context& InContext, const FSRPixelShaderInput &Inpur, FSRPixelShaderOutput &Output) = 0;
};



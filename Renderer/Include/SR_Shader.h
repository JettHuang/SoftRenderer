// \brief
//	shader class to perform as vs, ps.
//

#pragma once

#include "SR_Common.h"


class FSR_Context;

// PS Context
struct FSRPixelShaderContext
{
	FMVPMatrixs	_mvps;
	class FSR_Material* _material;
};

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

	virtual void Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput &Input, FSRPixelShaderOutput &Output) = 0;
	virtual uint32_t OutputColorCount() { return 1; }
};


// simple vs & ps
class FSR_SimpleVertexShader : public FSR_VertexShader
{
public:
	virtual void Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output) override;
};

class FSR_SimplePixelShader : public FSR_PixelShader
{
public:
	virtual void Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output) override;
	virtual uint32_t OutputColorCount() override { return 1; }
};

// depth only vs & ps
class FSR_DepthOnlyVertexShader : public FSR_VertexShader
{
public:
	virtual void Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output) override;
};

class FSR_DepthOnlyPixelShader : public FSR_PixelShader
{
public:
	virtual void Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output) override;
	virtual uint32_t OutputColorCount() override { return 0; }
};

// diffuse mesh vs & ps
class FSR_SimpleMeshVertexShader : public FSR_VertexShader
{
public:
	virtual void Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output) override;
};

class FSR_SimpleMeshPixelShader : public FSR_PixelShader
{
public:
	virtual void Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output) override;
	virtual uint32_t OutputColorCount() override { return 1; }
};

// \brief
//		shader implementation
//

#include "SR_Shader.h"
#include "SR_Context.h"


// simple vs & ps with color
void FSR_SimpleVertexShader::Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output)
{
	Output._vertex = InContext._mvps._mvp * Input._vertex;
	Output._attributes = Input._attributes;
}

void FSR_SimplePixelShader::Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output)
{
	const glm::vec3 &V4 = Input._attributes._members[0];
	glm::vec4& color = Output._colors[0];
	color.r = V4.x;
	color.g = V4.y;
	color.b = V4.z;
	color.a = 1.f;
}

// depth only
void FSR_DepthOnlyVertexShader::Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output)
{
	Output._vertex = InContext._mvps._mvp * Input._vertex;
	Output._attributes._count = 0;
}

void FSR_DepthOnlyPixelShader::Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output)
{
	// do nothing
}


// mesh vs & ps with diffuse texture
void FSR_SimpleMeshVertexShader::Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output)
{
	Output._vertex = InContext._mvps._mvp * Input._vertex;
	Output._attributes = Input._attributes;
}

void FSR_SimpleMeshPixelShader::Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output)
{
#if 0
	glm::vec4 n = Input._attributes._members[0] * glm::vec4(0.5) + glm::vec4(0.5); // transform normal values [-1, 1] -> [0, 1] to visualize better
	Output._colors[0] = n;
	Output._color_cnt = 1;
#else
	const glm::vec4 &uv = Input._attributes._members[1];
	float RGBA[4];
	if (InContext._material && InContext._material->_diffuse_tex)
	{
		InContext._material->_diffuse_tex->Sample2DNearest(uv.x, uv.y, RGBA);
		//memcpy(RGBA, &(Input._attributes._members[0].r), sizeof(RGBA));
	}
	memcpy(&Output._colors[0].r, RGBA, sizeof(RGBA));
#endif
}

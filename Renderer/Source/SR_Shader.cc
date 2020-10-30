// \brief
//		shader implementation
//

#include "SR_Shader.h"
#include "SR_Context.h"


// simple vs & ps with color
void FSR_SimpleVertexShader::Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output)
{
	Output._vertex = InContext._mvp * Input._vertex;
	Output._attributes = Input._attributes;
}

void FSR_SimplePixelShader::Process(const FSR_Context& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output)
{
	const glm::vec3 &V3 = Input._attributes._members[0];
	glm::vec4& color = Output._colors[0];
	color.r = V3.x;
	color.g = V3.y;
	color.b = V3.z;
	color.a = 1.f;
}

// mesh vs & ps with diffuse texture
void FSR_SimpleMeshVertexShader::Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output)
{
	Output._vertex = InContext._mvp * Input._vertex;
	Output._attributes = Input._attributes;
}

void FSR_SimpleMeshPixelShader::Process(const FSR_Context& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output)
{
#if 0
	glm::vec3 n = Input._attributes._members[0] * glm::vec3(0.5) + glm::vec3(0.5); // transform normal values [-1, 1] -> [0, 1] to visualize better
	Output._colors[0] = glm::vec4(n, 1.f);
	Output._color_cnt = 1;
#else
	const glm::vec3 &uv = Input._attributes._members[1];
	float RGBA[4];
	if (InContext._material && InContext._material->_diffuse_tex)
	{
		InContext._material->_diffuse_tex->Sample2DNearest(uv.x, uv.y, RGBA);
	}
	memcpy(&Output._colors[0].r, RGBA, sizeof(RGBA));
#endif
}

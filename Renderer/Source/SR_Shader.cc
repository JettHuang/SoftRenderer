// \brief
//		shader implementation
//

#include "SR_Shader.h"
#include "SR_Context.h"


// simple vs & ps with color
void FSR_SimpleVertexShader::Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output)
{
	glm::mat4x4 mvp = InContext._projection * InContext._modelview;
	Output._vertex = mvp * glm::vec4(Input._vertex, 1.f);
	Output._attributes = Input._attributes;
}

void FSR_SimplePixelShader::Process(const FSR_Context& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output)
{
	Output._colors[0] = glm::vec4(Input._attributes._members[0], 1.f);
	Output._color_cnt = 1;
}

// mesh vs & ps with diffuse texture
void FSR_SimpleMeshVertexShader::Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output)
{
	glm::mat4x4 mvp = InContext._projection * InContext._modelview;
	Output._vertex = mvp * glm::vec4(Input._vertex, 1.f);
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
	float R = 0.f, G = 0.f, B = 0.f, A = 0.f;
	if (InContext._material->_diffuse_tex) 
	{
		InContext._material->_diffuse_tex->Sample2DNearest(uv.x, uv.y, R, G, B, A);
	}
	Output._colors[0] = glm::vec4(R, G, B, A);
	Output._color_cnt = 1;
#endif
}

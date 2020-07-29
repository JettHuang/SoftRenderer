// \brief
//		shader implementation
//

#include "SR_Shader.h"
#include "SR_Context.h"


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

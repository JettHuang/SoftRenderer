// DemoScene.cc
//
//

#include "DemoScene.h"

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))


void FDemoScene_Cubes::Init()
{
	_vs = std::make_shared<FSR_SimpleVertexShader>();
	_ps = std::make_shared<FSR_SimplePixelShader>();

	InitializeSceneObjects(_objects);
}

void FDemoScene_Cubes::DrawScene(FSR_Context& ctx, const glm::mat4x4& InViewMat, float InDeltaSeconds)
{
	ctx.SetShader(_vs, _ps);

	// objects
	static const glm::vec3 vertices[] =
	{
		{ 1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ 1.0f, 1.0f, -1.0f },
		{  1.0f, 1.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f },
	};
	// Use per-face colors
	static const glm::vec3 colors[] =
	{
		glm::vec3(0, 0, 1),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 1),
		glm::vec3(1, 1, 1),
		glm::vec3(1, 0, 1),
		glm::vec3(1, 1, 0)
	};
	static const uint32_t indices[] =
	{
		// 6 faces of cube * 2 triangles per-face * 3 vertices per-triangle = 36 indices
		1,3,0,
		7,5,4,
		4,1,0,
		5,2,1,
		2,7,3,
		0,7,4,
		1,2,3,
		7,6,5,
		4,5,1,
		5,6,2,
		2,6,7,
		0,3,7
	};

	// Loop over objects in the scene
	FSRVertex v0, v1, v2;
	v0._attributes._count = 1;
	v1._attributes._count = 1;
	v2._attributes._count = 1;

	for (size_t n = 0; n < _objects.size(); n++)
	{
		glm::mat4x4 modelview = InViewMat * _objects[n];
		ctx.SetModelViewMatrix(modelview);

		// Loop over triangles in a given object and rasterize them one by one
		for (uint32_t idx = 0; idx < ARR_SIZE(indices) / 3; idx++)
		{
			const glm::vec3& color = colors[indices[idx * 3] % 6];

			v0._vertex = vertices[indices[idx * 3]];
			v0._attributes._members[0] = color;
			v1._vertex = vertices[indices[idx * 3 + 1]];
			v1._attributes._members[0] = color;
			v2._vertex = vertices[indices[idx * 3 + 2]];
			v2._attributes._members[0] = color;

			FSR_Renderer::DrawTriangle(ctx, v0, v1, v2);
		} // end for idx
	} // end for n
}

// test multi-cubes
// https://github.com/NotCamelCase/RasterizationInOneWeekend/blob/master/Go3D.h
void FDemoScene_Cubes::InitializeSceneObjects(std::vector<glm::mat4>& objects)
{
	// Construct a scene of few cubes randomly positioned

	const glm::mat4 identity(1.f);

	glm::mat4 M0 = glm::translate(identity, glm::vec3(0, 0, 2.f));
	M0 = glm::rotate(M0, glm::radians(45.f), glm::vec3(0, 1, 0));

	glm::mat4 M1 = glm::translate(identity, glm::vec3(-3.75f, 0, 0));
	M1 = glm::rotate(M1, glm::radians(30.f), glm::vec3(1, 0, 0));

	glm::mat4 M2 = glm::translate(identity, glm::vec3(3.75f, 0, 0));
	M2 = glm::rotate(M2, glm::radians(60.f), glm::vec3(0, 1, 0));

	glm::mat4 M3 = glm::translate(identity, glm::vec3(0, 0, -2.f));
	M3 = glm::rotate(M3, glm::radians(90.f), glm::vec3(0, 0, 1));

	// Change the order of cubes being rendered, see how it changes with and without depth test
	objects.push_back(M0);
	objects.push_back(M1);
	objects.push_back(M2);
	objects.push_back(M3);
}


// DemoScene.cc
//
//

#include "DemoScene.h"

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))


void FDemoScene_Cubes::Init(FCamera &InCamera)
{
	_vs = std::make_shared<FSR_SimpleVertexShader>();
	_ps = std::make_shared<FSR_SimplePixelShader>();

	InitializeSceneObjects(_objects);

	const glm::vec3 eye(0, 3.75, 6.5);
	const glm::vec3 lookat(0, 0, 0);
	const glm::vec3 up(0, 1, 0);
	InCamera.Init(eye, up, 0, 0);
}

void FDemoScene_Cubes::DrawScene(FSR_Context& ctx, const glm::mat4x4& InViewMat, float InDeltaSeconds)
{
	ctx.SetShader(_vs, _ps);

	// objects
	static const glm::vec4 vertices[] =
	{
		{ 1.0f, -1.0f, -1.0f, 1.f },
		{ 1.0f, -1.0f, 1.0f, 1.f },
		{ -1.0f, -1.0f, 1.0f, 1.f },
		{ -1.0f, -1.0f, -1.0f, 1.f },
		{ 1.0f, 1.0f, -1.0f, 1.f },
		{  1.0f, 1.0f, 1.0f, 1.f },
		{ -1.0f, 1.0f, 1.0f, 1.f },
		{ -1.0f, 1.0f, -1.0f, 1.f },
	};
	// Use per-face colors
	static const glm::vec4 colors[] =
	{
		glm::vec4(0, 0, 1, 1),
		glm::vec4(0, 1, 0, 1),
		glm::vec4(0, 1, 1, 1),
		glm::vec4(1, 1, 1, 1),
		glm::vec4(1, 0, 1, 1),
		glm::vec4(1, 1, 0, 1)
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

	static const glm::vec3 axies[] =
	{
		glm::vec3(0, 1, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, 1)
	};
	static float rot_speed = 15.f;

	// Loop over objects in the scene
	FSRVertex v0, v1, v2;
	v0._attributes._count = 1;
	v1._attributes._count = 1;
	v2._attributes._count = 1;

	float delt_rot = rot_speed * InDeltaSeconds;
	for (size_t n = 0; n < _objects.size(); n++)
	{
		_object_rots[n] += delt_rot;
		glm::mat4x4 M0 = glm::rotate(_objects[n], glm::radians(_object_rots[n]), axies[n]);
		glm::mat4x4 modelview = InViewMat * M0;
		ctx.SetModelViewMatrix(modelview);

		// Loop over triangles in a given object and rasterize them one by one
		for (uint32_t idx = 0; idx < ARR_SIZE(indices) / 3; idx++)
		{
			const glm::vec4& color = colors[indices[idx * 3] % 6];

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
	glm::mat4 M1 = glm::translate(identity, glm::vec3(-3.75f, 0, 0));
	glm::mat4 M2 = glm::translate(identity, glm::vec3(3.75f, 0, 0));
	glm::mat4 M3 = glm::translate(identity, glm::vec3(0, 0, -2.f));
	
	// Change the order of cubes being rendered, see how it changes with and without depth test
	objects.push_back(M0);
	objects.push_back(M1);
	objects.push_back(M2);
	objects.push_back(M3);

	_object_rots.push_back(10.f);
	_object_rots.push_back(30.f);
	_object_rots.push_back(40.f);
	_object_rots.push_back(60.f);
}

//////////////////////////////////////////////////////////////////////////

void FDemoScene_Meshes::Init(FCamera& InCamera)
{
	_vs = std::make_shared<FSR_SimpleMeshVertexShader>();
	_ps = std::make_shared<FSR_SimpleMeshPixelShader>();

	// load mesh
	std::cerr << "Loading mesh .... " << std::endl;
	_SceneMesh = std::make_shared<FSR_Mesh>();
	if (!_SceneMesh->LoadFromObjFile("./Assets/sponza.obj", "./Assets/"))
	{
		std::cerr << "Load .obj scene failed." << std::endl;
	}
	std::cerr << "Loading mesh Finished.... " << std::endl;

	glm::vec3 eye(0, -8.5, -5);
	glm::vec3 lookat(20, 5, 1);
	glm::vec3 up(0, 1, 0);

	InCamera.Init(eye, up, 0, 0);
}

void FDemoScene_Meshes::DrawScene(FSR_Context& ctx, const glm::mat4x4& InViewMat, float InDeltaSeconds)
{
	if (_SceneMesh)
	{
		ctx.SetShader(_vs, _ps);

		ctx.SetModelViewMatrix(InViewMat);
		FSR_Renderer::DrawMesh(ctx, *_SceneMesh);
	}
}

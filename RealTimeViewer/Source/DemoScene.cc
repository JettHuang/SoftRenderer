// DemoScene.cc
//
//

#include "DemoScene.h"

#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))


void FDemoScene_Quad::Init(FCamera& InCamera)
{
	_vs = std::make_shared<FSR_SimpleVertexShader>();
	_ps = std::make_shared<FSR_SimplePixelShader>();

	const glm::vec3 eye(0, 0, 10);
	const glm::vec3 lookat(0, 0, 0);
	const glm::vec3 up(0, 1, 0);
	InCamera.Init(eye, up, 0, 0);
}

void FDemoScene_Quad::DrawScene(FSR_Context& ctx, const glm::mat4x4& InViewMat, float InDeltaSeconds)
{
	ctx.SetShader(_vs, _ps);

	FSRVertex v0, v1, v2, v3;

	v0._vertex = glm::vec4(10, -10, 1.0, 1.0);
	v0._attributes._members[0] = glm::vec4(1.0, 0.0, 0.0, 1.f);
	v0._attributes._count = 1;

	v1._vertex = glm::vec4(-10, 10, 1.0, 1.0);
	v1._attributes._members[0] = glm::vec4(0.0, 1.0, 0.0, 1.f);
	v1._attributes._count = 1;

	v2._vertex = glm::vec4(10, 10, 1.0, 1.f);
	v2._attributes._members[0] = glm::vec4(0.0, 0.0, 1.0, 1.f);
	v2._attributes._count = 1;

	v3._vertex = glm::vec4(-10, -10, 1.0, 1.f);
	v3._attributes._members[0] = glm::vec4(0.0, 1.0, 0.0, 1.f);
	v3._attributes._count = 1;

	ctx.SetCullFaceMode(EFrontFace::FACE_CW);
	ctx.SetModelViewMatrix(InViewMat);
	FSR_Renderer::DrawTriangle(ctx, v0, v1, v2);
	FSR_Renderer::DrawTriangle(ctx, v0, v3, v1);
}

//////////////////////////////////////////////////////////////////////////
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
	_depthonly_vs = std::make_shared<FSR_DepthOnlyVertexShader>();
	_depthonly_ps = std::make_shared<FSR_DepthOnlyPixelShader>();

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

	InCamera.Init(eye, up, -90, 0);
}

void FDemoScene_Meshes::DrawScene(FSR_Context& ctx, const glm::mat4x4& InViewMat, float InDeltaSeconds)
{
	if (_SceneMesh)
	{
		ctx.SetModelViewMatrix(InViewMat);

		// pass 1
		// ctx.SetShader(_depthonly_vs, _depthonly_ps);
		// FSR_Renderer::DrawMesh(ctx, *_SceneMesh);

		// pass 2
		ctx.SetShader(_vs, _ps);
		FSR_Renderer::DrawMesh(ctx, *_SceneMesh);
	}
}


//////////////////////////////////////////////////////////////////////////
// https://zhuanlan.zhihu.com/p/21961722
class FTeapotMaterial : public FSR_Material
{
public:
	FTeapotMaterial(float metalness, float smoothness)
		: _metalness(metalness)
		, _smoothness(smoothness)
	{}

	float _metalness;
	float _smoothness;
};

class FTeapot_VertexShader : public FSR_VertexShader
{
public:
	virtual void Process(const FSR_Context& InContext, const FSRVertexShaderInput& Input, FSRVertexShaderOutput& Output) override
	{
		Output._vertex = InContext._mvps._mvp * Input._vertex;
		Output._attributes = Input._attributes;
	}
};

class FTeapot_PixelShader : public FSR_PixelShader
{
public:
	FTeapot_PixelShader()
	{
		albedo = glm::vec3(1.0, 0.782, 0.344);
		kFb = glm::vec3(0.04, 0.04, 0.04);
		light_dir = glm::fastNormalize(glm::vec3(0.0f, 0.0f, 1.0f));
		light_color = glm::vec3(1.f, 1.f, 1.f);
		view_dir = glm::vec3(0, 0, 1.f);
		halfvector = glm::fastNormalize(view_dir + light_dir);
	}

	virtual uint32_t OutputColorCount() override { return 1; }

	virtual void Process(const FSRPixelShaderContext& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output) override
	{
#if 0
		glm::vec3 N = glm::fastNormalize(InContext._mvps._modelview_inv_t * Input._attributes._members[0]);
		glm::vec3 n = N * glm::vec3(0.5) + glm::vec3(0.5); // transform normal values [-1, 1] -> [0, 1] to visualize better
		Output._colors[0] = glm::vec4(n, 1.f);
		Output._color_cnt = 1;
#else
		FTeapotMaterial* material = dynamic_cast<FTeapotMaterial*>(InContext._material);
		float smoothness = material->_smoothness;
		float metalness = material->_metalness;

		diffuse = albedo * (1.f - metalness);
		specular = glm::mix(kFb, albedo, metalness);

		glm::vec3 N = glm::fastNormalize(InContext._mvps._modelview_inv_t * Input._attributes._members[0]);
		float NdotH = glm::clamp(glm::dot(N, halfvector), 0.f, 1.f);
		float HdotV = glm::clamp(glm::dot(halfvector, view_dir), 0.f, 1.f);
		float NdotL = glm::clamp(glm::dot(N, light_dir), 0.f, 1.f);
		glm::vec3 fresnel = fresnelSchlick(HdotV, specular);

		glm::vec3 color = (diffuse + ((smoothness + 2.f) / 8.f) * powf(NdotH, smoothness) * fresnel) * light_color * NdotL;
		Output._colors[0] = glm::vec4(color, 1.f);
#endif
	}

	glm::vec3 fresnelSchlick(float HdotV, const glm::vec3& F0) const
	{
		return F0 + (glm::vec3(1.f, 1.f, 1.f) - F0) * powf(1.f - HdotV, 5.f);
	}

protected:
	glm::vec3 albedo;
	glm::vec3 diffuse;
	glm::vec3 kFb;
	glm::vec3 specular;
	glm::vec3 light_dir;
	glm::vec3 light_color;
	glm::vec3 view_dir;
	glm::vec3 halfvector;
};


void FDemoScene_Teapot::Init(FCamera& InCamera)
{
	_vs = std::make_shared<FTeapot_VertexShader>();
	_ps = std::make_shared<FTeapot_PixelShader>();

	_materials[0] = std::make_shared<FTeapotMaterial>(0.f, 5.f);
	_materials[1] = std::make_shared<FTeapotMaterial>(0.3f, 5.f);
	_materials[2] = std::make_shared<FTeapotMaterial>(0.6f, 5.f);
	_materials[3] = std::make_shared<FTeapotMaterial>(0.8f, 5.f);
	_materials[4] = std::make_shared<FTeapotMaterial>(1.0f, 5.f);

	// load mesh
	std::cerr << "Loading mesh .... " << std::endl;
	_SceneMesh = std::make_shared<FSR_Mesh>();
	if (!_SceneMesh->LoadFromObjFile("./Assets/teapot.obj", "./Assets/"))
	{
		std::cerr << "Load .obj scene failed." << std::endl;
	}
	std::cerr << "Loading mesh Finished.... " << std::endl;

	glm::vec3 eye(0, 2.0, 2.0);
	glm::vec3 lookat(0, 0, 0);
	glm::vec3 up(0, 1, 0);

	InCamera.Init(eye, up, 0, -45);
	InCamera.MovementSpeed = 1.f;
}

void FDemoScene_Teapot::DrawScene(FSR_Context& ctx, const glm::mat4x4& InViewMat, float InDeltaSeconds)
{
	if (_SceneMesh)
	{
		float offsetx = -2.f;
		for (int i = 0; i < 5; ++i, offsetx += 1.f)
		{
			const glm::mat4 modelview = glm::translate(InViewMat, glm::vec3(offsetx, 0, 0));
			ctx.SetModelViewMatrix(modelview);
			ctx.SetMaterial(_materials[i]);

			ctx.SetShader(_vs, _ps);
			FSR_Renderer::DrawMesh(ctx, *_SceneMesh);
		} // end for i
	}
}

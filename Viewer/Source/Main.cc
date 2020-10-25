// \brief
//		main entry.
//

#include <cstdio>
#include <iostream>
#include <vector>
#include "SR_Headers.h"
#include "svpng.inc"


#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

// output ppm
void OutputPPM(const std::shared_ptr<FSR_Buffer2D>& InBuffer2D)
{
	int32_t image_width, image_height;

	if (!InBuffer2D)
	{
		std::cerr << "OutputPPM failed. buffer2d is null" << std::endl;
		return;
	}

	image_width = InBuffer2D->Width();
	image_height = InBuffer2D->Height();
	std::cerr << "photo size: " << image_width << ", " << image_height << std::endl;
	std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

	for (int32_t j = image_height - 1; j >= 0; --j)
	{
		for (int32_t i = 0; i < image_width; ++i)
		{
			float R, G, B, A;
			InBuffer2D->Read(i, j, R, G, B, A);
			
			// Replace NaN components with zero. See explanation in Ray Tracing: The Rest of Your Life.
			if (R != R) R = 0.0f;
			if (G != G) G = 0.0f;
			if (B != B) B = 0.0f;

			// Write the translated [0,255] value of each color component.
			std::cout << static_cast<int>(256 * glm::clamp(R, 0.0f, 0.999f)) << ' '
					  << static_cast<int>(256 * glm::clamp(G, 0.0f, 0.999f)) << ' '
					  << static_cast<int>(256 * glm::clamp(B, 0.0f, 0.999f)) << '\n';
		}
	} // end j
}

void OutputPNG(const std::shared_ptr<FSR_Buffer2D>& InBuffer2D)
{
	int32_t image_width, image_height;

	if (!InBuffer2D)
	{
		std::cerr << "OutputPNG failed. buffer2d is null" << std::endl;
		return;
	}

	image_width = InBuffer2D->Width();
	image_height = InBuffer2D->Height();
	if (image_width <= 0 || image_height <= 0)
	{
		std::cerr << "OutputPNG failed. image_width or image_height is invalid" << std::endl;
		return;
	}

	unsigned char* rgb = new unsigned char[image_width* image_height*3];
	if (!rgb) {
		return;
	}

	unsigned char* ptr = rgb;
	for (int32_t j = image_height - 1; j >= 0; --j)
	{
		for (int32_t i = 0; i < image_width; ++i)
		{
			float R, G, B, A;
			InBuffer2D->Read(i, j, R, G, B, A);

			// Replace NaN components with zero. See explanation in Ray Tracing: The Rest of Your Life.
			if (R != R) R = 0.0f;
			if (G != G) G = 0.0f;
			if (B != B) B = 0.0f;

			// Write the translated [0,255] value of each color component.
			*ptr++ = static_cast<int>(256 * glm::clamp(R, 0.0f, 0.999f));
			*ptr++ = static_cast<int>(256 * glm::clamp(G, 0.0f, 0.999f));
			*ptr++ = static_cast<int>(256 * glm::clamp(B, 0.0f, 0.999f));
		}
	} // end j

	FILE* fp = fopen("output.png", "wb");
	if (fp)
	{
		svpng(fp, image_width, image_height, rgb, 0);
		fclose(fp);
	}

	delete[] rgb;
}


void OutputImage(const std::shared_ptr<FSR_Buffer2D>& InBuffer2D)
{
#if 0
	OutputPPM(InBuffer2D);
#else
	OutputPNG(InBuffer2D);
#endif
}

// test simple triangle
void Example_SingleTriangle()
{
	FSR_Context ctx;
	std::shared_ptr<FSR_VertexShader> vs = std::make_shared<FSR_SimpleVertexShader>();
	std::shared_ptr<FSR_PixelShader> ps = std::make_shared<FSR_SimplePixelShader>();

	ctx.SetRenderTarget(600, 600, 1);
	ctx.SetViewport(0, 0, 600, 600);
	ctx.SetCullFaceMode(EFrontFace::FACE_CW);
	ctx.SetShader(vs, ps);
	ctx.ClearRenderTarget(glm::vec4(0, 0, 0, 0));

	FSRVertex v0, v1, v2, v3;

	v0._vertex = glm::vec3(-0.5, -0.5, 1.0);
	v0._attributes._members[0] = glm::vec3(1.0, 0.0, 0.0);
	v0._attributes._count = 1;

	v1._vertex = glm::vec3(-0.5, 0.5, 1.0);
	v1._attributes._members[0] = glm::vec3(0.0, 1.0, 0.0);
	v1._attributes._count = 1;

	v2._vertex = glm::vec3(0.5, 0.5, 1.0);
	v2._attributes._members[0] = glm::vec3(0.0, 0.0, 1.0);
	v2._attributes._count = 1;

	v3._vertex = glm::vec3(0.5, -0.5, 1.0);
	v3._attributes._members[0] = glm::vec3(0.0, 1.0, 0.0);
	v3._attributes._count = 1;

	FSR_Renderer::DrawTriangle(ctx, v0, v1, v2);
	FSR_Renderer::DrawTriangle(ctx, v0, v2, v3);

	OutputImage(ctx.GetColorBuffer(0));

#if SR_ENABLE_PERFORMACE_STAT
	ctx._stats->DisplayStats(std::cerr);
#endif
}

// test multi-cubes
// https://github.com/NotCamelCase/RasterizationInOneWeekend/blob/master/Go3D.h
void InitializeSceneObjects(std::vector<glm::mat4>& objects)
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

void Example_Multi_Cubes()
{
	const uint32_t kWidth = 1280u;
	const uint32_t kHeight = 720u;

	FSR_Context ctx;
	std::shared_ptr<FSR_VertexShader> vs = std::make_shared<FSR_SimpleVertexShader>();
	std::shared_ptr<FSR_PixelShader> ps = std::make_shared<FSR_SimplePixelShader>();

	ctx.SetRenderTarget(kWidth, kHeight, 1);
	ctx.SetViewport(0, 0, kWidth, kHeight);
	ctx.SetCullFaceMode(EFrontFace::FACE_CCW);
	ctx.SetShader(vs, ps);
	ctx.ClearRenderTarget(glm::vec4(0, 0, 0, 0));
	// setup camera
	// Build view & projection matrices (right-handed sysem)
	const float nearPlane = 0.1f;
	const float farPlane = 100.f;
	const glm::vec3 eye(0, 3.75, 6.5);
	const glm::vec3 lookat(0, 0, 0);
	const glm::vec3 up(0, 1, 0);

	const glm::mat4 view = glm::lookAt(eye, lookat, up);
	const glm::mat4 proj = glm::perspective(glm::radians(60.f), static_cast<float>(kWidth) / static_cast<float>(kHeight), nearPlane, farPlane);
	ctx.SetProjectionMatrix(proj);

	// objects
	glm::vec3 vertices[] =
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
	glm::vec3 colors[] =
	{
		glm::vec3(0, 0, 1),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 1),
		glm::vec3(1, 1, 1),
		glm::vec3(1, 0, 1),
		glm::vec3(1, 1, 0)
	};
	uint32_t indices[] =
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

	// Let's draw multiple objects
	std::vector<glm::mat4> objects;
	InitializeSceneObjects(objects);

	FPerformanceCounter PerfCounter;
	PerfCounter.StartPerf();

	// Loop over objects in the scene
	FSRVertex v0, v1, v2;
	v0._attributes._count = 1;
	v1._attributes._count = 1;
	v2._attributes._count = 1;
	for (size_t n = 0; n < objects.size(); n++)
	{
		glm::mat4x4 modelview = view * objects[n];
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

	std::cerr << " Draw Cubes Elapse microseconds: " << PerfCounter.EndPerf() << std::endl;

	OutputImage(ctx.GetColorBuffer(0));

#if SR_ENABLE_PERFORMACE_STAT
	ctx._stats->DisplayStats(std::cerr);
#endif
}

void Example_Mesh_Scene()
{
	const uint32_t kWidth = 1280u;
	const uint32_t kHeight = 720u;

	FSR_Context ctx;
	std::shared_ptr<FSR_VertexShader> vs = std::make_shared<FSR_SimpleMeshVertexShader>();
	std::shared_ptr<FSR_PixelShader> ps = std::make_shared<FSR_SimpleMeshPixelShader>();

	ctx.SetRenderTarget(kWidth, kHeight, 1);
	ctx.SetViewport(0, 0, kWidth, kHeight);
	ctx.SetCullFaceMode(EFrontFace::FACE_CCW);
	ctx.SetShader(vs, ps);
	ctx.ClearRenderTarget(glm::vec4(0, 0, 0, 0));
	// setup camera
	// Build view & projection matrices (right-handed sysem)
	float nearPlane = 0.125f;
	float farPlane = 5000.f;
	glm::vec3 eye(0, -8.5, -5);
	glm::vec3 lookat(20, 5, 1);
	glm::vec3 up(0, 1, 0);

	const glm::mat4 view = glm::lookAt(eye, lookat, up);
	const glm::mat4 modelview = glm::rotate(view, glm::radians(-30.f), glm::vec3(0, 1, 0));
	const glm::mat4 proj = glm::perspective(glm::radians(60.f), static_cast<float>(kWidth) / static_cast<float>(kHeight), nearPlane, farPlane);
	ctx.SetModelViewMatrix(modelview);
	ctx.SetProjectionMatrix(proj);

	// load mesh
	std::cerr << "Loading mesh .... " << std::endl;
	std::shared_ptr<FSR_Mesh> SceneMesh = std::make_shared<FSR_Mesh>();
	if (!SceneMesh->LoadFromObjFile("./Assets/sponza.obj", "./Assets/"))
	{
		std::cerr << "Load .obj scene failed." << std::endl;
	}

	std::cerr << "Start Draw Mesh ... " << std::endl;

	FPerformanceCounter PerfCounter;
	PerfCounter.StartPerf();
	FSR_Renderer::DrawMesh(ctx, *SceneMesh);
	std::cerr << " Draw Mesh Elapse microseconds: " << PerfCounter.EndPerf() << std::endl;

	// ouput image
	OutputImage(ctx.GetColorBuffer(0));

#if SR_ENABLE_PERFORMACE_STAT
	ctx._stats->DisplayStats(std::cerr);
#endif
}

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
		Output._vertex = InContext._mvp * glm::vec4(Input._vertex, 1.f);
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

	virtual void Process(const FSR_Context& InContext, const FSRPixelShaderInput& Input, FSRPixelShaderOutput& Output) override
	{
#if 0
		glm::vec3 N = glm::fastNormalize(InContext._modelview_inv_t * Input._attributes._members[0]);
		glm::vec3 n = N * glm::vec3(0.5) + glm::vec3(0.5); // transform normal values [-1, 1] -> [0, 1] to visualize better
		Output._colors[0] = glm::vec4(n, 1.f);
		Output._color_cnt = 1;
#else
		std::shared_ptr<FTeapotMaterial> material = std::dynamic_pointer_cast<FTeapotMaterial>(InContext._material);
		float smoothness = material->_smoothness;
		float metalness = material->_metalness;

		diffuse = albedo * (1.f - metalness);
		specular = glm::mix(kFb, albedo, metalness);

		glm::vec3 N = glm::fastNormalize(InContext._modelview_inv_t * Input._attributes._members[0]);
		float NdotH = glm::clamp(glm::dot(N, halfvector), 0.f, 1.f);
		float HdotV = glm::clamp(glm::dot(halfvector, view_dir), 0.f, 1.f);
		float NdotL = glm::clamp(glm::dot(N, light_dir), 0.f, 1.f);
		glm::vec3 fresnel = fresnelSchlick(HdotV, specular);
		
		glm::vec3 color = (diffuse + ((smoothness + 2.f) / 8.f) * powf(NdotH, smoothness) * fresnel) * light_color* NdotL;
		Output._colors[0] = glm::vec4(color, 1.f);
		Output._color_cnt = 1;
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

void Example_Teapot_Scene()
{
	const uint32_t kWidth = 1280u;
	const uint32_t kHeight = 720u;

	FSR_Context ctx;
	std::shared_ptr<FSR_VertexShader> vs = std::make_shared<FTeapot_VertexShader>();
	std::shared_ptr<FSR_PixelShader> ps = std::make_shared<FTeapot_PixelShader>();

	ctx.SetRenderTarget(kWidth, kHeight, 1);
	ctx.SetViewport(0, 0, kWidth, kHeight);
	ctx.SetCullFaceMode(EFrontFace::FACE_CCW);
	ctx.SetShader(vs, ps);
	ctx.ClearRenderTarget(glm::vec4(0, 0, 0, 0));
	// setup camera
	// Build view & projection matrices (right-handed sysem)
	float nearPlane = 0.125f;
	float farPlane = 5000.f;
	glm::vec3 eye(0, 2.0, 2.0);
	glm::vec3 lookat(0, 0, 0);
	glm::vec3 up(0, 1, 0);

	const glm::mat4 view = glm::lookAt(eye, lookat, up);
	const glm::mat4 proj = glm::perspective(glm::radians(60.f), static_cast<float>(kWidth) / static_cast<float>(kHeight), nearPlane, farPlane);
	ctx.SetModelViewMatrix(view);
	ctx.SetProjectionMatrix(proj);

	// load mesh
	std::cerr << "Loading mesh .... " << std::endl;
	std::shared_ptr<FSR_Mesh> SceneMesh = std::make_shared<FSR_Mesh>();
	if (!SceneMesh->LoadFromObjFile("./Assets/teapot.obj", "./Assets/"))
	{
		std::cerr << "Load .obj scene failed." << std::endl;
	}

	std::cerr << "Start Draw Mesh ... " << std::endl;

	std::shared_ptr<FTeapotMaterial> materials[5];
	materials[0] = std::make_shared<FTeapotMaterial>(0.f, 5.f);
	materials[1] = std::make_shared<FTeapotMaterial>(0.3f, 5.f);
	materials[2] = std::make_shared<FTeapotMaterial>(0.6f, 5.f);
	materials[3] = std::make_shared<FTeapotMaterial>(0.8f, 5.f);
	materials[4] = std::make_shared<FTeapotMaterial>(1.0f, 5.f);

	FPerformanceCounter PerfCounter;
	PerfCounter.StartPerf();

	float offsetx = -2.f;
	for (int i = 0; i < 5; ++i, offsetx +=1.f)
	{
		const glm::mat4 modelview = glm::translate(view, glm::vec3(offsetx, 0, 0));
		ctx.SetModelViewMatrix(modelview);
		ctx.SetMaterial(materials[i]);

		FSR_Renderer::DrawMesh(ctx, *SceneMesh);
	} // end for i

	
	std::cerr << " Draw Mesh Elapse microseconds: " << PerfCounter.EndPerf() << std::endl;

	// ouput image
	OutputImage(ctx.GetColorBuffer(0));

#if SR_ENABLE_PERFORMACE_STAT
	ctx._stats->DisplayStats(std::cerr);
#endif
}

int main()
{
	//Example_SingleTriangle();
	//Example_Multi_Cubes();
	//Example_Mesh_Scene();
	Example_Teapot_Scene();
}

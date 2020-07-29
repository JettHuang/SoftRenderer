// \brief
//		main entry.
//

#include <iostream>
#include "SR_Headers.h"


// output ppm
void OuputPPM(const std::shared_ptr<FSR_Buffer2D>& InBuffer2D)
{
	int32_t image_width, image_height;

	if (!InBuffer2D || !InBuffer2D->IsValid())
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

// simple triangle test
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

	FSRVertex v0, v1, v2;

	v0._vertex = glm::vec3(-0.5, -0.5, 1.0);
	v0._attributes._members[0] = glm::vec3(1.0, 0.0, 0.0);
	v0._attributes._count = 1;

	v1._vertex = glm::vec3(-0.5, 0.5, 1.0);
	v1._attributes._members[0] = glm::vec3(0.0, 1.0, 0.0);
	v1._attributes._count = 1;

	v2._vertex = glm::vec3(0.5, 0.5, 1.0);
	v2._attributes._members[0] = glm::vec3(0.0, 0.0, 1.0);
	v2._attributes._count = 1;

	FSR_Renderer::DrawTriangle(ctx, v0, v1, v2);

	OuputPPM(ctx.GetColorBuffer(0));
}

int main()
{
	Example_SingleTriangle();
}

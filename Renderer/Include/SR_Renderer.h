// \brief
//		render & rasterizer
//

#pragma once

#include "SR_Common.h"
#include "SR_Context.h"
#include "SR_Mesh.h"


// renderer
class FSR_Renderer
{
public:
	// draw a triangle
	static void DrawTriangle(const FSR_Context &InContext, const FSRVertex &InA, const FSRVertex&InB, const FSRVertex&InC);

	// draw a mesh
	// NOTE: this function will modify context's material.
	static void DrawMesh(FSR_Context& InContext, const FSR_Mesh &InMesh);

	static bool EnableMultiThreads();

	// Flush
	static void Flush(FSR_Context& InContext);

	static void TerminateMultiThreads(FSR_Context& InContext);
};

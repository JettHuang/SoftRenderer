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
	static void DrawMesh(const FSR_Context& InContext, const FSR_Mesh &InMesh);

protected:
	static uint32_t ClipAgainstNearPlane(const FSRVertexShaderOutput InVerts[3], FSRVertexShaderOutput OutVerts[4]);
	static void InterpolateVertex_Linear(const FSRVertexShaderOutput& P1, const FSRVertexShaderOutput& P2, float t, FSRVertexShaderOutput& OutVert);
	static void RasterizeTriangle(const FSR_Context& InContext, const FSRVertexShaderOutput& A, const FSRVertexShaderOutput& B, const FSRVertexShaderOutput& C);
};

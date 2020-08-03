// \brief
//		renderer
//


/*
** The following is a discussion of the math used to do edge clipping against
** a clipping plane.
**
**     P1 is an end point of the edge
**     P2 is the other end point of the edge
**
**     Q = (1 - t) *P1 + t*P2
**     That is, Q lies somewhere on the line formed by P1 and P2.
**
**     0 <= t <= 1
**     This constrains Q to lie between P1 and P2.
**
**     C is the plane equation for the clipping plane
**
**     D1 = P1 dot C
**     D1 is the distance between P1 and C.  If P1 lies on the plane
**     then D1 will be zero.  The sign of D1 will determine which side
**     of the plane that P1 is on, with negative being outside.
**
**     D2 = P2 dot C
**     D2 is the distance between P2 and C.  If P2 lies on the plane
**     then D2 will be zero.  The sign of D2 will determine which side
**     of the plane that P2 is on, with negative being outside.
**
** Because we are trying to find the intersection of the P1 P2 line
** segment with the clipping plane we require that:
**
**     Q dot C = 0
**
** Therefore
**
**     ((1-t)*P1 + t*P2) dot C = 0
**
**     (t*P2 + P1 - t*P1) dot C = 0
**
**     t*P2 dot C + P1 dot C - t*P1 dot C = 0
**
** Substituting D1 and D2 in
**
**     t*D2 + D1 - t*D1 = 0
**
** Solving for t
**
**     t = -D1 / (D2 - D1)
**
**     t = D1 / (D1 - D2)
*/

/* the view volume plane (NDC space):

	1. < 1,  0,  0, 1>    left
	2. <-1,  0,  0, 1>    right
	3. < 0,  0,  1, 1>    front
	4. < 0,  0, -1, 1>    back
	5. < 0, -1,  0, 1>    top
	6. < 0,  1,  0, 1>    bottom
*/

#include <algorithm>
#include "SR_Renderer.h"


inline void InterpolateVertex_Linear(const FSRVertexShaderOutput& P1, const FSRVertexShaderOutput& P2, float t, FSRVertexShaderOutput& OutVert)
{
	assert(P1._attributes._count == P2._attributes._count);

	OutVert._vertex = glm::mix(P1._vertex, P2._vertex, t);
	for (uint32_t i = 0; i < P1._attributes._count; i++)
	{
		OutVert._attributes._members[i] = glm::mix(P1._attributes._members[i], P2._attributes._members[i], t);
	}
	OutVert._attributes._count = P1._attributes._count;
}

static uint32_t ClipAgainstNearPlane(const FSRVertexShaderOutput InVerts[3], FSRVertexShaderOutput OutVerts[4])
{
	const FSRVertexShaderOutput* P1 = &InVerts[2];
	float D1 = P1->_vertex.z + P1->_vertex.w;

	uint32_t newVerts = 0;
	float t = 0.f;
	for (int i = 0; i < 3; ++i)
	{
		const FSRVertexShaderOutput* P2 = &InVerts[i];
		float D2 = P2->_vertex.z + P2->_vertex.w;

		if (D2 >= 0.f) // P2 is at the front of plane
		{
			if (D2 == 0.f || D1 >= 0.f) {
				OutVerts[newVerts]._vertex = P2->_vertex;
				OutVerts[newVerts]._attributes = P2->_attributes;
				newVerts++;
			}
			else // P1 is at the back of plane
			{
				t = D1 / (D1 - D2);
				InterpolateVertex_Linear(*P1, *P2, t, OutVerts[newVerts]);
				newVerts++;
				OutVerts[newVerts]._vertex = P2->_vertex;
				OutVerts[newVerts]._attributes = P2->_attributes;
				newVerts++;
			}
		}
		else // P2 is at the back of plane
		{
			if (D1 > 0.f) {
				t = D1 / (D1 - D2);
				InterpolateVertex_Linear(*P1, *P2, t, OutVerts[newVerts]);
				newVerts++;
			}
			// otherwise discards the P1, P2
		}

		P1 = P2;
		D1 = D2;
	} // end for i

	return newVerts;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/visibility-problem-depth-buffer-depth-interpolation
// https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes
struct FSR_RasterizedVert
{
	glm::vec3	_ndc_pos;
	glm::vec3	_screen_pos;
	float		_inv_w;
};

static bool intersect(const FSR_Rectangle& InA, const FSR_Rectangle& InB, FSR_Rectangle& Out)
{
	float minx = std::max(InA._minx, InB._minx);
	float miny = std::max(InA._miny, InB._miny);
	float maxx = std::min(InA._maxx, InB._maxx);
	float maxy = std::min(InA._maxy, InB._maxy);

	if (minx >= maxx || miny >= maxy) {
		return false;
	}

	Out._minx = minx;
	Out._miny = miny;
	Out._maxx = maxx;
	Out._maxy = maxy;
	return true;
}

// edge function
inline float EdgeFunction(const glm::vec3& A, const glm::vec3& B, const glm::vec3& P)
{
	return ((P.x - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x));
}

inline FSR_Rectangle BoundingboxOfTriangle(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2)
{
	float minx = V0.x, miny = V0.y, maxx = V0.x, maxy = V0.y;

	if (V1.x < minx) {
		minx = V1.x; 
	} else if (V1.x > maxx) { 
		maxx = V1.x; 
	}
	if (V1.y < miny) { 
		miny = V1.y; 
	} else if (V1.y > maxy) {
		maxy = V1.y; 
	}

	if (V2.x < minx) { 
		minx = V2.x; 
	} else if (V2.x > maxx) { 
		maxx = V2.x; 
	}
	if (V2.y < miny) { 
		miny = V2.y; 
	} else if (V2.y > maxy) { 
		maxy = V2.y; 
	}
	
	return FSR_Rectangle(minx, miny, maxx, maxy);
}


inline void DivideVertexAttributesByW(const FSRVertexAttributes& VInput, float InOneOverW, FSRVertexAttributes& PInput)
{
	for (uint32_t k = 0; k < VInput._count; ++k)
	{
		PInput._members[k] = VInput._members[k] * InOneOverW;
	}
	PInput._count = VInput._count;
}

inline void InterpolateVertexAttributes(const FSRVertexAttributes& V0, float w0,
	const FSRVertexAttributes& V1, float w1,
	const FSRVertexAttributes& V2, float w2,
	float Z,
	FSRVertexAttributes& Output)
{
	for (uint32_t k = 0; k < V0._count; ++k)
	{
		Output._members[k] = (V0._members[k] * w0 +
			V1._members[k] * w1 +
			V2._members[k] * w2) * Z;
	}

	Output._count = V0._count;
}

static void RasterizeTriangle(const FSR_Context& InContext, const FSRVertexShaderOutput& A, const FSRVertexShaderOutput& B, const FSRVertexShaderOutput& C)
{
#if SR_ENABLE_PERFORMACE_STAT
	FPerformanceCounter	PerfCounter;
	const std::shared_ptr<FSR_Performance>& Stats = InContext._stats;
	double elapse_microseconds = 0.0;
#endif

	const FSRVertexShaderOutput* ABC[3] = { &A, &B, &C };
	FSR_RasterizedVert screen[3];

	// perspective divide
	for (uint32_t i = 0; i < 3; ++i)
	{
		float inv_w = 1.f / ABC[i]->_vertex.w;
		screen[i]._inv_w = inv_w;
		screen[i]._ndc_pos.x = ABC[i]->_vertex.x * inv_w;
		screen[i]._ndc_pos.y = ABC[i]->_vertex.y * inv_w;
		screen[i]._ndc_pos.z = ABC[i]->_vertex.z * inv_w;
		screen[i]._screen_pos = InContext.NdcToScreenPostion(screen[i]._ndc_pos);
	}

	uint32_t iv0 = 0, iv1 = 1, iv2 = 2;
	float E012 = EdgeFunction(screen[0]._screen_pos, screen[1]._screen_pos, screen[2]._screen_pos);
	if (E012 > -1.f && E012 < 1.f)
	{
		return; // DISCARD!
	}

	bool bClockwise = (E012 >= 0.f);
	bool bClipped = bClockwise ^ (InContext._front_face == EFrontFace::FACE_CW);
	if (bClipped)
	{
		return;  // DISCARD!
	}

	if (!bClockwise) // exchange v to clockwise
	{
		iv1 = 2; iv2 = 1;
		E012 = -E012;
	}

	FSR_Rectangle bbox;
	const FSR_Rectangle bbox_triangle = BoundingboxOfTriangle(screen[0]._screen_pos, screen[1]._screen_pos, screen[2]._screen_pos);
	if (!intersect(bbox_triangle, InContext.ViewportRectangle(), bbox))
	{
		return; // DISCARD!
	}

	const FSR_RasterizedVert& SV0 = screen[iv0];
	const FSR_RasterizedVert& SV1 = screen[iv1];
	const FSR_RasterizedVert& SV2 = screen[iv2];

	FSRVertexAttributes VA0, VA1, VA2;
	DivideVertexAttributesByW(ABC[iv0]->_attributes, SV0._inv_w, VA0);
	DivideVertexAttributesByW(ABC[iv1]->_attributes, SV1._inv_w, VA1);
	DivideVertexAttributesByW(ABC[iv2]->_attributes, SV2._inv_w, VA2);

	const int32_t X0 = static_cast<int32_t>(floor(bbox._minx));
	const int32_t Y0 = static_cast<int32_t>(floor(bbox._miny));
	const int32_t X1 = static_cast<int32_t>(ceilf(bbox._maxx));
	const int32_t Y1 = static_cast<int32_t>(ceilf(bbox._maxy));

	glm::vec3 P(0, 0, 0);
	FSRPixelShaderInput PixelInput;
	FSRPixelShaderOutput PixelOutput;
	
	for (int32_t cy = Y0; cy < Y1; ++cy)
	{
		for (int32_t cx = X0; cx < X1; ++cx)
		{
			P.x = cx + 0.5f;
			P.y = cy + 0.5f;

			float E12 = EdgeFunction(SV1._screen_pos, SV2._screen_pos, P);
			float E20 = EdgeFunction(SV2._screen_pos, SV0._screen_pos, P);
			float E01 = EdgeFunction(SV0._screen_pos, SV1._screen_pos, P);
			if (E12 < 0.f || E20 < 0.f || E01 < 0.f)
			{
				// outside of the triangle
				continue;
			}

			glm::vec3 edge12 = SV2._screen_pos - SV1._screen_pos;
			glm::vec3 edge20 = SV0._screen_pos - SV2._screen_pos;
			glm::vec3 edge01 = SV1._screen_pos - SV0._screen_pos;

			// top-left rule:
			// the pixel or point is considered to overlap a triangle if it is either inside the triangle or 
			// lies on either a triangle top edge or any edge that is considered to be a left edge.
			if (E12 == 0.f) {
				if (!((edge12.y == 0.f && edge12.x > 0.f) || (edge12.y > 0.f))) {
					continue;
				}
			}
			if (E20 == 0.f) {
				if (!((edge20.y == 0.f && edge20.x > 0.f) || (edge20.y > 0.f))) {
					continue;
				}
			}
			if (E01 == 0.f) {
				if (!((edge01.y == 0.f && edge01.x > 0.f) || (edge01.y > 0.f))) {
					continue;
				}
			}

			// perspective correct interpolate
			const float w0 = E12 / E012;
			const float w1 = E20 / E012;
			const float w2 = (1.f - w0 - w1); // fixed for (w0 + w1 + w2) != 1.0

			const float depth = w0 * SV0._screen_pos.z + w1 * SV1._screen_pos.z + w2 * SV2._screen_pos.z;
			const float W = 1.f / (w0 * SV0._inv_w + w1 * SV1._inv_w + w2 * SV2._inv_w);
			
#if SR_ENABLE_PERFORMACE_STAT
			PerfCounter.StartPerf();
#endif
			bool bPassDepth = InContext.DepthTestAndOverride(cx, cy, depth);
#if SR_ENABLE_PERFORMACE_STAT
			elapse_microseconds = PerfCounter.EndPerf();
			Stats->_depth_tw_count++;
			Stats->_depth_total_microseconds += elapse_microseconds;
#endif
			if (!bPassDepth)
			{
				continue;
			}

			// attributes
			InterpolateVertexAttributes(VA0, w0, VA1, w1, VA2, w2, W, PixelInput._attributes);

#if SR_ENABLE_PERFORMACE_STAT
			PerfCounter.StartPerf();
#endif
			InContext._ps->Process(InContext, PixelInput, PixelOutput);

#if SR_ENABLE_PERFORMACE_STAT
			elapse_microseconds = PerfCounter.EndPerf();
			Stats->_ps_invoke_count++;
			Stats->_ps_total_microseconds += elapse_microseconds;
#endif
			
#if SR_ENABLE_PERFORMACE_STAT
			PerfCounter.StartPerf();
#endif
			for (uint32_t k = 0; k < PixelOutput._color_cnt; ++k)
			{
				const std::shared_ptr<FSR_Texture2D>& rt = InContext._rt_colors[k];
				if (rt)
				{
					const glm::vec4& color = PixelOutput._colors[k];
					rt->Write(cx, cy, color.r, color.g, color.b, color.a);
				}
			} // end for k

#if SR_ENABLE_PERFORMACE_STAT
			elapse_microseconds = PerfCounter.EndPerf();
			Stats->_color_write_count += PixelOutput._color_cnt;
			Stats->_color_total_microseconds += elapse_microseconds;
#endif
		} //end cx
	} // end cy
}

//////////////////////////////////////////////////////////////////////////

inline bool IsOnNegtiveSideOf_LeftPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.x + V0.w) < 0.f) && ((V1.x + V1.w) < 0.f) && ((V2.x + V2.w) < 0.f);
}

inline bool IsOnNegtiveSideOf_RightPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w - V0.x) < 0.f) && ((V1.w - V1.x) < 0.f) && ((V2.w - V2.x) < 0.f);
}

inline bool IsOnNegtiveSideOf_FrontPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.z + V0.w) < 0.f) && ((V1.z + V1.w) < 0.f) && ((V2.z + V2.w) < 0.f);
}

inline bool IsOnNegtiveSideOf_BackPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w - V0.z) < 0.f) && ((V1.w - V1.z) < 0.f) && ((V2.w - V2.z) < 0.f);
}

inline bool IsOnNegtiveSideOf_TopPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w - V0.y) < 0.f) && ((V1.w - V1.y) < 0.f) && ((V2.w - V2.y) < 0.f);
}

inline bool IsOnNegtiveSideOf_BotPlane(const glm::vec4& V0, const glm::vec4& V1, const glm::vec4& V2)
{
	return ((V0.w + V0.y) < 0.f) && ((V1.w + V1.y) < 0.f) && ((V2.w + V2.y) < 0.f);
}

// draw a triangle
void FSR_Renderer::DrawTriangle(const FSR_Context& InContext, const FSRVertex& InA, const FSRVertex& InB, const FSRVertex& InC)
{
#if SR_ENABLE_PERFORMACE_STAT
	FPerformanceCounter	PerfCounter;
	const std::shared_ptr<FSR_Performance>& Stats = InContext._stats;
	double elapse_microseconds = 0.0;
#endif

	FSRVertexShaderOutput Verts[3];

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif

	InContext._vs->Process(InContext, InA, Verts[0]);
	InContext._vs->Process(InContext, InB, Verts[1]);
	InContext._vs->Process(InContext, InC, Verts[2]);

#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();

	Stats->_triangles_count++;
	Stats->_vertexes_count += 3;
	Stats->_vs_invoke_count += 3;
	Stats->_vs_total_microseconds += elapse_microseconds;
#endif

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif
	// frustum culling
	bool bOutsideOfVolume = false;
	{
		const glm::vec4& homogeneous_v0 = Verts[0]._vertex;
		const glm::vec4& homogeneous_v1 = Verts[1]._vertex;
		const glm::vec4& homogeneous_v2 = Verts[2]._vertex;

		bOutsideOfVolume =
			IsOnNegtiveSideOf_LeftPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_RightPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_FrontPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_BackPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_TopPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2) ||
			IsOnNegtiveSideOf_BotPlane(homogeneous_v0, homogeneous_v1, homogeneous_v2);
	}

#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();
	Stats->_check_inside_frustum_count++;
	Stats->_check_inside_frustum_microseconds += elapse_microseconds;
#endif

	if (bOutsideOfVolume)
	{
		return; // DISCARD!
	}

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif

	FSRVertexShaderOutput newVerts[4];
	const uint32_t verts_cnt = ClipAgainstNearPlane(Verts, newVerts);
	
#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();

	Stats->_clip_invoke_count++;
	Stats->_clip_total_microseconds += elapse_microseconds;
#endif

#if SR_ENABLE_PERFORMACE_STAT
	PerfCounter.StartPerf();
#endif

	if (verts_cnt == 3)
	{
		RasterizeTriangle(InContext, newVerts[0], newVerts[1], newVerts[2]);
	}
	else if (verts_cnt == 4)
	{
		RasterizeTriangle(InContext, newVerts[0], newVerts[1], newVerts[2]);
		RasterizeTriangle(InContext, newVerts[2], newVerts[3], newVerts[0]);
	}

#if SR_ENABLE_PERFORMACE_STAT
	elapse_microseconds = PerfCounter.EndPerf();

	Stats->_raster_invoked_count += (verts_cnt == 3) ? 1 : ((verts_cnt == 4) ? 2 : 0);
	Stats->_raster_total_microseconds += elapse_microseconds;
#endif
}


// draw a mesh
void FSR_Renderer::DrawMesh(FSR_Context& InContext, const FSR_Mesh& InMesh)
{
	const std::vector<FSRVertex> &VertexBuffer = InMesh._VertexBuffer;
	const std::vector<uint32_t>& IndexBuffer = InMesh._IndexBuffer;;
	const std::vector<std::shared_ptr<FSR_Material>>& Materials = InMesh._Materials;

	for (uint32_t k=0; k<InMesh._SubMeshes.size(); ++k)
	{
		const FSR_Mesh::FSR_SubMesh& subMesh = InMesh._SubMeshes[k];

		if (subMesh._MaterialIndex != SR_INVALID_INDEX) {
			InContext.SetMaterial(Materials[subMesh._MaterialIndex]);
		}
		
		// draw triangles
		const uint32_t triangleCount = subMesh._IndexCount / 3;
		for (uint32_t idx = 0; idx < triangleCount; idx++)
		{
			const FSRVertex& V0 = VertexBuffer[IndexBuffer[subMesh._IndexOffset + (idx * 3)]];
			const FSRVertex& V1 = VertexBuffer[IndexBuffer[subMesh._IndexOffset + (idx * 3 + 1)]];
			const FSRVertex& V2 = VertexBuffer[IndexBuffer[subMesh._IndexOffset + (idx * 3 + 2)]];

			DrawTriangle(InContext, V0, V1, V2);
		}

	} // end for k
}


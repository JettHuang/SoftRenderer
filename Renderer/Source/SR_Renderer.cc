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


static const glm::vec4 kLeftPlane(1,0,0,1);
static const glm::vec4 kRightPlane(-1,0,0,1);
static const glm::vec4 kFrontPlane(0,0,1,1);
static const glm::vec4 kBackPlane(0,0,-1,1);
static const glm::vec4 kTopPlane(0,-1,0,1);
static const glm::vec4 kBottomPlane(0,1,0,1);
static const glm::vec4* const kFrustumPlanes[] =
{
	&kLeftPlane,
	&kRightPlane,
	&kFrontPlane,
	&kBackPlane,
	&kTopPlane,
	&kBottomPlane
};

// draw a triangle
void FSR_Renderer::DrawTriangle(const FSR_Context& InContext, const FSRVertex& InA, const FSRVertex& InB, const FSRVertex& InC)
{
	FSRVertexShaderOutput Verts[3];

	InContext._vs->Process(InContext, InA, Verts[0]);
	InContext._vs->Process(InContext, InB, Verts[1]);
	InContext._vs->Process(InContext, InC, Verts[2]);

	// clipping
	for (uint32_t i=0; i<sizeof(kFrustumPlanes)/sizeof(kFrustumPlanes[0]); ++i)
	{
		const glm::vec4* Plane = kFrustumPlanes[i];
		bool bOutside = (glm::dot(*Plane, Verts[0]._vertex) < 0.f) && 
						(glm::dot(*Plane, Verts[1]._vertex) < 0.f) &&
						(glm::dot(*Plane, Verts[2]._vertex) < 0.f);

		if (bOutside) {
			return; // Discard this triangle
		}
	}

	FSRVertexShaderOutput newVerts[4];
	uint32_t verts_cnt = ClipAgainstNearPlane(Verts, newVerts);
	if (verts_cnt == 3)
	{
		RasterizeTriangle(InContext, newVerts[0], newVerts[1], newVerts[2]);
	}
	else if (verts_cnt == 4)
	{
		RasterizeTriangle(InContext, newVerts[0], newVerts[1], newVerts[2]);
		RasterizeTriangle(InContext, newVerts[2], newVerts[3], newVerts[0]);
	}
}

uint32_t FSR_Renderer::ClipAgainstNearPlane(const FSRVertexShaderOutput InVerts[3], FSRVertexShaderOutput OutVerts[4])
{
	const FSRVertexShaderOutput* P1 = &InVerts[2];
	float D1 = glm::dot(P1->_vertex, kFrontPlane);

	uint32_t newVerts = 0;
	float t = 0.f;
	for (int i=0; i<3; ++i)
	{
		const FSRVertexShaderOutput* P2 = &InVerts[i];
		float D2 = glm::dot(P2->_vertex, kFrontPlane);

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

void FSR_Renderer::InterpolateVertex_Linear(const FSRVertexShaderOutput& P1, const FSRVertexShaderOutput& P2, float t, FSRVertexShaderOutput& OutVert)
{
	assert(P1._attributes._count == P2._attributes._count);

	OutVert._vertex = glm::mix(P1._vertex, P2._vertex, t);
	for (uint32_t i = 0; i <P1._attributes._count; i++)
	{
		OutVert._attributes._members[i] = glm::mix(P1._attributes._members[i], P2._attributes._members[i], t);
	}
	OutVert._attributes._count = P1._attributes._count;
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

static bool intersect(const FSR_Rectangle& InA, const FSR_Rectangle& InB, FSR_Rectangle &Out)
{
	float minx = std::max(InA._min.x, InB._min.x);
	float miny = std::max(InA._min.y, InB._min.y);
	float maxx = std::min(InA._max.x, InB._max.x);
	float maxy = std::min(InA._max.y, InB._max.y);

	if (minx >= maxx || miny >= maxy)
	{
		return false;
	}

	Out._min = glm::vec2(minx, miny);
	Out._max = glm::vec2(maxx, maxy);
	return true;
}

// edge function
inline float EdgeFunction(const glm::vec3 &A, const glm::vec3 &B, const glm::vec3 &P)
{
	float E = (P.x - A.x) * (B.y - A.y) - (P.y - A.y) * (B.x - A.x);
	return E;
}

inline FSR_Rectangle BoundingboxOfTriangle(const FSR_RasterizedVert Verts[3])
{
	float x0 = Verts[0]._screen_pos.x;
	float y0 = Verts[0]._screen_pos.y;
	float x1 = Verts[1]._screen_pos.x;
	float y1 = Verts[1]._screen_pos.y;
	float x2 = Verts[2]._screen_pos.x;
	float y2 = Verts[2]._screen_pos.y;

	FSR_Rectangle rect;
	rect._min.x = std::min(std::min(x0, x1), x2);
	rect._min.y = std::min(std::min(y0, y1), y2);
	rect._max.x = std::max(std::max(x0, x1), x2);
	rect._max.y = std::max(std::max(y0, y1), y2);
	
	return rect;
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
										FSRVertexAttributes&Output)
{
	for (uint32_t k = 0; k < V0._count; ++k)
	{
		Output._members[k] = (V0._members[k] * w0 + 
							  V1._members[k] * w1 + 
							  V2._members[k] * w2) * Z;
	}

	Output._count = V0._count;
}

void FSR_Renderer::RasterizeTriangle(const FSR_Context& InContext, const FSRVertexShaderOutput& A, const FSRVertexShaderOutput& B, const FSRVertexShaderOutput& C)
{
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
	float E012 = EdgeFunction(screen[iv0]._screen_pos, screen[iv1]._screen_pos, screen[iv2]._screen_pos);
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
	if (!intersect(BoundingboxOfTriangle(screen), InContext.ViewportRectangle(), bbox))
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

	const int32_t X0 = static_cast<int32_t>(floor(bbox._min.x));
	const int32_t Y0 = static_cast<int32_t>(floor(bbox._min.y));
	const int32_t X1 = static_cast<int32_t>(floor(bbox._max.x));
	const int32_t Y1 = static_cast<int32_t>(floor(bbox._max.y));
	
	glm::vec3 P(0,0,0);
	FSRPixelShaderInput PixelInput;
	FSRPixelShaderOutput PixelOutput;
	for (int32_t cx = X0; cx <= X1; ++cx)
	{
		for (int32_t cy=Y0; cy <= Y1; ++cy)
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
			float w0 = E12 / E012;
			float w1 = E20 / E012;
			float w2 = E01 / E012;

			float depth = w0 * SV0._screen_pos.z + w1 * SV1._screen_pos.z + w2 * SV2._screen_pos.z;
			float Z = 1.f / (w0 * SV0._inv_w + w1 * SV1._inv_w + w2 * SV2._inv_w);
			bool bPassDepth = InContext.DepthTestAndOverride(cx, cy, depth);
			if (!bPassDepth)
			{
				continue;
			}

			// attributes
			InterpolateVertexAttributes(VA0, w0, VA1, w1, VA2, w2, Z, PixelInput._attributes);
			InContext._ps->Process(InContext, PixelInput, PixelOutput);
			for (uint32_t k=0; k<PixelOutput._color_cnt; ++k)
			{
				const std::shared_ptr<FSR_Texture2D>& rt = InContext._rt_colors[k];
				if (rt && rt->IsValid())
				{
					const glm::vec4& color = PixelOutput._colors[k];
					rt->Write(cx, cy, color.r, color.g, color.b, color.a);
				}
			} // end for k
		} //end cy
	} // end cx
}

// draw a mesh
void FSR_Renderer::DrawMesh(const FSR_Context& InContext, const FSR_Mesh& InMesh)
{

}


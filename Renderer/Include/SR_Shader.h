// \brief
//	shader class to perform as vs, ps.
//

#pragma once

#include <cstdint> // uint32_t
#include <cstdlib> // size_t
#include <cassert>
#include <memory>


// vs shader
class FRS_VertexShader
{
public:
	virtual ~FRS_VertexShader();

	virtual void Process()
};



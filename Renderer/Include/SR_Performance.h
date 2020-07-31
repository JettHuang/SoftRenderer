// \brief
//		Time Query On Windows Platform.
//

#pragma once

#include <stdint.h> // uint32_t

double appInitTiming();
double appSeconds();
double appMicroSeconds();
int64_t appCycles();

class FPerformanceCounter
{
public:
	FPerformanceCounter() : _timestamp(0.0)
	{}

	inline void StartPerf()
	{
		_timestamp = appMicroSeconds();
	}

	// return micro-seconds elapsed
	inline double EndPerf()
	{
		double _endstamp = appMicroSeconds();
		return _endstamp - _timestamp;
	}

private:
	double  _timestamp;
};


// soft-raster performance statistic
class FSR_Performance
{
public:
	FSR_Performance() 
		: _triangles_count(0)
		, _vertexes_count(0)
		, _vs_invoke_count(0)
		, _vs_total_seconds(0)
		, _clip_invoke_count(0)
		, _clip_total_seconds(0)
		, _raster_invoked_count(0)
		, _raster_total_seconds(0)
		, _ps_invoke_count(0)
		, _ps_total_seconds(0)
		, _depth_tw_count(0)
		, _depth_total_seconds(0)
		, _color_write_count(0)
		, _color_total_seconds(0)
	{}


	// triangles count
	uint32_t	_triangles_count;
	uint32_t	_vertexes_count;

	uint32_t	_vs_invoke_count;
	double		_vs_total_seconds;

	uint32_t	_clip_invoke_count;
	double		_clip_total_seconds;

	uint32_t	_raster_invoked_count;
	double		_raster_total_seconds;

	uint32_t	_ps_invoke_count;
	double		_ps_total_seconds;
	
	uint32_t	_depth_tw_count;  // depth test write
	double		_depth_total_seconds;

	uint32_t	_color_write_count;
	uint32_t	_color_total_seconds;
};

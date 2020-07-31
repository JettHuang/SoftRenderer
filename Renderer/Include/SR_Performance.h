// \brief
//		Time Query On Windows Platform.
//

#pragma once

#include <stdint.h> // uint32_t
#include <iostream>


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
	{
		Reset();
	}

	void Reset()
	{
		_triangles_count = 0;
		_vertexes_count = 0;
		_vs_invoke_count = 0;
		_vs_total_microseconds = 0;
		_check_inside_frustum_count = 0;
		_check_inside_frustum_microseconds = 0;
		_clip_invoke_count = 0;
		_clip_total_microseconds = 0;
		_raster_invoked_count = 0;
		_raster_total_microseconds = 0;
		_ps_invoke_count = 0;
		_ps_total_microseconds = 0;
		_depth_tw_count = 0;
		_depth_total_microseconds = 0;
		_color_write_count = 0;
		_color_total_microseconds = 0;
	}

	void DisplayStats(std::ostream& output)
	{
		output << "--------------------" << std::endl;
		output << "SR Performance Stats: \n" <<
			"_triangles_count = " << _triangles_count << std::endl <<
			"_vertexes_count  = " << _vertexes_count << std::endl <<
			"_vs_invoke_count = " << _vs_invoke_count << std::endl <<
			"_vs_total_microseconds = " << _vs_total_microseconds << std::endl <<
			"_check_inside_frustum_count = " << _check_inside_frustum_count << std::endl <<
			"_check_inside_frustum_microseconds = " << _check_inside_frustum_microseconds << std::endl <<
			"_clip_invoke_count = " << _clip_invoke_count << std::endl <<
			"_clip_total_microseconds = " << _clip_total_microseconds << std::endl <<
			"_raster_invoked_count = " << _raster_invoked_count << std::endl <<
			"_raster_total_microseconds = " << _raster_total_microseconds << std::endl <<
			"_ps_invoke_count = " << _ps_invoke_count << std::endl <<
			"_ps_total_microseconds = " << _ps_total_microseconds << std::endl <<
			"_depth_tw_count = " << _depth_tw_count << std::endl <<
			"_depth_total_microseconds = " << _depth_total_microseconds << std::endl <<
			"_color_write_count = " << _color_write_count << std::endl <<
			"_color_total_microseconds = " << _color_total_microseconds << std::endl;
	}

public:
	// triangles count
	uint32_t	_triangles_count;
	uint32_t	_vertexes_count;

	uint32_t	_vs_invoke_count;
	double		_vs_total_microseconds;

	uint32_t	_check_inside_frustum_count;
	double		_check_inside_frustum_microseconds;

	uint32_t	_clip_invoke_count;
	double		_clip_total_microseconds;

	uint32_t	_raster_invoked_count;
	double		_raster_total_microseconds;

	uint32_t	_ps_invoke_count;
	double		_ps_total_microseconds;

	uint32_t	_depth_tw_count;  // depth test write
	double		_depth_total_microseconds;

	uint32_t	_color_write_count;
	double		_color_total_microseconds;
};

// \brief
//  memory buffer
//

#pragma once

#include "SR_Common.h"


class FSR_Buffer
{
public:
	FSR_Buffer()
		: _bytes_array(nullptr)
		, _bytes_count(0)
	{
	}

	FSR_Buffer(uint32_t bytes)
		: _bytes_array(nullptr)
		, _bytes_count(0)
	{
		resize(bytes);
	}

	~FSR_Buffer()
	{
		delete[] _bytes_array;
		_bytes_count = 0;
		_bytes_array = nullptr;
	}

	const uint8_t* data() const { return _bytes_array; }
	uint8_t* data() { return _bytes_array; }
	// count of bytes.
	uint32_t length() const { return _bytes_count; }

	bool resize(uint32_t newcount)
	{
		delete[] _bytes_array;
		_bytes_array = nullptr;
		_bytes_count = newcount;

		if (_bytes_count)
		{
			_bytes_array = new uint8_t[_bytes_count];
		}
		assert(_bytes_array != nullptr);

		return _bytes_array != nullptr;
	}

protected:
	uint32_t _bytes_count;
	uint8_t *_bytes_array;
};

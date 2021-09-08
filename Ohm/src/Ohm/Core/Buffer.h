#pragma once

#include "Ohm/Core/Log.h"
#include <algorithm>
#include <memory>

namespace Ohm
{
	using byte = uint8_t;

	struct Buffer
	{
		void* Data;
		uint32_t Size;

		Buffer()
			:Data(nullptr), Size(0) { }

		Buffer(void* data, uint32_t size)
			:Data(data), Size(size) { }

		static Buffer CreateCopy(const void* data, uint32_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.Data, data, size);
			return buffer;
		}

		void Allocate(uint32_t size)
		{
			delete[] Data;
			Data = nullptr;

			if (size == 0)
				return;

			Data = new byte[size];
			Size = size;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		void ZeroInitialize()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		template<typename T>
		T& Read(uint32_t offset = 0)
		{
			return *(T*)((byte*)Data + offset);
		}

		void Write(void* data, uint32_t size, uint32_t offset = 0)
		{
			if (offset + size > Size)
			{
				OHM_CRITICAL("Buffer overflow!  Buffer has {} bytes allocated.  Requesting {} bytes with a {} byte offset.", Size, size, offset);
				return;
			}

			memcpy((byte*)Data + offset, data, size);
		}

		operator bool() const { return Data; }

		byte& operator[](int index)
		{
			return ((byte*)Data)[index];
		}

		byte operator[](int index) const
		{
			return ((byte*)Data)[index];
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}
	};
}
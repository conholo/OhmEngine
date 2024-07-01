#pragma once
#include <string.h>
#include "Ohm/Core/Assert.h"
using byte = uint8_t;

struct Buffer
{
	void* Data = nullptr;
	size_t Size = 0;

	Buffer() = default;
	Buffer(void* data, size_t size)
		:Data(data), Size(size) { }

	static Buffer Copy(const void* data, uint32_t size)
	{
		Buffer buffer;
		buffer.Allocate(size);
		memcpy(buffer.Data, data, size);
		return buffer;
	}

	void Allocate(size_t size)
	{
		delete[] static_cast<byte*>(Data);
		Data = nullptr;

		if (size == 0)
			return;

		Data = new byte[size];
		Size = size;
	}

	void Release()
	{
		delete[] static_cast<byte*>(Data);
		Data = nullptr;
		Size = 0;
	}

	void ZeroInitialize() const
	{
		if (!Data) return;
		memset(Data, 0, Size);
	}

	template<typename T>
	void Write(void* data, size_t size, size_t offset = 0) const
	{
		ASSERT(size + offset <= Size, "Buffer Overflow: Size + Offset must be less than allocated buffer size.");
		memcpy((byte*)Data + offset, data, size);
	}

	template<typename T>
	T* Read(size_t offset = 0)
	{
		return (T*)((byte*)Data + offset);
	}

	byte* ReadBytes(uint32_t size, uint32_t offset) const
	{
		ASSERT(offset + size <= Size, "Buffer overflow!");
		const auto buffer = new byte[size];
		memcpy(buffer, static_cast<byte*>(Data) + offset, size);
		return buffer;
	}

	byte& operator[](int index)
	{
		return ((byte*)Data)[index];
	}

	byte operator[](int index) const
	{
		return ((byte*)Data)[index];
	}

	operator bool() const
	{
		return Data;
	}
	
	template<typename T>
	T* As()
	{
		return (T*)Data;
	}

	void* Get() const
	{
		return Data;
	}
};
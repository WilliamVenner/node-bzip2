#pragma once

#define BZ_NO_STDIO

#include <node.h>
#include <nan.h>

extern "C"
{
#include "../lib/bzip2/bzlib.h"
}

#define AUTO_BUFFERING_THRESHOLD 1073741824 // 1 GiB
#define TYPE_ERROR_MSG "data was not a string, buffer, or array"

namespace NodeBzip2
{
	enum BufferingMode
	{
		Auto,
		Always,
		Never
	};

	v8::Local<v8::Value> convertError(int result)
	{
		switch (result)
		{
		case BZ_CONFIG_ERROR:
			return Nan::Error("BZ_CONFIG_ERROR");
		case BZ_PARAM_ERROR:
			return Nan::Error("BZ_PARAM_ERROR");
		case BZ_MEM_ERROR:
			return Nan::Error("BZ_MEM_ERROR");
		case BZ_OUTBUFF_FULL:
			return Nan::Error("BZ_OUTBUFF_FULL");
		case BZ_DATA_ERROR:
			return Nan::Error("BZ_DATA_ERROR");
		case BZ_DATA_ERROR_MAGIC:
			return Nan::Error("BZ_DATA_ERROR_MAGIC");
		case BZ_UNEXPECTED_EOF:
			return Nan::Error("BZ_UNEXPECTED_EOF");

		default:
			char buffer[50];
			std::snprintf(buffer, sizeof(buffer), "unknown error (code %d)", result);
			return Nan::Error(buffer);
		}
	}

	class DataSlice
	{
	public:
		const char *data;
		size_t length;

		static DataSlice Owned(const char *data, size_t length)
		{
			return DataSlice(data, length, false);
		}

		static DataSlice Borrowed(const char *data, size_t length)
		{
			return DataSlice(data, length, true);
		}

		static DataSlice Clone(const char *data, size_t length)
		{
			char *newData = new char[length];
			memcpy(newData, data, length);
			return DataSlice(newData, length, false);
		}

		~DataSlice()
		{
			if (!borrowed)
			{
				delete[] data;
			}
		}

		// Move only
		DataSlice(const DataSlice &) = delete;
		DataSlice &operator=(const DataSlice &) = delete;
		DataSlice &operator=(DataSlice &&) = delete;

		DataSlice(DataSlice &&other) noexcept
		{
			data = other.data;
			length = other.length;
			borrowed = other.borrowed;

			other.data = nullptr;
			other.length = 0;
			other.borrowed = true;
		}

	private:
		bool borrowed;

		DataSlice(const char *data, size_t length, bool borrowed) : data(data), length(length), borrowed(borrowed) {}
	};

	class Result
	{
	private:
		unsigned int bz_error;
		std::vector<char> data;

		Result(unsigned int bz_error, std::vector<char> data) : bz_error(bz_error), data(data) {}

	public:
		static Result ok(std::vector<char> data)
		{
			return Result(BZ_OK, data);
		}

		static Result error(unsigned int bz_error)
		{
			assert(bz_error != BZ_OK);
			return Result(bz_error, std::vector<char>());
		}

		bool hasError()
		{
			return bz_error != BZ_OK;
		}

		unsigned int getError()
		{
			return bz_error;
		}

		std::vector<char> *getData()
		{
			if (bz_error == BZ_OK)
			{
				return &data;
			}
			else
			{
				return nullptr;
			}
		}

		static void NodeGc(char *data, void *hint)
		{
			delete static_cast<Result *>(hint);
		}

		// Move only
		Result(const Result &) = delete;
		Result &operator=(const Result &) = delete;
		Result(Result &&) = default;
		Result &operator=(Result &&) = default;
	};

	class Options
	{
	public:
		bool hasError;

		Options() {}
		Options(const v8::Local<v8::Object> &options) : Options() {}

	protected:
		void throwTypeError(const char *msg)
		{
			Nan::ThrowTypeError(msg);
			hasError = true;
		}
	};

	template <typename T>
	struct Context
	{
		const char *data;
		size_t length;
		T options;
		std::string strData;
	};
}
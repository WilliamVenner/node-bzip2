#pragma once

#define BZ_NO_STDIO

#include <node.h>
#include <nan.h>

extern "C" {
	#include "../lib/bzip2/bzlib.h"
}

#define AUTO_BUFFERING_THRESHOLD 1073741824 // 1 GiB
#define INVALID_JS_TYPE 0xCAFE

void FreeStdVector(char *data, void *hint)
{
	delete static_cast<std::vector<char> *>(hint);
}

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
	case INVALID_JS_TYPE:
		return Nan::TypeError("data was not a string, buffer, or array");

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

class CompressionTaskData {
public:
	const char* data;
	size_t length;

	static CompressionTaskData Owned(const char* data, size_t length) {
		return CompressionTaskData(data, length, false);
	}

	static CompressionTaskData Borrowed(const char* data, size_t length) {
		return CompressionTaskData(data, length, true);
	}

	static CompressionTaskData Clone(const char* data, size_t length) {
		char* newData = new char[length];
		memcpy(newData, data, length);
		return CompressionTaskData(newData, length, false);
	}

	~CompressionTaskData() {
		if (!borrowed) {
			delete[] data;
		}
	}

	// Move only
	CompressionTaskData(const CompressionTaskData&) = delete;
	CompressionTaskData& operator=(const CompressionTaskData&) = delete;
	CompressionTaskData& operator=(CompressionTaskData&&) = delete;

	CompressionTaskData(CompressionTaskData&& other) noexcept {
		data = other.data;
		length = other.length;
		borrowed = other.borrowed;

		other.data = nullptr;
		other.length = 0;
		other.borrowed = true;
	}
private:
	bool borrowed;

	CompressionTaskData(const char* data, size_t length, bool borrowed) : data(data), length(length), borrowed(borrowed) {}
};

class CompressionTaskResult {
private:
	unsigned int bz_error;
	std::vector<char> data;

	CompressionTaskResult(unsigned int bz_error, std::vector<char> data) : bz_error(bz_error), data(data) {}

public:
	static CompressionTaskResult ok(std::vector<char> data) {
		return CompressionTaskResult(BZ_OK, data);
	}

	static CompressionTaskResult error(unsigned int bz_error) {
		assert(bz_error != BZ_OK);
		return CompressionTaskResult(bz_error, std::vector<char>());
	}

	bool hasError() {
		return bz_error != BZ_OK;
	}

	unsigned int getError() {
		return bz_error;
	}

	std::vector<char>* getData() {
		if (bz_error == BZ_OK) {
			return &data;
		} else {
			return nullptr;
		}
	}

	static void NodeGc(char *data, void *hint) {
		printf("NodeGc\n");
		delete static_cast<CompressionTaskResult *>(hint);
	}

	// Move only
	CompressionTaskResult(const CompressionTaskResult&) = delete;
	CompressionTaskResult& operator=(const CompressionTaskResult&) = delete;
	CompressionTaskResult(CompressionTaskResult&&) = default;
	CompressionTaskResult& operator=(CompressionTaskResult&&) = default;
};

class CompressionTaskOptions
{
public:
	bool hasError;

	CompressionTaskOptions() {}
	CompressionTaskOptions(const v8::Local<v8::Object> &options) : CompressionTaskOptions() {}

protected:
	void throwTypeError(const char *msg)
	{
		Nan::ThrowTypeError(msg);
		hasError = true;
	}
};

template <typename T>
struct CompressionTaskContext {
	const char* data;
	size_t length;
	T options;
	std::string strData;
};
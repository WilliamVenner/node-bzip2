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

void returnResult(int result)
{
	switch (result)
	{
	case INVALID_JS_TYPE:
		Nan::ThrowTypeError("data was not a string, buffer, or array");
		break;

	case BZ_CONFIG_ERROR:
		Nan::ThrowError("BZ_CONFIG_ERROR");
		break;
	case BZ_PARAM_ERROR:
		Nan::ThrowError("BZ_PARAM_ERROR");
		break;
	case BZ_MEM_ERROR:
		Nan::ThrowError("BZ_MEM_ERROR");
		break;
	case BZ_OUTBUFF_FULL:
		Nan::ThrowError("BZ_OUTBUFF_FULL");
		break;
	case BZ_DATA_ERROR:
		Nan::ThrowError("BZ_DATA_ERROR");
		break;
	case BZ_DATA_ERROR_MAGIC:
		Nan::ThrowError("BZ_DATA_ERROR_MAGIC");
		break;
	case BZ_UNEXPECTED_EOF:
		Nan::ThrowError("BZ_UNEXPECTED_EOF");
		break;

	default:
		char buffer[50];
		std::snprintf(buffer, sizeof(buffer), "unknown error (code %d)", result);
		Nan::ThrowError(buffer);
		break;
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

	static void NodeDelete(char *data, void *hint) {
		delete static_cast<CompressionTaskResult *>(hint);
	}
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
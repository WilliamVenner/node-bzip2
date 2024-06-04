#pragma once

#define BZ_NO_STDIO

#include <node.h>
#include <nan.h>

extern "C" {
	#include "../lib/bzip2/bzlib.h"
}

#define AUTO_BUFFERING_THRESHOLD 1073741824 // 1 GiB
#define INVALID_JS_TYPE 0xCAFE

enum BufferingMode
{
	Auto,
	Always,
	Never
};

void FreeStdString(char *data, void *hint)
{
	delete static_cast<std::string *>(hint);
}

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
		Nan::ThrowError("unknown error");
		break;
	}
}
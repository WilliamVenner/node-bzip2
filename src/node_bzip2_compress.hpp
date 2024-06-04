#include "node_bzip2_common.hpp"

class CompressionOptions
{
public:
	int level;
	BufferingMode bufMode;
	bool hasError;

	CompressionOptions()
	{
		level = 9;
		bufMode = BufferingMode::Auto;
		hasError = false;
	}

	CompressionOptions(const v8::Local<v8::Object> &options) : CompressionOptions()
	{
		if (Nan::Has(options, Nan::New("level").ToLocalChecked()).FromJust())
		{
			auto levelVal = Nan::Get(options, Nan::New("level").ToLocalChecked()).ToLocalChecked();
			if (levelVal->IsInt32())
			{
				level = Nan::To<int32_t>(levelVal).FromJust();
				level = std::max(1, std::min(9, level));
			}
			else
			{
				throwTypeError("level must be an integer");
				return;
			}
		}

		if (Nan::Has(options, Nan::New("buffering").ToLocalChecked()).FromJust())
		{
			auto bufferingVal = Nan::Get(options, Nan::New("buffering").ToLocalChecked()).ToLocalChecked();
			if (bufferingVal->IsString())
			{
				const char *buffering = (*Nan::Utf8String(bufferingVal));
				if (strcmp(buffering, "auto") == 0)
				{
					bufMode = BufferingMode::Auto;
				}
				else if (strcmp(buffering, "always") == 0)
				{
					bufMode = BufferingMode::Always;
				}
				else if (strcmp(buffering, "never") == 0)
				{
					bufMode = BufferingMode::Never;
				}
				else
				{
					throwTypeError("buffering must be 'auto', 'always', or 'never'");
					return;
				}
			}
			else
			{
				throwTypeError("buffering must be a string");
				return;
			}
		}
	}

private:
	void throwTypeError(const char *msg)
	{
		Nan::ThrowTypeError(msg);
		hasError = true;
	}
};

int CompressRawUnbuffered(const char *data, const size_t length, std::vector<char> &out, const int level)
{
	try
	{
		out.resize(length + length / 100 + 600);
	}
	catch (std::bad_alloc &)
	{
		return BZ_MEM_ERROR;
	}

	unsigned int outSize = out.size();

	int result = BZ2_bzBuffToBuffCompress(out.data(), &outSize, (char *)data, length, level, 0, 30);
	if (result != BZ_OK)
	{
		return result;
	}

	out.resize(outSize);
	out.shrink_to_fit();

	return BZ_OK;
}

int CompressRawBuffered(const char *data, const size_t length, std::vector<char> &out, const int level)
{
	bz_stream strm;
	strm.bzalloc = NULL;
	strm.bzfree = NULL;
	strm.opaque = NULL;

	int result = BZ2_bzCompressInit(&strm, level, 0, 30);
	if (result != BZ_OK)
	{
		return result;
	}

	strm.next_in = (char *)data;
	strm.avail_in = length;

	int action = BZ_RUN;
	char buffer[4096];
	do
	{
		strm.next_out = buffer;
		strm.avail_out = sizeof(buffer);

		result = BZ2_bzCompress(&strm, action);
		if (result != BZ_RUN_OK && result != BZ_STREAM_END)
		{
			BZ2_bzCompressEnd(&strm);
			return result;
		}
		else if (action == BZ_RUN && strm.avail_in <= 0)
		{
			action = BZ_FINISH;
		}

		out.insert(out.end(), buffer, buffer + sizeof(buffer) - strm.avail_out);
	} while (result != BZ_STREAM_END);

	result = BZ2_bzCompressEnd(&strm);

	return result;
}

int CompressRaw(const char *data, const size_t length, std::vector<char> &out, const CompressionOptions &options)
{
	if (options.bufMode == BufferingMode::Always || (options.bufMode == BufferingMode::Auto && length >= AUTO_BUFFERING_THRESHOLD))
	{
		return CompressRawBuffered(data, length, out, options.level);
	}
	else
	{
		return CompressRawUnbuffered(data, length, out, options.level);
	}
}

NAN_METHOD(Compress)
{
	if (info.Length() < 1)
	{
		Nan::ThrowError("expected at least 1 argument, but got 0");
		return;
	}

	CompressionOptions options;

	if (info.Length() >= 2 && info[1]->IsObject())
	{
		options = CompressionOptions(info[1].As<v8::Object>());
		if (options.hasError)
			return;
	}

	int result = INVALID_JS_TYPE;
	std::vector<char> *out = new std::vector<char>();

	if (info[0]->IsString())
	{
		std::string data = (*Nan::Utf8String(info[0]));
		result = CompressRaw(data.c_str(), data.size(), *out, options);
	}
	else if (node::Buffer::HasInstance(info[0]))
	{
		char *data = node::Buffer::Data(info[0]);
		size_t length = node::Buffer::Length(info[0]);
		result = CompressRaw(data, length, *out, options);
	}
	else if (info[0]->IsObject())
	{
		auto val = info[0].As<v8::Value>();

		if (val->IsArrayBuffer())
		{
			auto arrayBuffer = val.As<v8::ArrayBuffer>();
			val = v8::Uint8Array::New(arrayBuffer, 0, arrayBuffer->ByteLength());
		}

		if (val->IsTypedArray())
		{
			auto byteArray = val.As<v8::Uint8Array>();
			Nan::TypedArrayContents<char> bytes(byteArray);
			if (*bytes == nullptr)
			{
				Nan::ThrowTypeError("typed array was not initialized");
				return;
			}
			result = CompressRaw(*bytes, bytes.length(), *out, options);
		}
	}

	if (result == BZ_OK)
	{
		auto buffer = Nan::NewBuffer(out->data(), out->size(), FreeStdString, out);
		info.GetReturnValue().Set(buffer.ToLocalChecked());
	}
	else
	{
		returnResult(result);
	}
}
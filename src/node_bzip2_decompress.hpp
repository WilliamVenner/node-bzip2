#include "node_bzip2_common.hpp"

class DecompressionOptions
{
public:
	bool small;
	bool hasError;

	DecompressionOptions()
	{
		small = false;
		hasError = false;
	}

	DecompressionOptions(const v8::Local<v8::Object> &options) : DecompressionOptions()
	{
		if (Nan::Has(options, Nan::New("small").ToLocalChecked()).FromJust())
		{
			auto smallVal = Nan::Get(options, Nan::New("small").ToLocalChecked()).ToLocalChecked();
			if (smallVal->IsBoolean())
			{
				small = Nan::To<bool>(smallVal).FromJust();
			}
			else
			{
				throwTypeError("small must be a boolean");
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

int DecompressRaw(const char *data, const size_t length, std::vector<char> &out, const DecompressionOptions &options)
{
	bz_stream strm;
	strm.bzalloc = NULL;
	strm.bzfree = NULL;
	strm.opaque = NULL;

	int result = BZ2_bzDecompressInit(&strm, 0, options.small ? 1 : 0);
	if (result != BZ_OK)
	{
		return result;
	}

	strm.next_in = (char *)data;
	strm.avail_in = length;

	char buffer[4096];
	do
	{
		strm.next_out = buffer;
		strm.avail_out = sizeof(buffer);

		result = BZ2_bzDecompress(&strm);
		if (result != BZ_OK && result != BZ_STREAM_END)
		{
			BZ2_bzDecompressEnd(&strm);
			return result;
		}

		out.insert(out.end(), buffer, buffer + sizeof(buffer) - strm.avail_out);
	} while (result != BZ_STREAM_END);

	result = BZ2_bzDecompressEnd(&strm);

	return result;
}

NAN_METHOD(Decompress)
{
	if (info.Length() < 1)
	{
		Nan::ThrowError("expected at least 1 argument, but got 0");
		return;
	}

	DecompressionOptions options;

	if (info.Length() >= 2 && info[1]->IsObject())
	{
		options = DecompressionOptions(info[1].As<v8::Object>());
		if (options.hasError)
			return;
	}

	int result = INVALID_JS_TYPE;
	std::vector<char> *out = new std::vector<char>();

	if (node::Buffer::HasInstance(info[0]))
	{
		char *data = node::Buffer::Data(info[0]);
		size_t length = node::Buffer::Length(info[0]);
		result = DecompressRaw(data, length, *out, options);
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
			result = DecompressRaw(*bytes, bytes.length(), *out, options);
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
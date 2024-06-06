#include "node_bzip2_common.hpp"

class DecompressionOptions : public CompressionTaskOptions
{
public:
	bool small;

	DecompressionOptions()
	{
		small = false;
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
};

CompressionTaskResult DecompressRaw(CompressionTaskData data, const DecompressionOptions &options)
{
	bz_stream strm;
	strm.bzalloc = NULL;
	strm.bzfree = NULL;
	strm.opaque = NULL;

	int result = BZ2_bzDecompressInit(&strm, 0, options.small ? 1 : 0);
	if (result != BZ_OK)
	{
		return CompressionTaskResult::error(result);
	}

	strm.next_in = (char *)data.data;
	strm.avail_in = data.length;

	std::vector<char> out;

	char buffer[4096];
	do
	{
		strm.next_out = buffer;
		strm.avail_out = sizeof(buffer);

		result = BZ2_bzDecompress(&strm);
		if (result != BZ_OK && result != BZ_STREAM_END)
		{
			BZ2_bzDecompressEnd(&strm);
			return CompressionTaskResult::error(result);
		}

		out.insert(out.end(), buffer, buffer + sizeof(buffer) - strm.avail_out);
	} while (result != BZ_STREAM_END);

	result = BZ2_bzDecompressEnd(&strm);

	if (result == BZ_OK) {
		return CompressionTaskResult::ok(out);
	} else {
		return CompressionTaskResult::error(result);
	}
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

	char* data;
	size_t length;

	if (node::Buffer::HasInstance(info[0]))
	{
		data = node::Buffer::Data(info[0]);
		length = node::Buffer::Length(info[0]);
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

			data = *bytes;
			length = bytes.length();
		}
	}

	CompressionTaskResult result = DecompressRaw(CompressionTaskData::Borrowed(data, length), options);

	if (!result.hasError())
	{
		CompressionTaskResult* resultAlloc = new CompressionTaskResult(result);
		std::vector<char>* out = resultAlloc->getData();

		auto buffer = Nan::NewBuffer(out->data(), out->size(), CompressionTaskResult::NodeDelete, resultAlloc);
		info.GetReturnValue().Set(buffer.ToLocalChecked());
	}
	else
	{
		returnResult(result.getError());
	}
}
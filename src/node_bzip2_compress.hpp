#include "node_bzip2_common.hpp"

class CompressionOptions : public CompressionTaskOptions
{
public:
	int level;
	BufferingMode bufMode;

	CompressionOptions()
	{
		level = 9;
		bufMode = BufferingMode::Auto;
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
};

CompressionTaskResult CompressRawUnbuffered(CompressionTaskData task, const int level)
{
	std::vector<char> out;

	try
	{
		out = std::vector<char>(task.length + task.length / 100 + 600);
	}
	catch (std::bad_alloc &)
	{
		return CompressionTaskResult::error(BZ_MEM_ERROR);
	}

	unsigned int outSize = out.size();

	int result = BZ2_bzBuffToBuffCompress(out.data(), &outSize, (char*)task.data, task.length, level, 0, 30);
	if (result != BZ_OK)
	{
		return CompressionTaskResult::error(result);
	}

	out.resize(outSize);
	out.shrink_to_fit();

	return CompressionTaskResult::ok(out);
}

CompressionTaskResult CompressRawBuffered(CompressionTaskData task, const int level)
{
	bz_stream strm;
	strm.bzalloc = NULL;
	strm.bzfree = NULL;
	strm.opaque = NULL;

	int result = BZ2_bzCompressInit(&strm, level, 0, 30);
	if (result != BZ_OK)
	{
		return CompressionTaskResult::error(result);
	}

	strm.next_in = (char*)task.data;
	strm.avail_in = task.length;

	std::vector<char> out;

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
			return CompressionTaskResult::error(result);
		}
		else if (action == BZ_RUN && strm.avail_in <= 0)
		{
			action = BZ_FINISH;
		}

		out.insert(out.end(), buffer, buffer + sizeof(buffer) - strm.avail_out);
	} while (result != BZ_STREAM_END);

	result = BZ2_bzCompressEnd(&strm);

	return CompressionTaskResult::ok(out);
}

CompressionTaskResult CompressRaw(CompressionTaskData task, const CompressionOptions &options)
{
	if (options.bufMode == BufferingMode::Always || (options.bufMode == BufferingMode::Auto && task.length >= AUTO_BUFFERING_THRESHOLD))
	{
		return CompressRawBuffered(task, options.level);
	}
	else
	{
		return CompressRawUnbuffered(task, options.level);
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

	std::string strData;

	const char* data;
	size_t length;

	if (info[0]->IsString())
	{
		strData = (*Nan::Utf8String(info[0]));
		data = strData.c_str();
		length = strData.length();
	}
	else if (node::Buffer::HasInstance(info[0]))
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

	CompressionTaskResult result = CompressRaw(CompressionTaskData::Borrowed(data, length), options);

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
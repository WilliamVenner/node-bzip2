#pragma once

#include "node_bzip2_common.hpp"

namespace NodeBzip2
{
	class DecompressionOptions : public Options
	{
	public:
		bool small;

		DecompressionOptions()
		{
			small = false;
		}

#ifdef NODE_VERSION
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
#endif
	};

	Result DecompressRaw(const DataSlice &data, const DecompressionOptions &options)
	{
		bz_stream strm;
		strm.bzalloc = NULL;
		strm.bzfree = NULL;
		strm.opaque = NULL;

		int result = BZ2_bzDecompressInit(&strm, 0, options.small ? 1 : 0);
		if (result != BZ_OK)
		{
			return Result::error(result);
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
				return Result::error(result);
			}

			out.insert(out.end(), buffer, buffer + sizeof(buffer) - strm.avail_out);
		} while (result != BZ_STREAM_END);

		result = BZ2_bzDecompressEnd(&strm);

		if (result == BZ_OK)
		{
			return Result::ok(std::move(out));
		}
		else
		{
			return Result::error(result);
		}
	}

#ifdef NODE_VERSION
	bool DecompressMethod(Nan::NAN_METHOD_ARGS_TYPE &info, Context<DecompressionOptions> &context)
	{
		if (info.Length() < 1)
		{
			Nan::ThrowError("expected at least 1 argument, but got 0");
			return false;
		}

		if (info.Length() >= 2 && !info[1]->IsNullOrUndefined() && info[1]->IsObject())
		{
			context.options = DecompressionOptions(info[1].As<v8::Object>());
			if (context.options.hasError)
				return false;
		}

		if (!info[0]->IsNullOrUndefined())
		{
			if (node::Buffer::HasInstance(info[0]))
			{
				context.data = node::Buffer::Data(info[0]);
				context.length = node::Buffer::Length(info[0]);
				return true;
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
						return false;
					}

					context.data = *bytes;
					context.length = bytes.length();
					return true;
				}
			}
		}

		Nan::ThrowTypeError(TYPE_ERROR_MSG);
		return false;
	}

	NAN_METHOD(Decompress)
	{
		Context<DecompressionOptions> context;
		if (!DecompressMethod(info, context))
			return;

		DataSlice data = DataSlice::Borrowed(context.data, context.length);

		Result result = DecompressRaw(data, context.options);

		if (!result.hasError())
		{
			/*
			// https://stackoverflow.com/questions/65797684/node-add-on-nannewbuffer-causes-memory-leak

			Result *resultAlloc = new Result(std::move(result));
			std::vector<char> *out = resultAlloc->getData();

			auto buffer = Nan::NewBuffer(out->data(), out->size(), Result::NodeGc, resultAlloc);
			*/

			std::vector<char> *out = result.getData();
			auto buffer = Nan::CopyBuffer(out->data(), out->size());
			info.GetReturnValue().Set(buffer.ToLocalChecked());
		}
		else
		{
			Nan::ThrowError(convertError(result.getError()));
		}
	}
#endif
}
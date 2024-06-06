#include "node_bzip2_compress.hpp"
#include "node_bzip2_async.hpp"

namespace NodeBzip2
{
	class AsyncCompressTask : public AsyncCompressionTask<CompressionOptions>
	{
		using AsyncCompressionTask<CompressionOptions>::AsyncCompressionTask;

		void Execute()
		{
			result = CompressRaw(task, options);
		}
	};

	NAN_METHOD(CompressAsync)
	{
		Context<CompressionOptions> context;
		if (!CompressMethod(info, context))
			return;

		Nan::Callback *callback = new Nan::Callback(Nan::To<v8::Function>(info[2]).ToLocalChecked());

		try
		{
			DataSlice data = DataSlice::Clone(context.data, context.length);
			Nan::AsyncQueueWorker(new AsyncCompressTask(callback, std::move(data), context.options));
		}
		catch (std::bad_alloc &)
		{
			Nan::ThrowError(convertError(BZ_MEM_ERROR));
		}
	}
}
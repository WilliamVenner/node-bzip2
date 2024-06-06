#include "node_bzip2_decompress.hpp"
#include "node_bzip2_async.hpp"

namespace NodeBzip2
{
	class AsyncDecompressTask : public AsyncCompressionTask<DecompressionOptions>
	{
		using AsyncCompressionTask<DecompressionOptions>::AsyncCompressionTask;

		void Execute()
		{
			result = DecompressRaw(task, options);
		}
	};

	NAN_METHOD(DecompressAsync)
	{
		Context<DecompressionOptions> context;
		if (!DecompressMethod(info, context))
			return;

		Nan::Callback *callback = new Nan::Callback(Nan::To<v8::Function>(info[2]).ToLocalChecked());

		try
		{
			DataSlice data = DataSlice::Clone(context.data, context.length);
			Nan::AsyncQueueWorker(new AsyncDecompressTask(callback, std::move(data), context.options));
		}
		catch (std::bad_alloc &)
		{
			Nan::ThrowError(convertError(BZ_MEM_ERROR));
		}
	}
}
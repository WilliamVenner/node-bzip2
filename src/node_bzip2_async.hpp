#pragma once

#include "node_bzip2_common.hpp"

namespace NodeBzip2
{
	template <typename T>
	class AsyncCompressionTask : public Nan::AsyncWorker
	{
	public:
		AsyncCompressionTask(Nan::Callback *callback, DataSlice task, T options) : Nan::AsyncWorker(callback), task(std::move(task)), options(options), result(Result::ok(std::vector<char>())) {}
		~AsyncCompressionTask() {}

		void HandleOKCallback()
		{
			Nan::HandleScope scope;

			v8::Local<v8::Value> argv[2];

			if (!result.hasError())
			{
				Result *resultAlloc = new Result(std::move(result));
				std::vector<char> *out = resultAlloc->getData();

				auto buffer = Nan::NewBuffer(out->data(), out->size(), Result::NodeGc, resultAlloc);

				argv[0] = Nan::Null();
				argv[1] = buffer.ToLocalChecked();
			}
			else
			{
				argv[0] = convertError(result.getError());
				argv[1] = Nan::Null();
			}

			callback->Call(2, argv, async_resource);
		}

	protected:
		DataSlice task;
		T options;

		Result result;
	};
}
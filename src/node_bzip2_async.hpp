#pragma once

#include "node_bzip2_common.hpp"

template <typename T>
class AsyncCompressionTask : public Nan::AsyncWorker
{
public:
	AsyncCompressionTask(Nan::Callback *callback, CompressionTaskData task, T options) : Nan::AsyncWorker(callback), task(std::move(task)), options(options), result(CompressionTaskResult::ok(std::vector<char>())) {}
	~AsyncCompressionTask() {}

	void HandleOKCallback()
	{
		Nan::HandleScope scope;

		v8::Local<v8::Value> argv[2];

		if (!result.hasError())
		{
			CompressionTaskResult* resultAlloc = new CompressionTaskResult(std::move(result));
			std::vector<char>* out = resultAlloc->getData();

			auto buffer = Nan::NewBuffer(out->data(), out->size(), CompressionTaskResult::NodeGc, resultAlloc);

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
	CompressionTaskData task;
	T options;

	CompressionTaskResult result;
};
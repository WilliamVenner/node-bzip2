#include "node_bzip2_common.hpp"
#include "node_bzip2_compress.hpp"
#include "node_bzip2_decompress.hpp"

NAN_MODULE_INIT(Initialize)
{
	Nan::Set(target, Nan::New("compress").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Compress)).ToLocalChecked());
	Nan::Set(target, Nan::New("decompress").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(Decompress)).ToLocalChecked());
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
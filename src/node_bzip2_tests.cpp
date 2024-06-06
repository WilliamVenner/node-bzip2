#include <cstdio>

#include "node_bzip2_common.hpp"
#include "node_bzip2_compress.hpp"
#include "node_bzip2_decompress.hpp"

NodeBzip2::Result doCompress()
{
	const char *dataPtr = new char[13];
	size_t dataLength = 13;
	memcpy((void *)dataPtr, "Hello, world!", 13);

	NodeBzip2::CompressionOptions options;
	NodeBzip2::DataSlice data = NodeBzip2::DataSlice::Owned(dataPtr, dataLength);
	NodeBzip2::Result result = NodeBzip2::CompressRaw(data, options);

	return result;
}

NodeBzip2::Result doDecompress(NodeBzip2::Result cResult)
{
	std::vector<char> *out = cResult.getData();
	NodeBzip2::DataSlice data = NodeBzip2::DataSlice::Clone(out->data(), out->size());
	NodeBzip2::DecompressionOptions options;
	NodeBzip2::Result result = NodeBzip2::DecompressRaw(data, options);

	return result;
}

NodeBzip2::Result doCompressDecompress()
{
	NodeBzip2::Result cResult = doCompress();
	assert(!cResult.hasError());

	NodeBzip2::Result dResult = doDecompress(std::move(cResult));
	return dResult;
}

int doTest()
{
	NodeBzip2::Result result = doCompressDecompress();
	assert(!result.hasError());

	if (strncmp(result.getData()->data(), "Hello, world!", 13) == 0)
	{
		printf("Success\n");
		return 0;
	}
	else
	{
		printf("Failure\n");
		return 1;
	}
}

int main()
{
	return doTest();
}
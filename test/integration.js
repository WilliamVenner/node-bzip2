console.log('integration.js: Running');

let package;
try {
	package = require('./package.json');
} catch (e) {
	if (e.code === 'MODULE_NOT_FOUND') {
		package = require('../package.json');
	} else {
		throw e;
	}
}

const bzip2 = require(package.name === 'node-bzip2' ? '../src/node-bzip2.js' : 'node-bzip2');

{
	// Compress some data
	const compressedBytes = bzip2.compress('Hello, world!', { level: 9, buffering: 'auto' });

	// Decompress the data
	const decompressedBytes = bzip2.decompress(compressedBytes, { small: false });

	// Decode the decompressed data as a UTF-8 string
	const decompressed = (new TextDecoder('utf8')).decode(decompressedBytes);

	console.log(decompressed); // Hello, world!
}

(async () => {
	// Compress some data
	const compressedBytes = await bzip2.compressAsync('Hello, world!', { level: 9, buffering: 'auto' });

	// Decompress the data
	const decompressedBytes = await bzip2.decompressAsync(compressedBytes, { small: false });

	// Decode the decompressed data as a UTF-8 string
	const decompressed = (new TextDecoder('utf8')).decode(decompressedBytes);

	console.log(decompressed); // Hello, world!
})().then(() => {
	console.log('integration.js: OK');
	process.exit(0);
});
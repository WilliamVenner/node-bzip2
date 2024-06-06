console.log('async.js: Running');

const bzip2 = require('../src/node-bzip2');

async function doAsyncTest() {
	const compressedAsync = await bzip2.compressAsync('Hello world');
	const decompressedAsync = await bzip2.decompressAsync(compressedAsync);

	const decompressedText = (new TextDecoder('utf8')).decode(new Uint8Array(decompressedAsync));
	if (decompressedText !== 'Hello world') {
		throw new Error('Decompression failed');
	}
}

Promise.all([
	doAsyncTest(),
	doAsyncTest(),
	doAsyncTest(),
	doAsyncTest(),
]).then(() => {
	global.gc();
	global.gc();

	console.log('async.js: OK');
	process.exit(0);
});
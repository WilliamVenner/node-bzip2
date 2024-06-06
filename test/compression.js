console.log('compression.js: Running');

const bzip2 = require('../src/node-bzip2');

const input = 'This is a test string for bzip2 compression'.repeat(10000);

function testCompression(inputObj, encoding = 'utf8') {
	const compressed = bzip2.compress(inputObj);
	if (compressed.length === 0) {
		throw new Error('Compression failed');
	}

	const decompressed = bzip2.decompress(compressed);

	const decompressedText = (new TextDecoder(encoding)).decode(new Uint8Array(decompressed));

	if (input !== decompressedText) {
		throw new Error(`Decompression failed (input length: ${inputObj.length}, compressed length: ${compressed.length}, decompressed length: ${decompressed.length})`);
	}
}

testCompression(input);
testCompression(new Buffer.from(input, 'utf8'));
testCompression(new Uint8Array(Buffer.from(input, 'utf8')));
testCompression(new Uint16Array(Buffer.from(input, 'utf8')), 'utf-16le');

{
	const arrayBuffer = new ArrayBuffer(input.length);
	const view = new Uint8Array(arrayBuffer);
	for (let i = 0; i < input.length; i++) {
		view[i] = input.charCodeAt(i);
	}
	testCompression(arrayBuffer);
}

global.gc();
global.gc();

console.log('compression.js: OK');
process.exit(0);
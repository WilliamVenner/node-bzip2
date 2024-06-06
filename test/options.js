console.log('options.js: Running');

const bzip2 = require('../src/node-bzip2');

const compressed = bzip2.compress('Hello world');

function verify(compressed) {
	const decompressed = bzip2.decompress(compressed);
	const decompressedText = (new TextDecoder('utf8')).decode(new Uint8Array(decompressed));
	if (decompressedText !== 'Hello world') {
		throw new Error('Decompression failed');
	}
}

verify(bzip2.compress('Hello world', { level: 7, buffering: 'auto' }));
verify(bzip2.compress('Hello world', { level: 1, buffering: 'never' }));
verify(bzip2.compress('Hello world', { level: 9, buffering: 'always' }));
verify(bzip2.compress('Hello world', { level: 10 }));
verify(bzip2.compress('Hello world', { level: 0 }));
verify(bzip2.compress('Hello world', { level: -1 }));

bzip2.decompress(compressed, { small: false });
bzip2.decompress(compressed, { small: true });

try {
	bzip2.compress('Hello world', { buffering: 'blah' });
} catch (error) {
	if (!(error instanceof TypeError) || error.message !== "buffering must be 'auto', 'always', or 'never'") {
		throw error;
	}
}

try {
	bzip2.decompress(compressed, { small: 'small' });
} catch (error) {
	if (!(error instanceof TypeError) || error.message !== 'small must be a boolean') {
		throw error;
	}
}

try {
	bzip2.compress('Hello world', { buffering: 55 });
} catch (error) {
	if (!(error instanceof TypeError) || error.message !== 'buffering must be a string') {
		throw error;
	}
}

try {
	bzip2.compress('Hello world', { level: 'level' });
} catch (error) {
	if (!(error instanceof TypeError) || error.message !== 'level must be an integer') {
		throw error;
	}
}

try {
	bzip2.compress('Hello world', { level: 54.53454 });
} catch (error) {
	if (!(error instanceof TypeError) || error.message !== 'level must be an integer') {
		throw error;
	}
}

console.log('options.js: OK');
process.exit(0);
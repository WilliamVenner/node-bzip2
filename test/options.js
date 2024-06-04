const bzip2 = require('../src/node-bzip2');

const compressed = bzip2.compress('Hello world');

bzip2.compress('Hello world', { level: 7, buffering: 'auto' });
bzip2.compress('Hello world', { level: 1, buffering: 'never' });
bzip2.compress('Hello world', { level: 9, buffering: 'always' });
bzip2.compress('Hello world', { level: 10 });
bzip2.compress('Hello world', { level: 0 });
bzip2.compress('Hello world', { level: -1 });

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
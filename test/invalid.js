console.log('invalid.js: Running');

const bzip2 = require('../src/node-bzip2');

[bzip2.compress, bzip2.decompress].forEach(fn => {
	[undefined, 53].forEach(data => {
		try {
			fn(data);
		} catch (e) {
			if (!(e instanceof TypeError) || e.message !== "data was not a string, buffer, or array") {
				throw e;
			}
		}
	});
});

[bzip2.compress, bzip2.decompress].forEach(fn => {
	try {
		fn();
	} catch (e) {
		if (!(e instanceof Error) || e.message !== "expected at least 1 argument, but got 0") {
			throw e;
		}
	}
});

console.log('invalid.js: OK');
process.exit(0);
const bzip2 = require('../src/node-bzip2');

[bzip2.compress, bzip2.decompress].forEach(fn => {
	try {
		fn(53);
	} catch (e) {
		if (!(e instanceof TypeError) || e.message !== "data was not a string, buffer, or array") {
			throw e;
		}
	}
});

console.log('invalid.js: OK');
process.exit(0);
/**
 * NodeJS bindings for BZip2 (libbz2).
 * @module node-bzip2
 */

/**
* Configuration options for BZip2 compression.
* @typedef {Object} CompressionOptions
* @property {('auto'|'always'|'never')} [buffering] **`always`** **(default)** is somewhat slower but reallocates memory as needed (and therefore uses significantly less memory)
*
* **`never`** may be faster but requires an allocation roughly the same size as the input data
*
* **`auto`** will only reallocate memory as needed if the input is larger than 1 GiB.
* @property {Number} [level] Compression level from `1` to `9` **(default: `9`)**
*/

/**
* Configuration options for BZip2 decompression.
* @typedef {Object} DecompressionOptions
* @property {Boolean} [small] If **`true`**, the decompressor will use less memory but be slower **(default: `false`)**
*/

const bindings = require('bindings');
const nodeBzip2 = bindings('node_bzip2');

function promiseify(fn, data, options) {
	return new Promise((resolve, reject) => {
		fn(data, options, (err, ok) => {
			if (ok !== null) {
				resolve(ok);
			} else {
				reject(err);
			}
		});
	});
}

/**
 * Compresses the given data using BZip2.
 * @param {Buffer|string|Uint8Array|Uint8ClampedArray|Int16Array|Uint16Array|Int32Array|Uint32Array} data The data to be compressed.
 * @param {CompressionOptions} [options] Configuration for the compression.
 * @returns {Buffer} The compressed data.
 */
exports.compress = nodeBzip2.compress;

/**
 * Decompresses the given data using Bzip2.
 * @param {Buffer} data The data to be decompressed.
 * @param {DecompressionOptions} [options] Configuration for the decompression.
 * @returns {Buffer} The decompressed data.
 */
exports.decompress = nodeBzip2.decompress;

/**
 * Compresses the given data using BZip2 asynchronously.
 * @param {Buffer|string|Uint8Array|Uint8ClampedArray|Int16Array|Uint16Array|Int32Array|Uint32Array} data The data to be compressed.
 * @param {CompressionOptions} [options] Configuration for the compression.
 * @returns {Promise<Buffer>} The compressed data.
 */
exports.compressAsync = function (...args) {
	return promiseify(nodeBzip2.compressAsync, ...args);
}

/**
 * Decompresses the given data using Bzip2 asynchronously.
 * @param {Buffer} data The data to be decompressed.
 * @param {DecompressionOptions} [options] Configuration for the decompression.
 * @returns {Promise<Buffer>} The decompressed data.
 */
exports.decompressAsync = function (...args) {
	return promiseify(nodeBzip2.decompressAsync, ...args);
}
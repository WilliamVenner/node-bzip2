[![NPM](https://img.shields.io/npm/v/node-bzip2.svg)](https://www.npmjs.com/package/node-bzip2)

# node-bzip2

NodeJS bindings for BZip2.

This package will compile the BZip2 library from source and link against it, exposing functions for compressing and decompressing data using the BZip2 algorithm in NodeJS.

**This package does not work on the web and is designed for use in NodeJS only.**

## Installation

```bash
npm install node-bzip2 --save
```

## Usage

The package exposes two functions: `compress` and `decompress` (and their respective async versions `compressAsync` and `decompressAsync`).

Both functions can take a `string`, [`Buffer`](https://nodejs.org/api/buffer.html), or [typed array](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Typed_arrays) as input and return a [`Buffer`](https://nodejs.org/api/buffer.html) containing the compressed or decompressed data.

Additional options such as compression level and buffering behavior can be passed as an optional second argument, explained in the respective functions' JSDocs.

```javascript
const bzip2 = require('node-bzip2');

// Compress some data
const compressedBytes = bzip2.compress('Hello, world!', { level: 9, buffering: 'auto' });

// Decompress the data
const decompressedBytes = bzip2.decompress(compressedBytes);

// Decode the decompressed data as a UTF-8 string
const decompressed = (new TextDecoder('utf8')).decode(decompressedBytes);

console.log(decompressed); // Hello, world!
```

You can also use the async functions to compress and decompress data asynchronously:

```javascript
const bzip2 = require('node-bzip2');

// Compress some data
const compressedBytes = await bzip2.compressAsync('Hello, world!', { level: 9, buffering: 'auto' });

// Decompress the data
const decompressedBytes = await bzip2.decompressAsync(compressedBytes);

// Decode the decompressed data as a UTF-8 string
const decompressed = (new TextDecoder('utf8')).decode(decompressedBytes);

console.log(decompressed); // Hello, world!
```
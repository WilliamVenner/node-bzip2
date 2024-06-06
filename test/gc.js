console.log('gc.js: Running');

const bzip2 = require('../src/node-bzip2');

global.gc();
global.gc();

let initialMemoryUsage = process.memoryUsage().heapUsed;
let allocatedMemoryUsage;
let deallocatedMemoryUsage;
console.log('Initial memory usage: ', initialMemoryUsage);

{
	const entries = [];

	for (let i = 0; i < 100; i++) {
		entries.push(bzip2.compress('Hello, world!'));
	}

	global.gc();
	global.gc();

	allocatedMemoryUsage = process.memoryUsage().heapUsed;
	console.log('Memory usage after allocations: ', allocatedMemoryUsage);
}

global.gc();
global.gc();

deallocatedMemoryUsage = process.memoryUsage().heapUsed;
console.log('Memory usage after GC: ', deallocatedMemoryUsage);

console.log('Freed: ', initialMemoryUsage - deallocatedMemoryUsage);

console.log('gc.js: OK');
process.exit(0);
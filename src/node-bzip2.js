const bindings = require('bindings');
const nodeBzip2 = bindings('node_bzip2');

module.exports = {
    compress: nodeBzip2.compress,
	decompress: nodeBzip2.decompress
};
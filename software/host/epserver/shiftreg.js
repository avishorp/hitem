'use strict'

const util = require('util')

function SRBuffer(len) {
	this.buffer = new Buffer(len)
}


SRBuffer.prototype.lshift = function(b)
{
	let i;
	for(i=0; i < this.buffer.length-1; i++)
		this.buffer[i] = this.buffer[i+1]
	
	this.buffer[this.buffer.length-1] = b
	
	return this
}


module.exports = SRBuffer

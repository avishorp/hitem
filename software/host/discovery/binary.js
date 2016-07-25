'use strict'

module.exports = {
	UInt8: v => { const b = new Buffer(1); b.writeUInt8(v); return b },
	UInt16LE: v => { const b = new Buffer(2); b.writeUInt16LE(v); return b },
	UInt32LE: v => { const b = new Buffer(4); b.writeUInt32LE(v); return b },
	UInt16BE: v => { const b = new Buffer(2); b.writeUInt16BE(v); return b },
	UInt32BE: v => { const b = new Buffer(4); b.writeUInt32BE(v); return b },
	string: (v, l) => { const b = new Buffer(l); b.fill(0).write(v, 'utf-8'); return b }
}

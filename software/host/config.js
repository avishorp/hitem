
const stripJSONComments = require('strip-json-comments')
const fs = require('fs')
const path = require('path')

const configFilename = path.resolve(__dirname, 'config.json')
const configContent = fs.readFileSync(configFilename).toString()
console.log(configFilename)
console.log(configContent)
module.exports = JSON.parse(stripJSONComments(configContent))

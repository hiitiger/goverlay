/* eslint-disable @typescript-eslint/no-var-requires */
const { app } = require('electron')
const path = require('path')


function loadNativeLib() {
  const lib_path = path.join(__dirname, '../../../../electron-overlay')
  return require(lib_path)
}

module.exports = {
  loadNativeLib,
}

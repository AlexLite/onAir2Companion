const fs = require('fs')
const path = require('path')

const LOG_FILE = path.join(process.env.TEMP || 'C:\\Temp', 'onair-bridge-module.log')
function writeLog(msg) {
  try {
    fs.appendFileSync(LOG_FILE, `${new Date().toISOString()} ${msg}\n`)
  } catch (_) {}
}

writeLog('main.js startup')

try {
  require('./dist/index.js')
  writeLog('dist/index.js loaded')
} catch (err) {
  writeLog(`startup error: ${err && err.stack ? err.stack : String(err)}`)
  console.error('Failed to start onair-bridge module', err)
  process.exit(1)
}

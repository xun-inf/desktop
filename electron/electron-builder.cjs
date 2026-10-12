const path = require('node:path');
const fs = require('node:fs');

module.exports = {
  appId: 'com.zxb.live-streamer',
  productName: 'LIVE Streamer',
  executableName: 'live-streamer',
  directories: { output: '../build/installer' },
  electronDist: 'node_modules/electron/dist',
  asar: true,
  npmRebuild: false,
  files: ['dist/**/*', '!dist/**/*.map', 'src/preload/**/*.cjs', 'package.json'],
  extraFiles: [
    { from: '../build/bin/Release', to: '.', filter: ['media-service.exe', 'node-streampreview.node'] },
    { from: '../config/bin', to: '.', filter: ['**/*'] },
    { from: '../config', to: 'config', filter: ['*.ico'] },
  ],
  beforePack() {
    for (const name of ['media-service.exe', 'node-streampreview.node']) {
      if (!fs.existsSync(path.join(__dirname, '../build/bin/Release', name))) {
        throw new Error(`Missing Release binary: ${name}. Run npm run dist:win.`);
      }
    }
  },
  win: { target: [{ target: 'nsis', arch: ['x64'] }], icon: '../config/app.ico' },
  nsis: {
    // Internal payload; package-win.mjs embeds it in the branded setup executable.
    artifactName: 'live-streamer-${version}-windows-${arch}-payload.${ext}',
    differentialPackage: false,
    oneClick: false,
    perMachine: false,
    allowToChangeInstallationDirectory: true,
    createDesktopShortcut: true,
    createStartMenuShortcut: true,
    shortcutName: 'LIVE Streamer',
    runAfterFinish: false,
  },
};

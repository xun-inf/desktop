import { spawnSync } from 'node:child_process';
import { rmSync, readFileSync, readdirSync, statSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const electronDir = fileURLToPath(new URL('..', import.meta.url));
const root = path.resolve(electronDir, '..');
if (process.platform !== 'win32' || process.arch !== 'x64') {
  throw new Error('The installer requires Windows x64 with Visual Studio 2022 C++ tools.');
}
delete process.env.ELECTRON_RUN_AS_NODE;
process.env.ELECTRON_BUILDER_CACHE ??= path.join(root, 'build', 'electron-builder-cache');

function run(command, args) {
  const result = spawnSync(command, args, { cwd: electronDir, stdio: 'inherit', env: process.env });
  if (result.error) throw result.error;
  if (result.status !== 0) process.exit(result.status ?? 1);
}

// Use a separate CMake cache and static MSVC runtime for distributable binaries.
const nativeBuild = path.join(root, 'build', 'installer-core');
run('cmake', ['-S', path.join(root, 'core'), '-B', nativeBuild,
  '-G', 'Visual Studio 17 2022', '-A', 'x64', '-T', 'v143',
  '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded',
  `-DFETCHCONTENT_BASE_DIR=${path.join(nativeBuild, '_deps')}`]);
run('cmake', ['--build', nativeBuild, '--config', 'Release', '--parallel']);
// Remove stale files and Windows directory casing left by earlier builds.
const dist = path.resolve(electronDir, 'dist');
if (path.dirname(dist) !== path.resolve(electronDir)) throw new Error('Invalid dist directory');
rmSync(dist, { recursive: true, force: true });
run(process.execPath, [process.env.npm_execpath, 'run', 'build']);

const { build, Platform } = await import('electron-builder');
await build({
  projectDir: electronDir,
  config: path.join(electronDir, 'electron-builder.cjs'),
  targets: Platform.WINDOWS.createTarget('nsis'),
  publish: 'never',
});

const { UUID } = await import('builder-util-runtime');
const { version } = JSON.parse(readFileSync(path.join(electronDir, 'package.json'), 'utf8'));
const { default: configuration } = await import('../electron-builder.cjs');
const guid = UUID.v5(configuration.appId, UUID.parse('50e065bc-3134-11e6-9bab-38c9862bdaf3'));
const output = path.join(root, 'build', 'installer');
function directorySize(directory) {
  return readdirSync(directory, { withFileTypes: true }).reduce((total, entry) => {
    const file = path.join(directory, entry.name);
    return total + (entry.isDirectory() ? directorySize(file) : statSync(file).size);
  }, 0);
}
const setupBuild = path.join(root, 'build', 'installer-ui');
run('cmake', ['-S', path.join(root, 'installer'), '-B', setupBuild,
  '-G', 'Visual Studio 17 2022', '-A', 'x64', '-T', 'v143',
  `-DSETUP_VERSION=${version}`, `-DSETUP_GUID=${guid}`,
  `-DSETUP_PAYLOAD=${path.join(output, `live-streamer-${version}-windows-x64-payload.exe`)}`,
  `-DSETUP_REQUIRED_BYTES=${directorySize(path.join(output, 'win-unpacked')) + 50 * 1024 * 1024}`,
  `-DSETUP_OUTPUT_DIR=${output}`]);
run('cmake', ['--build', setupBuild, '--config', 'Release', '--parallel']);
// A blockmap from the previous plain-NSIS installer does not describe this executable.
rmSync(path.join(output, `live-streamer-${version}-windows-x64-setup.exe.blockmap`), { force: true });
console.log(`Installer: ${path.join(output, `live-streamer-${version}-windows-x64-setup.exe`)}`);

# node-bizmodules

Independent C++11 business modules for Electron Main. The sole fixture is `TestModule`;
no account, room, legacy configuration or media business is installed.

```js
const { createBizModules } = require('./node-bizmodules.node');
const host = createBizModules();
await host.initialize();
const subscription = host.testModule.subscribe('changed', value => console.log(value));
await host.testModule.echo('hello');       // Promise<string>
await host.testModule.publish('changed'); // synchronously emits changed; Promise<string>
subscription.dispose();                 // idempotent; detached dispose is safe
await host.shutdown();                  // terminal and idempotent
```

`getStatus()` returns `{ state: 'created' | 'ready' | 'closed',
approvedModules: ['TestModule'], subscriptions: number }`. Calls before initialize or
after shutdown reject; subscribe throws synchronously. Strings are limited to 32768
UTF-8 bytes. A native listener exception rejects its originating publish Promise.
Callbacks can dispose subscriptions or shut down the host while an event is emitted.
Native callback references are weak; the JavaScript disposer retains its callback and
host through GC-visible properties. Keep the subscription alive while listening.

- `src/runtime/module.*`: common lifecycle and safe event/subscription ownership.
- `src/runtime/module_host.*`: creates the static registry, coordinates lifecycle and status.
- `src/runtime/module_registry.*`: native `ModuleFactory` table, identity/property checks and assembly.
- `src/modules/test/`: the fixture owns its named methods, event contract and N-API binding.
- `src/addon.cpp`: root lifecycle methods and environment cleanup hook.

To add a module, create its own `src/modules/<name>/` directory and `Module` subclass,
implement all declared methods and its named N-API object, then append a factory in
`module_registry.cpp` and actual sources in `CMakeLists.txt`. Use the C++ class name
as module ID (`TestModule`) and camelCase as root property (`testModule`). Register
its own methods/events in Electron `main/bizModules/moduleRegistry.ts` using the
root property as the Electron IPC capability ID (`testModule`), bind named methods
and events in the module-local `contract.ts` / `ipc.ts`, and extend the static sandbox
preload API and public declarations together. Renderer types follow the actual facade.
Never add business entries to `MediaServiceContainer` or expose the native host lifecycle
to Renderer. New native sources must be UTF-8 with BOM and CRLF.

Electron uses the shared `ipc:invoke`, `ipc:subscribe`, `ipc:unsubscribe`, and
`ipc:event` channels. Main validates registered top-level page/frame identity and
capability/method/event whitelists. Cross-document navigation pauses delivery;
confirmed cancellation restores the same document, while a committed navigation,
renderer loss, destruction or shutdown clears its subscriptions. Preload multiplexes local
callbacks, so disposing one callback keeps the shared Main subscription until the last
callback is removed.

The complete process, ownership and extension guide is maintained in
[technical-architecture.md](../../docs/technical-architecture.md).

Build and reproduce the real addon + product Renderer verification (the verification
runner is a local ignored cache, not a file guaranteed by a clean checkout):

```powershell
python scripts/build.py --config Debug
python build/cache/verification/verify_bizmodules.py --config Debug
```

Evidence is written to a unique `build/logs/bizmodules-registry/<run>/` directory;
`latest-run.txt` points to the last successful run. It includes native/mechanism
logs, real product IPC assertions, `step_shot-renderer-pass.png`, and closed/zero-subscription
status recorded during natural product exit. Verification uses temporary probe windows
for raw invalid IPC and same-origin iframe tests; the product Main window uses its
actual staged preload and addon.

The full product verification exposed an existing media addon shutdown defect:
media-only Electron stop/quit crashed during TSFN/AsyncResource environment teardown,
while biz-only stop/quit exited normally. The minimal media cleanup correction releases
the status TSFN after its producer thread joins and ignores aborted callbacks with a
null environment. It preserves media behavior and enables natural product exit.

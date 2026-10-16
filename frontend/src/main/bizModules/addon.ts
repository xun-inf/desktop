import { createRequire } from 'node:module';

// node-bizmodules 原生契约的本地镜像；仅 Main 管理宿主生命周期。
export interface Disposable {
  dispose(): void;
}

export interface BizModulesStatus {
  state: 'created' | 'ready' | 'closed';
  approvedModules: string[];
  subscriptions: number;
}

export interface TestModuleApi {
  echo(value: string): Promise<string>;
  publish(value: string): Promise<string>;
  subscribe(event: 'changed', callback: (value: string) => void): Disposable;
}

export interface BizModulesApi {
  initialize(): Promise<void>;
  shutdown(): Promise<void>;
  getStatus(): BizModulesStatus;
  testModule: TestModuleApi;
}

const require = createRequire(import.meta.url);

export function loadBizModules(modulePath: string): BizModulesApi {
  const addon = require(modulePath) as object;
  const factory = Object.getOwnPropertyDescriptor(addon, 'createBizModules')?.value;
  if (typeof factory !== 'function') throw new Error('Invalid node-bizmodules factory');
  return factory.call(addon) as BizModulesApi;
}

/** Framework fixture only. No production business modules are registered. */
export interface BizStatus {
  state: 'created' | 'ready' | 'closed';
  approvedModules: readonly ['TestModule'];
  subscriptions: number;
}
export interface BizSubscription { dispose(): void; }
export interface TestModule {
  echo(value: string): Promise<string>;
  publish(value: string): Promise<string>;
  /** Synchronous JS-thread events; an exception rejects the originating publish. */
  subscribe(event: 'changed', callback: (value: string) => void): BizSubscription;
}
export interface BizModules {
  initialize(): Promise<void>;
  /** Idempotent terminal shutdown. Create a new host to restart. */
  shutdown(): Promise<void>;
  getStatus(): BizStatus;
  testModule: TestModule;
}
export function createBizModules(): BizModules;

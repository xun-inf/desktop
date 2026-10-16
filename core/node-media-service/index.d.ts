/// <reference types="node" />
export interface MediaServiceOptions {
  executable: string;
  pipeName?: string;
  logPath?: string;
  startupTimeoutMs?: number;
  /** Integer 100..15000; default 1500. Natural exit budget, not a success override. */
  shutdownTimeoutMs?: number;
  arguments?: string[];
}
export interface Status {
  connected: boolean;
  message: string;
  phase: 'idle' | 'starting' | 'handshaking' | 'ready' | 'disconnected' | 'stopped';
  pid: number;
  session: string;
  /** Native terminal result for trusted host diagnostics; phase stopped alone is not success. */
  readonly nativeClose: Readonly<NativeCloseResult>;
}
export interface NativeCloseResult {
  readonly processPresent: boolean;
  readonly forced: boolean;
  readonly exitCodeKnown: boolean;
  readonly waitResult: number;
  readonly exitCode: number;
  readonly success: boolean;
  readonly jobAccountingKnown: boolean;
  readonly jobActiveAtExit: number;
  readonly jobActiveAfterDrain: number;
}
export interface Disposable {
  /** Idempotent; dispose() unregisters the underlying event/status callback. */
  dispose(): void;
}
/** Texture handle bridge (原 node-streampreview 独立 addon，已并入本模块). */
export function duplicate(pid: number, sourceHandle: bigint): Buffer;
export function close(localHandle: Buffer): void;
export interface Rect {
  x: number;
  y: number;
  width: number;
  height: number;
}
export interface FrameEvent {
  /** preview id（业务自管理；不再有 key→id 门面）。 */
  id: number;
  /** 归还 token：传给 streamPreviewService.releaseVideoFrame(id, token)。 */
  token: bigint;
  /** 本进程已复制的 NT HANDLE Buffer；复制失败时为 undefined（方案 B：业务方负责归还帧）。 */
  handle?: Buffer;
  width: number;
  height: number;
  timestampUs: number;
}
export interface NativeWindowService {
  create(id: number, host: bigint, hostVisible: boolean): Promise<void>;
  refresh(id: number, host: bigint, hostVisible: boolean): Promise<void>;
  releaseAll(): Promise<void>;
  release(id: number): Promise<void>;
  setRect(id: number, x: number, y: number, width: number, height: number): Promise<void>;
  setVisible(id: number, visible: boolean): Promise<void>;
}
export interface StreamPreviewService {
  createStreamPreview(id: number): Promise<void>;
  releaseStreamPreview(id: number): Promise<void>;
  /** 归还帧（方案 B：业务方消费完 frame.handle 后调用）。 */
  releaseVideoFrame(id: number, token: bigint): Promise<void>;
  /** 订阅 frame 事件；单监听者，dispose() 反注册。 */
  subscribeFrame(callback: (frame: FrameEvent) => void): Disposable;
}
export interface MediaService {
  initialize(): Promise<void>;
  getStatus(): Status;
  subscribeStatus(callback: (status: Status) => void): Disposable;
  /** root 属性名 = camel(serviceId)，由 services/service_registry 统一挂载。 */
  nativeWindowService: NativeWindowService;
  streamPreviewService: StreamPreviewService;
  /** Terminal cleanup. */
  stop(): Promise<void>;
}
export function createMediaService(options: MediaServiceOptions): MediaService;

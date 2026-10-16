import type { BigoMainApi, BigoMediaServiceApi, BigoBizModulesApi } from '../preload/preload';
export type { MediaStatus, NativeWindowRect, StreamPreviewFrameMeta } from '../main/mediaService/contract';
export type StreamPreviewSubscribe = BigoMediaServiceApi['streamPreviewService']['subscribe'];
declare global {
  interface Window {
    bigoMain: BigoMainApi;
    bigoMediaService: BigoMediaServiceApi;
    bigoBizModules: BigoBizModulesApi;
  }
}

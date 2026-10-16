import type { WindowControl, WindowOptions } from '../../common/page-contract';

// 页面窗口门面：业务代码统一经此访问 bigoMain.windows。
export const windowsService = {
  showWindow(options: WindowOptions): Promise<string> { return window.bigoMain.windows.showWindow(options); },
  closeWindow(id: string): Promise<void> { return window.bigoMain.windows.closeWindow(id); },
  closeAllWindow(): Promise<void> { return window.bigoMain.windows.closeAllWindow(); },
  control(action: WindowControl): Promise<{ maximized: boolean }> { return window.bigoMain.windows.control(action); },
  ready(): void { window.bigoMain.windows.ready(); },
  getState(): Promise<{ maximized: boolean }> { return window.bigoMain.windows.getState(); },
  onState(callback: (state: { maximized: boolean }) => void): () => void { return window.bigoMain.windows.onState(callback); },
};

# LIVE Streamer

## Windows x64 安装包

需要 Node.js/npm、CMake 3.25+、Visual Studio 2022（v143 C++ 工具及 Windows SDK），
以及 Node-API 头文件（与现有原生构建相同，由 CMake 从 node-gyp 缓存查找）。

在项目根目录双击 `package-win-x64.bat`，或在终端运行：

```powershell
.\package-win-x64.bat --no-pause
```

脚本会在缺少 electron-builder 时执行 `npm ci` 安装依赖；双击运行后会保留窗口以便查看结果。
也可以手动执行：

```powershell
cd electron
npm ci
npm run dist:win
```

命令会编译 Release 原生程序和 Electron 页面，使用 electron-builder 生成 NSIS 安装引擎，
再将它嵌入带有自定义界面的单文件安装包：
`build/installer/live-streamer-1.0.0-windows-x64-setup.exe`。
版本号来自 `electron/package.json`。解包后的程序位于 `build/installer/win-unpacked`。

打包配置：`electron/electron-builder.cjs`；安装界面源码：`installer/`。
安装界面采用 640×483 深色无边框布局，支持路径选择、一键安装、完成后启动和失败重试，
适配 Windows DPI 缩放。使用项目自身图标，不依赖参考安装包的图片或私有 DLL。
安装过程显示活动进度条和真实阶段，等待 NSIS 成功退出后才显示完成，不显示模拟百分比。

安装包包含媒体服务、原生预览模块、图标和 `config/bin` 资源；原生程序使用静态 MSVC 运行库。
默认仅为当前用户安装并创建快捷方式，卸载入口仍由 NSIS 注册到 Windows 设置。
选择的目标应是空文件夹或已有的 LIVE Streamer 安装目录。
`build/installer/*-payload.exe` 是嵌入用中间产物，分发时只需要 `*-setup.exe`。

静默安装示例（返回底层安装引擎的退出码）：

```powershell
Start-Process .\build\installer\live-streamer-1.0.0-windows-x64-setup.exe -ArgumentList '/S', '"/D=D:\Apps\LIVE Streamer"' -Wait
```

界面预览不执行安装：运行安装包并传入 `--preview=welcome`、`--preview=progress`、
`--preview=complete` 或 `--preview=error`。预览模式下主按钮仅关闭窗口。
预览模式可附加 `--capture=完整路径.png` 导出图片并退出。

运行安装器验证（四种界面预览、实际安装、覆盖安装和卸载）：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File installer/smoke.ps1 -Install
```

验证使用 `build/installer-ui-smoke` 中的临时目录；检测到已有安装时会拒绝执行安装测试。
省略 `-Install` 则仅导出四种界面预览。

安装后的日志保存在 Electron 用户数据目录下的 `logs`，可用 `--main-log` 覆盖。
默认不发布产物；当前自定义外层安装包未配置代码签名。

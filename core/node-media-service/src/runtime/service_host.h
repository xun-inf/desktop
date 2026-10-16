#pragma once

#include "runtime/transport.h"
#include "services/service_proxy.h"

#include <node_api.h>
#include <windows.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace node_media_service
{
// media-service 子进程启动参数（addon.cpp 从 JS options 解析而来）。
struct ServiceHostOptions
{
    std::wstring executable;
    std::wstring pipePath;
    std::wstring logPath;
    uint32_t startupTimeoutMs = 10000;
    uint32_t shutdownTimeoutMs = 1500;
    std::vector<std::wstring> arguments;
};

// 服务映射宿主：启动 media-service.exe，连接 Named Pipe，完成 Session 握手。
// 具体服务实例由 services/service_registry 的工厂表统一创建，宿主按 serviceId 持有；
// createServiceRoot 挂出的服务对象同样来自 installAllServices，宿主不感知具体服务。
struct StatusCallback
{
    napi_ref callback = nullptr;
    bool disposed = false;
};

class ServiceHost
{
public:
    explicit ServiceHost(napi_env env);
    ~ServiceHost();

    ServiceHost(const ServiceHost&) = delete;
    ServiceHost& operator=(const ServiceHost&) = delete;

    // 启动子进程、连接管道、Session hello 握手；失败时清理并返回 false。
    bool start(const ServiceHostOptions& options);
    // 终止子进程并断开连接（可重复调用；线程安全）。
    void shutdown();

    bool ready() const
    {
        return m_ready.load();
    }

    Transport* transport() const
    {
        return m_transport.get();
    }
    // 供 createServiceRoot / installAllServices 使用：返回全部服务代理原始指针。
    std::vector<ServiceProxy*> services() const;

    // ---- N-API 门面（createServiceRoot 的 N-API 回调调用） ----
    napi_value initialize(napi_env env);
    napi_value stop(napi_env env);
    napi_value getStatus(napi_env env);
    napi_value subscribeStatus(napi_env env, napi_value callback);

    // 子进程意外退出（监视线程）：断开并通知状态。
    void onDisconnected();
    // 状态通知的 JS 线程入口（TSFN 回调）。
    void notifyStatusOnJs();
    // 状态订阅 dispose 回调需要移除回调（内部实现）。
    void removeStatusCallback(StatusCallback* entry);

private:
    bool startProcess(const ServiceHostOptions& options);
    bool handshake(DWORD timeoutMs);
    void setPhase(const std::string& phase, const std::string& message);
    napi_value buildStatus(napi_env env);
    void notifyStatus();

    napi_env m_env = nullptr;
    std::unique_ptr<Transport> m_transport;
    // 全部服务代理实例（services/service_registry 工厂表创建，按 serviceId 匹配）。
    std::vector<std::unique_ptr<ServiceProxy>> m_services;

    HANDLE m_process = nullptr;
    std::thread m_processMonitor;
    std::atomic<bool> m_stopped {false};
    std::atomic<bool> m_ready {false};
    std::atomic<bool> m_connected {false};
    std::string m_phase = "idle";
    std::string m_message;
    mutable std::mutex m_statusMutex;
    uint32_t m_shutdownTimeoutMs = 1500;
    DWORD m_pid = 0;

    // 状态订阅回调（仅在 JS 线程遍历）。
    std::mutex m_subMutex;
    std::vector<std::unique_ptr<StatusCallback>> m_statusCallbacks;
    napi_threadsafe_function m_statusTsfn = nullptr;
};

// 创建 root 对象并绑定方法；root 被 GC 时调用 host->shutdown() 并删除 host。
napi_value createServiceRoot(napi_env env, ServiceHost* host);
} // namespace node_media_service

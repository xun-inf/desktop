#pragma once

#include "config.h"
#include "event_loop.h"
#include "ipc_server.h"
#include "service_registry.h"

#include <cstdint>
#include <memory>

class MediaServiceApplication
{
    static MediaServiceApplication* s_instance;

public:
    MediaServiceApplication(int argc, char** argv);
    ~MediaServiceApplication();

    MediaServiceApplication(const MediaServiceApplication&) = delete;
    MediaServiceApplication& operator=(const MediaServiceApplication&) = delete;

    static MediaServiceApplication* instance()
    {
        return MediaServiceApplication::s_instance;
    }

    Config* config()
    {
        return &m_config;
    }

    IpcServer* ipcServer()
    {
        return &m_ipcServer;
    }

    EventLoop* eventLoop()
    {
        return &m_eventLoop;
    }

    ServiceRegistry* serviceRegistry()
    {
        return m_serviceRegistry.get();
    }

    // 准备事件循环、Service 注册表与业务处理器，可重复调用。
    bool initialize();

    // 每轮运行前须 initialize；运行结束或启动失败后需重新初始化。
    bool exec();

private:
    void cleanup();

    bool m_initialized = false;

    Config m_config;

    // UI 循环必须比 IPC 回调和业务 Service 存活更久。
    EventLoop m_eventLoop;

    IpcServer m_ipcServer;

    // Service 管理层：services::registerAll 在 initialize() 构造并注册全部业务 Service，
    // application 不直接持有具体 Service（见 services/register.h）。
    std::unique_ptr<ServiceRegistry> m_serviceRegistry;
};

#define msApp MediaServiceApplication::instance()

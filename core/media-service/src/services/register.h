#pragma once

class EventLoop;
class IpcServer;
class ServiceRegistry;

// services 层统一服务注册接口：application 只调用一次 registerAll，
// 不再在 application 里逐服务构造/注册（避免业务叠加）。
// 新增业务 Service：实现静态 createService(IpcServer*, EventLoop*)（签名见 Service::Factory），
// 并在 register.cpp 的 kFactories 表登记一行；application 无需改动。
namespace services
{

bool registerAll(ServiceRegistry& registry, IpcServer* server, EventLoop* eventLoop);

} // namespace services

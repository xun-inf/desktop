#include "services/service_registry.h"

#include "services/native_window_service.h"
#include "services/stream_preview_service.h"

#include "runtime/napi_util.h"

#include <cstring>

namespace node_media_service
{
namespace
{

ServiceProxy* createNativeWindowService(Transport* transport)
{
    return new NativeWindowService(transport);
}

void installNativeWindowServiceProxy(napi_env env, napi_value target, napi_value owner, ServiceProxy* service)
{
    installNativeWindowService(env, target, owner, static_cast<NativeWindowService*>(service));
}

ServiceProxy* createStreamPreviewService(Transport* transport)
{
    return new StreamPreviewService(transport);
}

void installStreamPreviewServiceProxy(napi_env env, napi_value target, napi_value owner, ServiceProxy* service)
{
    installStreamPreviewService(env, target, owner, static_cast<StreamPreviewService*>(service));
}

// 全部内置服务的工厂表：serviceId = 代理 serviceId()（类名），与 media-service 侧一致。
const ServiceFactory kServiceFactories[] = {
    {"NativeWindowService", "nativeWindowService", createNativeWindowService, installNativeWindowServiceProxy},
    {"StreamPreviewService", "streamPreviewService", createStreamPreviewService, installStreamPreviewServiceProxy},
};

} // namespace

const ServiceFactory* serviceFactories()
{
    return kServiceFactories;
}

std::size_t serviceFactoryCount()
{
    return sizeof(kServiceFactories) / sizeof(kServiceFactories[0]);
}

void installAllServices(napi_env env, napi_value root, napi_value owner, const std::vector<ServiceProxy*>& services)
{
    for (std::size_t index = 0; index < serviceFactoryCount(); ++index)
    {
        const ServiceFactory& factory = serviceFactories()[index];
        ServiceProxy* service = nullptr;
        for (ServiceProxy* candidate : services)
        {
            if (std::strcmp(candidate->serviceId(), factory.serviceId) == 0)
            {
                service = candidate;
                break;
            }
        }
        if (service == nullptr)
        {
            continue;
        }
        napi_value serviceObject = object(env);
        factory.install(env, serviceObject, owner, service);
        keepOwner(env, serviceObject, owner);
        property(env, root, factory.propertyName, serviceObject);
    }
}

} // namespace node_media_service


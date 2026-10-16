#include "services/register.h"

#include "base/logger.h"
#include "service_registry.h"
#include "services/native_window/native_window_service.h"
#include "services/session/session_service.h"
#include "services/stream_preview/stream_preview_service.h"

#include <memory>
#include <string>

namespace
{

const char kComponent[] = "services";

} // namespace

namespace services
{
namespace
{

// 全部内置 Service 的工厂表：service_id = 工厂返回实例的 serviceId()（类名）。
// 新增业务 Service 在此登记一行即可，application 不感知具体服务。
const Service::Factory kFactories[] = {
    &NativeWindowService::createService,
    &StreamPreviewService::createService,
    &SessionService::createService,
};

} // namespace

bool registerAll(ServiceRegistry& registry, IpcServer* server, EventLoop* eventLoop)
{
    bool ok = true;
    for (const Service::Factory factory : kFactories)
    {
        std::shared_ptr<Service> service = factory(server, eventLoop);
        if (service == nullptr)
        {
            mediaservice::logger().logError(kComponent, "factory returned null service");
            ok = false;
            continue;
        }
        if (!registry.registerService(service))
        {
            mediaservice::logger().logError(kComponent,
                                            "failed to register service: " + std::string(service->serviceId()));
            ok = false;
        }
    }
    return ok;
}

} // namespace services

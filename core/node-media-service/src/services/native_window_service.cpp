#include "native_window_service.h"

#include "runtime/napi_util.h"
#include "runtime/transport.h"
#include "protos/native_window_service.h"
#include "protos/service.h"

#include <cstring>
#include <stdexcept>

namespace node_media_service
{

namespace
{

struct NativeWindowServiceMethodContext
{
    NativeWindowService* service;
    const char* name;
};

void native_window_serviceMethodFinalizer(napi_env, void* data, void*)
{
    delete static_cast<NativeWindowServiceMethodContext*>(data);
}

napi_value native_window_serviceMethod(napi_env env, napi_callback_info info)
{
    try
    {
        napi_value args[8] = {};
        size_t count = 8;
        void* data = nullptr;
        NMS_NAPI(napi_get_cb_info)(env, info, &count, args, nullptr, &data);
        NativeWindowServiceMethodContext* context = static_cast<NativeWindowServiceMethodContext*>(data);
        const char* method = context->name;
        if (std::strcmp(method, "create") == 0)
        {
            if (count < 3) throw std::runtime_error("NativeWindowService.create requires 3 arguments");
            return context->service->create(env, uint32Value(env, args[0]), ulongValue(env, args[1]), boolValue(env, args[2]));
        }
        if (std::strcmp(method, "refresh") == 0)
        {
            if (count < 3) throw std::runtime_error("NativeWindowService.refresh requires 3 arguments");
            return context->service->refresh(env, uint32Value(env, args[0]), ulongValue(env, args[1]), boolValue(env, args[2]));
        }
        if (std::strcmp(method, "releaseAll") == 0)
        {
            return context->service->releaseAll(env);
        }
        if (std::strcmp(method, "release") == 0)
        {
            if (count < 1) throw std::runtime_error("NativeWindowService.release requires 1 arguments");
            return context->service->release(env, uint32Value(env, args[0]));
        }
        if (std::strcmp(method, "setRect") == 0)
        {
            if (count < 5) throw std::runtime_error("NativeWindowService.setRect requires 5 arguments");
            return context->service->setRect(env, uint32Value(env, args[0]), int32Value(env, args[1]), int32Value(env, args[2]), int32Value(env, args[3]), int32Value(env, args[4]));
        }
        if (std::strcmp(method, "setVisible") == 0)
        {
            if (count < 2) throw std::runtime_error("NativeWindowService.setVisible requires 2 arguments");
            return context->service->setVisible(env, uint32Value(env, args[0]), boolValue(env, args[1]));
        }
        throw std::runtime_error(std::string("Unknown native_window_service method: ") + method);
    }
    catch (const std::exception& error)
    {
        NMS_NAPI(napi_throw_error)(env, nullptr, error.what());
        return nullptr;
    }
}

void addNativeWindowServiceMethod(napi_env env, napi_value target, napi_value owner, NativeWindowService* service, const char* name)
{
    NativeWindowServiceMethodContext* context = new NativeWindowServiceMethodContext();
    context->service = service;
    context->name = name;
    napi_value function = nullptr;
    NMS_NAPI(napi_create_function)(env, name, NAPI_AUTO_LENGTH, native_window_serviceMethod, context, &function);
    NMS_NAPI(napi_add_finalizer)(env, function, context, native_window_serviceMethodFinalizer, nullptr, nullptr);
    keepOwner(env, function, owner);
    property(env, target, name, function);
}

} // namespace

NativeWindowService::NativeWindowService(Transport* transport) : ServiceProxy(transport)
{
}

napi_value NativeWindowService::create(napi_env env, uint32_t id, uint64_t host, bool host_visible)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateCreateArgs(builder, id, host, host_visible);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("NativeWindowService", "create",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "NativeWindowService.create failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

napi_value NativeWindowService::refresh(napi_env env, uint32_t id, uint64_t host, bool host_visible)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateRefreshArgs(builder, id, host, host_visible);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("NativeWindowService", "refresh",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "NativeWindowService.refresh failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

napi_value NativeWindowService::releaseAll(napi_env env)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateReleaseAllArgs(builder);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("NativeWindowService", "releaseAll",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "NativeWindowService.releaseAll failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

napi_value NativeWindowService::release(napi_env env, uint32_t id)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateReleaseArgs(builder, id);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("NativeWindowService", "release",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "NativeWindowService.release failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

napi_value NativeWindowService::setRect(napi_env env, uint32_t id, int32_t x, int32_t y, int32_t width, int32_t height)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateSetRectArgs(builder, id, x, y, width, height);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("NativeWindowService", "setRect",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "NativeWindowService.setRect failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

napi_value NativeWindowService::setVisible(napi_env env, uint32_t id, bool visible)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateSetVisibleArgs(builder, id, visible);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("NativeWindowService", "setVisible",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "NativeWindowService.setVisible failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

void installNativeWindowService(napi_env env, napi_value target, napi_value owner, NativeWindowService* service)
{
    addNativeWindowServiceMethod(env, target, owner, service, "create");
    addNativeWindowServiceMethod(env, target, owner, service, "refresh");
    addNativeWindowServiceMethod(env, target, owner, service, "releaseAll");
    addNativeWindowServiceMethod(env, target, owner, service, "release");
    addNativeWindowServiceMethod(env, target, owner, service, "setRect");
    addNativeWindowServiceMethod(env, target, owner, service, "setVisible");
}

} // namespace node_media_service

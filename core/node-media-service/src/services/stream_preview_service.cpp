#include "stream_preview_service.h"

#include "runtime/napi_util.h"
#include "runtime/transport.h"
#include "protos/stream_preview_service.h"
#include "protos/service.h"

#include <cstring>
#include <stdexcept>

namespace node_media_service
{

namespace
{

napi_value decodeFrameEvent(napi_env env, Transport* transport, const std::vector<uint8_t>& payload)
{
    if (payload.empty()) return undefined(env);
    flatbuffers::Verifier verifier(payload.data(), payload.size());
    if (!services::VerifyFrameEventBuffer(verifier)) return undefined(env);
    const services::FrameEvent* value = services::GetFrameEvent(payload.data());
    napi_value decoded = object(env);
    property(env, decoded, "id", number(env, static_cast<double>(value->id())));
    property(env, decoded, "token", bigint(env, value->token()));
    property(env, decoded, "handle", handleBuffer(env, transport ? transport->childPid() : 0, value->handle()));
    property(env, decoded, "width", number(env, static_cast<double>(value->width())));
    property(env, decoded, "height", number(env, static_cast<double>(value->height())));
    property(env, decoded, "timestampUs", number(env, static_cast<double>(value->timestamp_us())));
    return decoded;
}

struct StreamPreviewServiceFrameDisposeContext
{
    StreamPreviewService* service;
    bool disposed;
};

napi_value stream_preview_serviceFrameDispose(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NMS_NAPI(napi_get_cb_info)(env, info, nullptr, nullptr, nullptr, &data);
    StreamPreviewServiceFrameDisposeContext* context = static_cast<StreamPreviewServiceFrameDisposeContext*>(data);
    if (context != nullptr && !context->disposed)
    {
        context->disposed = true;
        context->service->unsubscribeFrame(env);
    }
    napi_value result = nullptr;
    NMS_NAPI(napi_get_undefined)(env, &result);
    return result;
}

void stream_preview_serviceFrameDisposeFinalizer(napi_env, void* data, void*)
{
    delete static_cast<StreamPreviewServiceFrameDisposeContext*>(data);
}

struct StreamPreviewServiceMethodContext
{
    StreamPreviewService* service;
    const char* name;
};

void stream_preview_serviceMethodFinalizer(napi_env, void* data, void*)
{
    delete static_cast<StreamPreviewServiceMethodContext*>(data);
}

napi_value stream_preview_serviceMethod(napi_env env, napi_callback_info info)
{
    try
    {
        napi_value args[8] = {};
        size_t count = 8;
        void* data = nullptr;
        NMS_NAPI(napi_get_cb_info)(env, info, &count, args, nullptr, &data);
        StreamPreviewServiceMethodContext* context = static_cast<StreamPreviewServiceMethodContext*>(data);
        const char* method = context->name;
        if (std::strcmp(method, "createStreamPreview") == 0)
        {
            if (count < 1) throw std::runtime_error("StreamPreviewService.createStreamPreview requires 1 arguments");
            return context->service->createStreamPreview(env, uint32Value(env, args[0]));
        }
        if (std::strcmp(method, "releaseStreamPreview") == 0)
        {
            if (count < 1) throw std::runtime_error("StreamPreviewService.releaseStreamPreview requires 1 arguments");
            return context->service->releaseStreamPreview(env, uint32Value(env, args[0]));
        }
        if (std::strcmp(method, "releaseVideoFrame") == 0)
        {
            if (count < 2) throw std::runtime_error("StreamPreviewService.releaseVideoFrame requires 2 arguments");
            return context->service->releaseVideoFrame(env, uint32Value(env, args[0]), ulongValue(env, args[1]));
        }
        if (std::strcmp(method, "subscribeFrame") == 0)
        {
            if (count < 1) throw std::runtime_error("StreamPreviewService.subscribeFrame requires 1 arguments");
            return context->service->subscribeFrame(env, args[0]);
        }
        throw std::runtime_error(std::string("Unknown stream_preview_service method: ") + method);
    }
    catch (const std::exception& error)
    {
        NMS_NAPI(napi_throw_error)(env, nullptr, error.what());
        return nullptr;
    }
}

void addStreamPreviewServiceMethod(napi_env env, napi_value target, napi_value owner, StreamPreviewService* service, const char* name)
{
    StreamPreviewServiceMethodContext* context = new StreamPreviewServiceMethodContext();
    context->service = service;
    context->name = name;
    napi_value function = nullptr;
    NMS_NAPI(napi_create_function)(env, name, NAPI_AUTO_LENGTH, stream_preview_serviceMethod, context, &function);
    NMS_NAPI(napi_add_finalizer)(env, function, context, stream_preview_serviceMethodFinalizer, nullptr, nullptr);
    keepOwner(env, function, owner);
    property(env, target, name, function);
}

} // namespace

StreamPreviewService::StreamPreviewService(Transport* transport) : ServiceProxy(transport)
{
}

napi_value StreamPreviewService::subscribeFrame(napi_env env, napi_value callback)
{
    napi_valuetype type = napi_undefined;
    NMS_NAPI(napi_typeof)(env, callback, &type);
    if (type != napi_function)
    {
        throw std::runtime_error("Expected a frame event callback");
    }
    if (m_frameCallback != nullptr)
    {
        NMS_NAPI(napi_delete_reference)(env, m_frameCallback);
        m_frameCallback = nullptr;
    }
    NMS_NAPI(napi_create_reference)(env, callback, 1, &m_frameCallback);
    m_transport->setEventCallback("StreamPreviewService", "frame",
        [this](napi_env env, const std::vector<uint8_t>& payload)
        {
            dispatchFrame(env, payload);
        });
    StreamPreviewServiceFrameDisposeContext* context = new StreamPreviewServiceFrameDisposeContext();
    context->service = this;
    context->disposed = false;
    napi_value dispose = nullptr;
    NMS_NAPI(napi_create_function)(env, "dispose", NAPI_AUTO_LENGTH, stream_preview_serviceFrameDispose, context, &dispose);
    NMS_NAPI(napi_add_finalizer)(env, dispose, context, stream_preview_serviceFrameDisposeFinalizer, nullptr, nullptr);
    keepOwner(env, dispose, callback);
    napi_value result = object(env);
    property(env, result, "dispose", dispose);
    return result;
}

void StreamPreviewService::unsubscribeFrame(napi_env env)
{
    if (m_frameCallback != nullptr)
    {
        NMS_NAPI(napi_delete_reference)(env, m_frameCallback);
        m_frameCallback = nullptr;
    }
    m_transport->setEventCallback("StreamPreviewService", "frame", nullptr);
}

void StreamPreviewService::dispatchFrame(napi_env env, const std::vector<uint8_t>& payload)
{
    if (m_frameCallback == nullptr) return;
    napi_value decoded = decodeFrameEvent(env, m_transport, payload);
    napi_value callback = nullptr;
    NMS_NAPI(napi_get_reference_value)(env, m_frameCallback, &callback);
    napi_value global = nullptr;
    NMS_NAPI(napi_get_global)(env, &global);
    napi_value argv[] = {decoded};
    napi_value ignored = nullptr;
    NMS_NAPI(napi_call_function)(env, global, callback, 1, argv, &ignored);
}

napi_value StreamPreviewService::createStreamPreview(napi_env env, uint32_t id)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateCreateStreamPreviewArgs(builder, id);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("StreamPreviewService", "createStreamPreview",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "StreamPreviewService.createStreamPreview failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

napi_value StreamPreviewService::releaseStreamPreview(napi_env env, uint32_t id)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateReleaseStreamPreviewArgs(builder, id);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("StreamPreviewService", "releaseStreamPreview",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "StreamPreviewService.releaseStreamPreview failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

napi_value StreamPreviewService::releaseVideoFrame(napi_env env, uint32_t id, uint64_t token)
{
    flatbuffers::FlatBufferBuilder builder;
    const auto args = services::CreateReleaseVideoFrameArgs(builder, id, token);
    builder.Finish(args);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NMS_NAPI(napi_create_promise)(env, &deferred, &promise);
    m_transport->invoke("StreamPreviewService", "releaseVideoFrame",
        builder.GetBufferPointer(), builder.GetSize(),
        env, deferred,
        [](napi_env env, napi_deferred deferred, const services::InvokeRsp* reply)
        {
            if (!reply->success())
            {
                const std::string message = reply->message() ? reply->message()->str() : "StreamPreviewService.releaseVideoFrame failed";
                NMS_NAPI(napi_reject_deferred)(env, deferred, exception(env, message));
                return;
            }
            NMS_NAPI(napi_resolve_deferred)(env, deferred, undefined(env));
        });
    return promise;
}

void installStreamPreviewService(napi_env env, napi_value target, napi_value owner, StreamPreviewService* service)
{
    addStreamPreviewServiceMethod(env, target, owner, service, "createStreamPreview");
    addStreamPreviewServiceMethod(env, target, owner, service, "releaseStreamPreview");
    addStreamPreviewServiceMethod(env, target, owner, service, "releaseVideoFrame");
    addStreamPreviewServiceMethod(env, target, owner, service, "subscribeFrame");
}

} // namespace node_media_service

#include "appletController.h"

namespace skyline::service::am {
    void ICommonStateGetter::QueueMessage(ICommonStateGetter::Message message) {
        messageQueue.emplace(message);
        messageEvent->Signal();
    }

    ICommonStateGetter::ICommonStateGetter(const DeviceState &state, ServiceManager &manager) : messageEvent(std::make_shared<type::KEvent>(state)), BaseService(state, manager, false, Service::am_ICommonStateGetter, {
        {0x0, SFUNC(ICommonStateGetter::GetEventHandle)},
        {0x1, SFUNC(ICommonStateGetter::ReceiveMessage)},
        {0x9, SFUNC(ICommonStateGetter::GetCurrentFocusState)},
        {0x5, SFUNC(ICommonStateGetter::GetOperationMode)},
        {0x6, SFUNC(ICommonStateGetter::GetPerformanceMode)},
        {0x3C, SFUNC(ICommonStateGetter::GetDefaultDisplayResolution)}
    }) {
        operationMode = static_cast<OperationMode>(state.settings->GetBool("operation_mode"));
        state.logger->Info("Switch on mode: {}", static_cast<bool>(operationMode) ? "Docked" : "Handheld");
        QueueMessage(Message::FocusStateChange);
    }

    void ICommonStateGetter::GetEventHandle(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto handle = state.thisProcess->InsertItem(messageEvent);
        state.logger->Debug("Event Handle: 0x{:X}", handle);
        response.copyHandles.push_back(handle);
    }

    void ICommonStateGetter::ReceiveMessage(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        if (messageQueue.empty()) {
            response.errorCode = constant::status::NoMessages;
            return;
        }
        response.Push<u32>(static_cast<u32>(messageQueue.front()));
        messageQueue.pop();
    }

    void ICommonStateGetter::GetCurrentFocusState(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u8>(static_cast<u8>(focusState));
    }

    void ICommonStateGetter::GetOperationMode(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u8>(static_cast<u8>(operationMode));
    }

    void ICommonStateGetter::GetPerformanceMode(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u32>(static_cast<u32>(operationMode));
    }

    void ICommonStateGetter::GetDefaultDisplayResolution(type::KSession& session, ipc::IpcRequest& request, ipc::IpcResponse& response) {
        if (operationMode == OperationMode::Handheld) {
            response.Push<u32>(constant::HandheldResolutionW);
            response.Push<u32>(constant::HandheldResolutionH);
        }  else if (operationMode == OperationMode::Docked) {
            response.Push<u32>(constant::DockedResolutionW);
            response.Push<u32>(constant::DockedResolutionH);
        }
    }

    ISelfController::ISelfController(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::am_ISelfController, {
        {0xB, SFUNC(ISelfController::SetOperationModeChangedNotification)},
        {0xC, SFUNC(ISelfController::SetPerformanceModeChangedNotification)},
        {0xD, SFUNC(ISelfController::SetFocusHandlingMode)},
        {0x10, SFUNC(ISelfController::SetOutOfFocusSuspendingEnabled)},
        {0x28, SFUNC(ISelfController::CreateManagedDisplayLayer)}
    }) {}

    void ISelfController::SetOperationModeChangedNotification(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {}

    void ISelfController::SetPerformanceModeChangedNotification(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {}

    void ISelfController::SetFocusHandlingMode(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {}

    void ISelfController::SetOutOfFocusSuspendingEnabled(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {}

    void ISelfController::CreateManagedDisplayLayer(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.logger->Debug("Creating Managed Layer");
        if (state.gpu->layerStatus == gpu::LayerStatus::Initialized)
            throw exception("The application is creating more than one layer");
        state.gpu->layerStatus = gpu::LayerStatus::Initialized;
        response.Push<u64>(0);
    }

    IWindowController::IWindowController(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::am_IWindowController, {
        {0x1, SFUNC(IWindowController::GetAppletResourceUserId)},
        {0xA, SFUNC(IWindowController::AcquireForegroundRights)}
    }) {}

    void IWindowController::GetAppletResourceUserId(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push(static_cast<u64>(state.thisProcess->mainThread));
    }

    void IWindowController::AcquireForegroundRights(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {}

    IAudioController::IAudioController(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::am_IAudioController, {
    }) {}

    IDisplayController::IDisplayController(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::am_IDisplayController, {
    }) {}

    ILibraryAppletCreator::ILibraryAppletCreator(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::am_ILibraryAppletCreator, {
    }) {}

    IApplicationFunctions::IApplicationFunctions(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::am_IApplicationFunctions, {
        {0x28, SFUNC(IApplicationFunctions::NotifyRunning)}
    }) {}

    void IApplicationFunctions::NotifyRunning(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        response.Push<u8>(1);
    }

    IDebugFunctions::IDebugFunctions(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::am_IDebugFunctions, {
    }) {}

    IAppletCommonFunctions::IAppletCommonFunctions(const DeviceState& state, ServiceManager& manager) : BaseService(state, manager, false, Service::am_IAppletCommonFunctions, {
    }) {}
}

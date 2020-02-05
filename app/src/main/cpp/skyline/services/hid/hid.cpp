#include "hid.h"
#include <os.h>

using namespace skyline::input::npad;

namespace skyline::service::hid {
    IAppletResource::IAppletResource(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::hid_IAppletResource, {
        {0x0, SFUNC(IAppletResource::GetSharedMemoryHandle)}
    }) {}

    void IAppletResource::GetSharedMemoryHandle(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto handle = state.process->InsertItem<type::KSharedMemory>(state.input->hidKMem);
        state.logger->Debug("HID Shared Memory Handle: 0x{:X}", handle);
        response.copyHandles.push_back(handle);
    }

    hid::hid(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, false, Service::hid, {
        {0x0, SFUNC(hid::CreateAppletResource)},
        {0x64, SFUNC(hid::SetSupportedNpadStyleSet)},
        {0x66, SFUNC(hid::SetSupportedNpadIdType)},
        {0x67, SFUNC(hid::ActivateNpad)},
        {0x78, SFUNC(hid::SetNpadJoyHoldType)},
        {0x7A, SFUNC(hid::SetNpadJoyAssignmentModeSingleByDefault)},
        {0x7B, SFUNC(hid::SetNpadJoyAssignmentModeSingle)},
        {0x7C, SFUNC(hid::SetNpadJoyAssignmentModeDual)}
    }) {
        state.input->commonNpad->Activate(); // Not all homebrew activates npad this manually
    }

    void hid::CreateAppletResource(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        resource = std::make_shared<IAppletResource>(state, manager);
        manager.RegisterService(resource, session, response);
    }

    void hid::SetSupportedNpadStyleSet(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        auto styleSet = request.Pop<NpadStyleSet>();
        state.logger->Debug("Controller Support:\nPro-Controller: {}\nJoy-Con: Handheld: {}, Dual: {}, L: {}, R: {}\nGameCube: {}\nPokeBall: {}\nNES: {}, NES Handheld: {}, SNES: {}", static_cast<bool>(styleSet.proController), static_cast<bool>(styleSet.joyconHandheld), static_cast<bool>(styleSet.joyconDual), static_cast<bool>(styleSet.joyconLeft), static_cast<bool>
        (styleSet.joyconRight), static_cast<bool>(styleSet.gamecube), static_cast<bool>(styleSet.palma), static_cast<bool>(styleSet.nes), static_cast<bool>(styleSet.nesHandheld), static_cast<bool>(styleSet.snes));
        state.input->commonNpad->supportedStyles = styleSet;
    }

    void hid::SetSupportedNpadIdType(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        const auto &buffer = request.inputBuf.at(0);
        state.logger->Error("{}", __func__);
        size_t numId = buffer.size / sizeof(NpadId);
        u64 address = buffer.address;
        for (size_t i = 0; i < numId; i++) {
            auto id = state.process->ReadMemory<NpadId>(address);
            state.input->npad.at(NpadIdToIndex(id))->supported = true;
            address += sizeof(NpadId);
        }
    }

    void hid::ActivateNpad(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.input->commonNpad->Activate();
    }

    void hid::SetNpadJoyHoldType(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        u64 appletResourceUID = request.Pop<u64>();
        state.input->commonNpad->orientation = request.Pop<NpadJoyOrientation>();
    }

    void hid::SetNpadJoyAssignmentModeSingleByDefault(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.input->npad.at(NpadIdToIndex(request.Pop<NpadId>()))->SetAssignment(NpadJoyAssignment::Single);
    }

    void hid::SetNpadJoyAssignmentModeSingle(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.input->npad.at(NpadIdToIndex(request.Pop<NpadId>()))->SetAssignment(NpadJoyAssignment::Single);
    }

    void hid::SetNpadJoyAssignmentModeDual(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.input->npad.at(NpadIdToIndex(request.Pop<NpadId>()))->SetAssignment(NpadJoyAssignment::Dual);
    }
}

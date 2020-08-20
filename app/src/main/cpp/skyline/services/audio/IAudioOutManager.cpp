// SPDX-License-Identifier: MPL-2.0
// Copyright © 2020 Skyline Team and Contributors (https://github.com/skyline-emu/)

#include <kernel/types/KProcess.h>
#include "IAudioOutManager.h"
#include "IAudioOut.h"

namespace skyline::service::audio {
    IAudioOutManager::IAudioOutManager(const DeviceState &state, ServiceManager &manager) : BaseService(state, manager, Service::audio_IAudioOutManager, "audio:IAudioOutManager", {
        {0x0, SFUNC(IAudioOutManager::ListAudioOuts)},
        {0x1, SFUNC(IAudioOutManager::OpenAudioOut)},
        {0x2, SFUNC(IAudioOutManager::ListAudioOuts)},
        {0x3, SFUNC(IAudioOutManager::OpenAudioOut)}
    }) {}

    void IAudioOutManager::ListAudioOuts(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        state.process->WriteMemory(reinterpret_cast<void *>(const_cast<char *>(constant::DefaultAudioOutName.data())), request.outputBuf.at(0).address, constant::DefaultAudioOutName.size());
    }

    void IAudioOutManager::OpenAudioOut(type::KSession &session, ipc::IpcRequest &request, ipc::IpcResponse &response) {
        u32 sampleRate = request.Pop<u32>();
        u16 channelCount = static_cast<u16>(request.Pop<u32>());

        state.logger->Debug("Opening an IAudioOut with sample rate: {}, channel count: {}", sampleRate, channelCount);

        sampleRate = sampleRate ? sampleRate : constant::SampleRate;
        channelCount = channelCount ? channelCount : constant::ChannelCount;
        manager.RegisterService(std::make_shared<IAudioOut>(state, manager, channelCount, sampleRate), session, response);

        response.Push<u32>(sampleRate);
        response.Push<u16>(channelCount);
        response.Push<u16>(0);
        response.Push(static_cast<u32>(skyline::audio::AudioFormat::Int16));
        response.Push(static_cast<u32>(skyline::audio::AudioOutState::Stopped));
    }
}

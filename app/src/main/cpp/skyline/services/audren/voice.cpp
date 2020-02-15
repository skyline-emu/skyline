#include <common.h>
#include "voice.h"
#include <kernel/types/KProcess.h>

namespace skyline::service::audren {
    Voice::Voice(const DeviceState &state) : state(state) {}

    void Voice::ProcessInput(const VoiceIn &input) {
        // Voice no longer in use, reset it
        if (acquired && !input.acquired) {
            bufferReload = true;
            bufferIndex = 0;
            sampleOffset = 0;

            output.playedSamplesCount = 0;
            output.playedWaveBuffersCount = 0;
            output.voiceDropsCount = 0;
        }

        acquired = input.acquired;

        if (!acquired)
            return;

        if (input.firstUpdate) {
            if (input.pcmFormat != audio::PcmFormat::Int16)
                throw exception("Unsupported voice PCM format: {}", input.pcmFormat);

            pcmFormat = input.pcmFormat;
            sampleRate = input.sampleRate;

            if (input.channelCount > 2)
                throw exception("Unsupported voice channel count: {}", input.channelCount);

            channelCount = input.channelCount;
            SetWaveBufferIndex(input.baseWaveBufferIndex);
        }

        waveBuffers = input.waveBuffers;
        volume = input.volume;
        playbackState = input.playbackState;
    }

    void Voice::UpdateBuffers() {
        WaveBuffer &currentBuffer = waveBuffers.at(bufferIndex);

        if (currentBuffer.size == 0)
            return;

        switch (pcmFormat) {
            case audio::PcmFormat::Int16:
                sampleBuffer.resize(currentBuffer.size / sizeof(i16));
                state.process->ReadMemory(sampleBuffer.data(), currentBuffer.position, currentBuffer.size);
                break;
            default:
                throw exception("Unsupported voice PCM format: {}", pcmFormat);
        }

        if (sampleRate != audio::constant::SampleRate)
            sampleBuffer = resampler.ResampleBuffer(sampleBuffer, static_cast<double>(sampleRate) / audio::constant::SampleRate, channelCount);

        if (channelCount == 1 && audio::constant::ChannelCount != channelCount) {
            size_t originalSize = sampleBuffer.size();
            sampleBuffer.resize((originalSize / channelCount) * audio::constant::ChannelCount);

            for (size_t monoIndex = originalSize - 1, targetIndex = sampleBuffer.size(); monoIndex > 0; monoIndex--)
                for (uint i = 0; i < audio::constant::ChannelCount; i++)
                    sampleBuffer[--targetIndex] = sampleBuffer[monoIndex];
        }
    }

    std::vector<i16> &Voice::GetBufferData(int maxSamples, int &outOffset, int &outSize) {
        WaveBuffer &currentBuffer = waveBuffers.at(bufferIndex);

        if (!acquired || playbackState != audio::AudioOutState::Started) {
            outSize = 0;
            return sampleBuffer;
        }

        if (bufferReload) {
            bufferReload = false;
            UpdateBuffers();
        }

        outOffset = sampleOffset;
        outSize = std::min(maxSamples * audio::constant::ChannelCount, static_cast<int>(sampleBuffer.size() - sampleOffset));

        output.playedSamplesCount += outSize / audio::constant::ChannelCount;
        sampleOffset += outSize;

        if (sampleOffset == sampleBuffer.size()) {
            sampleOffset = 0;

            if (currentBuffer.lastBuffer)
                playbackState = audio::AudioOutState::Paused;

            if (!currentBuffer.looping)
                SetWaveBufferIndex(bufferIndex + 1);

            output.playedWaveBuffersCount++;
        }

        return sampleBuffer;
    }

    void Voice::SetWaveBufferIndex(uint index) {
        bufferIndex = index & 3;
        bufferReload = true;
    }
}

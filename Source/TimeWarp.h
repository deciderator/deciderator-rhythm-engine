// SPDX-License-Identifier: AGPL-3.0-only
// Original DECIDERATOR causal time-remapping engine. No Gross Beat code/assets.
#pragma once
#include <array>
#include <atomic>
#include <vector>
#include <cmath>
#include <algorithm>

class TimeWarpEngine
{
    std::array<std::vector<float>, 2> history;
    std::array<float, 2> dcInput {}, dcOutput {};
    int write = 0, recorded = 0, fadeRemaining = 0, fadeLength = 96, waveCount = 0;
    double previousDelay = 0, oldDelay = 0;
    float peak = 0, dcCoefficient = 0.999f;
    float read(size_t ch, double delay) const noexcept
    {
        const auto length = static_cast<int>(history[ch].size());
        auto position = static_cast<double>(write) - delay;
        if (position < 0) position += length;
        const auto index = static_cast<int>(position);
        const auto fraction = static_cast<float>(position - index);
        return history[ch][static_cast<size_t>(index)] * (1 - fraction)
            + history[ch][static_cast<size_t>((index + 1) % length)] * fraction;
    }
public:
    std::array<std::atomic<float>, 256> waveform {};
    std::atomic<int> waveHead { 0 };
    std::atomic<float> phaseDisplay { 0 }, delayDisplay { 0 }, readiness { 0 };
    void prepare(double sr)
    {
        // Two 4-beat cycles at 20 BPM fit within 26 seconds.
        for (auto& channel : history) channel.assign(static_cast<size_t>(sr * 26.0) + 8, 0);
        fadeLength = std::max(8, static_cast<int>(sr * 0.002));
        dcCoefficient = static_cast<float>(std::exp(-6.283185307179586 * 5.0 / sr));
        reset();
    }
    void reset()
    {
        for (auto& channel : history) std::fill(channel.begin(), channel.end(), 0.0f);
        for (auto& value : waveform) value.store(0);
        write = recorded = fadeRemaining = waveCount = 0;
        previousDelay = oldDelay = 0; dcInput = {}; dcOutput = {}; peak = 0;
        waveHead.store(0); phaseDisplay.store(0); delayDisplay.store(0); readiness.store(0);
    }
    static float curveValue(const std::array<float, 33>& curve, double phase, bool stepped) noexcept
    {
        const auto position = std::clamp(phase, 0.0, 1.0) * 32.0;
        const auto index = std::min(31, static_cast<int>(position));
        const auto fraction = stepped ? 0.0f : static_cast<float>(position - index);
        return std::clamp(curve[static_cast<size_t>(index)] * (1 - fraction)
            + curve[static_cast<size_t>(index + 1)] * fraction, 0.0f, 1.0f);
    }
    std::array<float, 2> process(std::array<float, 2> input, double phase, double cycleSamples,
                                const std::array<float, 33>& curve, bool stepped, float mix) noexcept
    {
        const auto length = static_cast<int>(history[0].size());
        if (length == 0) return input;
        cycleSamples = std::max(16.0, cycleSamples);
        for (size_t ch = 0; ch < 2; ++ch) history[ch][static_cast<size_t>(write)] = input[ch];
        const auto value = curveValue(curve, phase, stepped);
        const auto delay = std::clamp(value * cycleSamples * 2.0, 0.0, static_cast<double>(length - 3));
        if (std::abs(delay - previousDelay) > 128.0 && fadeRemaining == 0)
        {
            oldDelay = previousDelay;
            fadeRemaining = fadeLength;
        }
        const auto available = static_cast<double>(recorded) >= delay;
        const auto blend = 1.0f - static_cast<float>(fadeRemaining) / static_cast<float>(fadeLength);
        auto output = input;
        for (size_t ch = 0; ch < 2; ++ch)
        {
            auto warped = available ? read(ch, delay) : input[ch];
            if (fadeRemaining > 0 && static_cast<double>(recorded) >= oldDelay)
                warped = read(ch, oldDelay) + blend * (warped - read(ch, oldDelay));
            if (delay > 1.0 && available)
            {
                const auto highpassed = warped - dcInput[ch] + dcCoefficient * dcOutput[ch];
                dcInput[ch] = warped; dcOutput[ch] = highpassed; warped = highpassed;
            }
            else { dcInput[ch] = warped; dcOutput[ch] = 0; }
            output[ch] = mix >= 1.0f ? warped : input[ch] + mix * (warped - input[ch]);
        }
        previousDelay = delay;
        if (fadeRemaining > 0) --fadeRemaining;
        peak = std::max(peak, std::max(std::abs(input[0]), std::abs(input[1])));
        if (++waveCount >= std::max(16, static_cast<int>(cycleSamples / 256.0)))
        {
            const auto head = waveHead.load(std::memory_order_relaxed);
            waveform[static_cast<size_t>(head)].store(std::min(1.0f, peak), std::memory_order_relaxed);
            waveHead.store((head + 1) % 256, std::memory_order_relaxed);
            phaseDisplay.store(static_cast<float>(phase), std::memory_order_relaxed);
            delayDisplay.store(value, std::memory_order_relaxed);
            readiness.store(static_cast<float>(std::min(1.0, recorded / (cycleSamples * 2.0))), std::memory_order_relaxed);
            waveCount = 0; peak = 0;
        }
        write = (write + 1) % length;
        recorded = std::min(recorded + 1, length - 2);
        return output;
    }
};

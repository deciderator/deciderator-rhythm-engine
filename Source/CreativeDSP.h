// SPDX-License-Identifier: AGPL-3.0-only
// SpaceFDN adapts Aquaton Reverb by aquanode (AGPLv3).
// Upstream: https://github.com/aquanodemusic/aquanode-plugins/tree/main/effects/aquaton-reverb
// Modified for DECIDERATOR, 2026-11-01: fixed 8-line network, bounded decay,
// preallocated buffers, no matrix UI/locks, no negative predelay or latency changes.
// Original reference files and full licence: third_party/aquanode/.
#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>

class SpaceFDN
{
    struct Allpass
    {
        float mem = 0;
        float process(float in, float coeff) noexcept
        {
            const float out = -coeff * in + mem;
            mem = in + coeff * out;
            return out;
        }
    };
    struct Line
    {
        std::vector<float> buffer;
        std::array<Allpass, 12> diffusion;
        float lp = 0, hp = 0, last = 0;
        double phase = 0;
        int write = 0;
    };
    std::array<Line, 8> lines;
    std::array<std::array<Allpass, 4>, 2> inputDiffusion;
    double sr = 48000;
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        for (auto& line : lines) line.buffer.assign(static_cast<size_t>(sr * 0.3) + 8, 0);
        reset();
    }
    void reset()
    {
        inputDiffusion = {};
        for (auto& line : lines)
        {
            std::fill(line.buffer.begin(), line.buffer.end(), 0.0f);
            line.diffusion = {}; line.lp = line.hp = line.last = 0; line.phase = 0; line.write = 0;
        }
    }
    std::array<float, 2> process(float left, float right, float size, float decay, float tone, float motion) noexcept
    {
        constexpr std::array<float, 8> base { 1013, 1103, 1201, 1303, 1409, 1511, 1619, 1733 };
        std::array<float, 2> input { left, right };
        for (size_t ch = 0; ch < 2; ++ch)
            for (auto& ap : inputDiffusion[ch]) input[ch] = ap.process(input[ch], 0.65f);
        std::array<float, 8> mixed;
        for (size_t i = 0; i < 8; ++i) mixed[i] = lines[i].last;
        // Normalised Hadamard feedback, same mixing family as Aquaton.
        for (size_t stride = 1; stride < 8; stride *= 2)
            for (size_t group = 0; group < 8; group += stride * 2)
                for (size_t i = 0; i < stride; ++i)
                {
                    const auto a = mixed[group + i], b = mixed[group + i + stride];
                    mixed[group + i] = a + b; mixed[group + i + stride] = a - b;
                }
        const auto feedback = 0.45f + decay * 0.49f;
        const auto cutoff = std::min(static_cast<float>(sr * 0.43), 500.0f * std::pow(32.0f, tone));
        const auto lpCoefficient = static_cast<float>(1.0 - std::exp(-6.283185307179586 * cutoff / sr));
        const auto hpCoefficient = static_cast<float>(1.0 - std::exp(-6.283185307179586 * 30.0 / sr));
        std::array<float, 2> output {};
        for (size_t i = 0; i < 8; ++i)
        {
            auto& line = lines[i];
            const auto length = static_cast<int>(line.buffer.size());
            if (length == 0) continue;
            line.phase += (0.13 + static_cast<double>(i) * 0.019) / sr;
            line.phase -= std::floor(line.phase);
            const auto delay = std::clamp(base[i] * (1.0 + size * 5.0) * sr / 44100.0
                + std::sin(line.phase * 6.283185307179586) * motion * sr * 0.00035, 1.0, static_cast<double>(length - 2));
            double read = line.write - delay;
            if (read < 0) read += length;
            const auto index = static_cast<int>(read);
            const auto fraction = static_cast<float>(read - index);
            auto out = line.buffer[static_cast<size_t>(index)] * (1 - fraction)
                + line.buffer[static_cast<size_t>((index + 1) % length)] * fraction;
            line.buffer[static_cast<size_t>(line.write)] = std::tanh((input[0] + input[1]) * 0.15f + mixed[i] * 0.35355339f * feedback);
            line.write = (line.write + 1) % length;
            for (auto& ap : line.diffusion) out = ap.process(out, 0.5f);
            line.lp += lpCoefficient * (out - line.lp);
            line.hp += hpCoefficient * (line.lp - line.hp);
            line.last = std::tanh(line.lp - line.hp);
            output[i % 2] += line.last * 0.5f;
        }
        return output;
    }
};

// Original DECIDERATOR stutter/lo-fi implementation. Not Granulate FX code.
class TextureEngine
{
    std::array<std::vector<float>, 2> history;
    std::array<float, 2> held {};
    int write = 0, start = 0, loopLength = 1, play = 0, lastStep = -1, holdCounter = 0, recorded = 0;
    bool captured = false;
    float envelope = 0, slew = 0.005f;
public:
    void prepare(double sr)
    {
        for (auto& channel : history) channel.assign(static_cast<size_t>(sr * 6.0) + 4, 0);
        slew = static_cast<float>(1.0 - std::exp(-1.0 / (sr * 0.003)));
        reset();
    }
    void reset()
    {
        for (auto& channel : history) std::fill(channel.begin(), channel.end(), 0.0f);
        write = start = play = holdCounter = recorded = 0; loopLength = 1; lastStep = -1; held = {}; envelope = 0; captured = false;
    }
    std::array<float, 2> process(std::array<float, 2> input, int step, int samplesPerSlice,
                                float glitch, float lofi, float bits, float reduction) noexcept
    {
        const auto length = static_cast<int>(history[0].size());
        if (length == 0) return input;
        const bool active = step % 2 != 0;
        if (step != lastStep)
        {
            if (active)
            {
                loopLength = std::clamp(samplesPerSlice, 16, length / 2);
                captured = recorded >= loopLength;
                start = (write - loopLength + length) % length;
                play = 0;
            }
            lastStep = step;
        }
        envelope += slew * ((active && captured ? glitch : 0.0f) - envelope);
        const auto fadeSamples = std::max(1, std::min(96, loopLength / 10));
        const auto edge = std::min(1.0f, static_cast<float>(std::min(play, loopLength - 1 - play)) / static_cast<float>(fadeSamples));
        for (size_t ch = 0; ch < 2; ++ch)
        {
            history[ch][static_cast<size_t>(write)] = input[ch];
            const auto repeated = history[ch][static_cast<size_t>((start + play) % length)] * edge;
            input[ch] += envelope * (repeated - input[ch]);
        }
        if (holdCounter <= 0)
        {
            const auto levels = std::pow(2.0f, bits - 1.0f);
            for (size_t ch = 0; ch < 2; ++ch) held[ch] = std::round(std::clamp(input[ch], -4.0f, 4.0f) * levels) / levels;
            holdCounter = std::max(1, static_cast<int>(std::round(reduction)));
        }
        --holdCounter;
        for (size_t ch = 0; ch < 2; ++ch) input[ch] = lofi >= 1.0f ? held[ch] : input[ch] + lofi * (held[ch] - input[ch]);
        play = (play + 1) % loopLength;
        write = (write + 1) % length;
        recorded = std::min(recorded + 1, length);
        return input;
    }
};

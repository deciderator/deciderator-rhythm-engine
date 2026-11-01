# Aquanode / Aquaton attribution

This distribution contains an adaptation of Aquaton Reverb by aquanode (Aqua Node).
Project: https://github.com/aquanodemusic/aquanode-plugins
Original module: effects/aquaton-reverb/src/PluginProcessor.h and PluginProcessor.cpp.
Licence: GNU Affero General Public License version 3; see LICENSE in this directory.
The upstream README states AGPLv3 except where otherwise noted.

Original reference sources and README are preserved, unmodified, under aquaton-original/.
Downloaded source snapshot reviewed on 2026-11-01. The original PluginProcessor.cpp
SHA256 is 2CCA342995CC104F150C2C7BE7B7D45737FC24B5168AE518DD0D36529151AB28.

Modifications by the DECIDERATOR project, 2026-11-01, in Source/CreativeDSP.h:

- Allpass diffusion and Hadamard FDN structure adapted into SpaceFDN.
- Fixed eight-line network and twelve tank allpasses per line.
- Preallocated circular buffers and fixed matrix; no audio-thread matrix lock.
- Decay bounded below unity; no freeze, negative predelay or dynamic latency.
- Simplified stereo routing, filtering and modulation controls.

This is not the complete Aquaton plugin, nor an endorsement by its author.
No Aquanode artwork, branding, presets or samples are included.
TextureEngine (stutter and digital lo-fi) is DECIDERATOR code, not Granulate FX code.

AGPL-covered code is provided without warranty. Recipients may modify and
redistribute it under the applicable licence. Keep this notice and all original
licences with derived source and binary distributions.

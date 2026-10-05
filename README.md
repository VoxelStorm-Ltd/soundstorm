# SoundStorm

SoundStorm is a C++ sound and music library developed for VoxelStorm games, most notably **AdvertCity** and **sphereFACE**. It combines a software mixer for positional sound effects with independent streaming music decks, allowing a game to change its soundscape smoothly as the player moves, switches worlds, or enters a new area.

The library was originally built for these games rather than as a standalone audio framework. This repository contains the implementation and public headers; asset loading, game logic, and build integration belong to the host application.

## Features

- **Spatial sound with propagation delay.** Each effect is rendered separately for the listener's left and right ears, with inverse-square distance attenuation and travel time based on a speed of sound of 343 m/s. Changing the source or listener position changes the delay, producing a Doppler-like pitch shift naturally through the playback position.
- **Head-relative stereo for headphones and VR.** Listener position and quaternion orientation determine the ear positions. Direction-dependent head shadowing adds attenuation and extra delay at the far ear. This is a lightweight geometric HRTF approximation, rather than convolution with measured HRTF filters.
- **Controllable sound instances.** Play one-shot or looping effects, move them, change their volume or playback speed, replace the current sample, or attach a following sample. A `soundgroup` keeps the per-channel instances together so the application can control one logical sound.
- **High dynamic range mixing.** Effects have a per-asset `hdr_scale` as well as a per-instance volume. Loud effects raise an adaptive output scaling window, which then decays towards its baseline. This lets an explosion temporarily dominate the mix, including the music, without permanently lowering quieter effects.
- **Multiple music decks.** Two decks are created by default; the constructor accepts another count. Each has its own playlist, Vorbis decoder, PCM buffers, volume, and fade target. Decks continue advancing when muted, making it possible to switch between ongoing soundtracks without restarting them.
- **Intro-to-loop music and crossfades.** Queue several tracks to play in order; the final track repeats indefinitely by default. An intro followed by a loop therefore needs only two queue entries. Per-sample volume ramps provide smooth fades independently of the game's frame rate.
- **Memory-backed assets.** Effects use raw floating-point PCM and music uses compressed Ogg Vorbis data already in memory. The games embed these assets in their executables; the library does not require filesystem access during playback.
- **Device selection and diagnostics.** Enumerate devices, switch output devices, adjust master volume, and query stream time, sample rate, and CPU usage. PortAudio handles output, with an ALSA realtime-scheduling request on Linux.

## Integration and asset formats

Compile [soundstorm/soundstorm.cpp](soundstorm/soundstorm.cpp) into your application and include [soundstorm/soundstorm.h](soundstorm/soundstorm.h). The SoundStorm source uses C++17 features and GNU-style attributes; the VectorStorm headers in the current game checkouts require C++23, which those projects also select. It depends on:

- PortAudio and its **PortAudioCpp** C++ bindings;
- libogg, libvorbis, and libvorbisfile;
- VectorStorm headers providing `vec3f` and `quatf`;
- Boost headers (`boost/range/iterator_range.hpp`);
- the host-project helper headers `platform_defines.h` and `cast_if_required.h`, which are not bundled here;
- platform thread support (and the PortAudio ALSA extension on Linux).

There is currently no standalone build configuration or bundled asset pack. Add the dependency include paths and link libraries in your application's build system.

`load()` expects **headerless mono PCM consisting of native 32-bit floats**, suitably aligned for access as `float`. It does not decode WAV files. `music_load()` expects **stereo Ogg Vorbis** bytes; the decoder reads the first two channels directly. Prepare both kinds of asset at the output sample rate: the engine requests **44,100 Hz**, and does not convert asset sample rates. Output is stereo, non-interleaved floating-point audio, with a requested callback size of 64 frames. The channel enumeration includes other speaker names, but the implemented listener and music paths assume stereo.

Both loading calls retain **non-owning views** of the supplied memory. Keep the backing storage alive and at a stable address until SoundStorm has finished using it, preferably until after the `soundstorm` object is destroyed. Passing a temporary string, freeing an asset, or reallocating its storage invalidates that view. Music is streamed *from compressed memory*: only a portion is decoded to PCM at a time, but the compressed asset remains resident.

## Playing sound effects

The following function accepts application-owned PCM storage and plays a positional one-shot. The application keeps the engine and storage alive while playback runs; returning from this function does not wait for the sound to finish.

```cpp
#include <string_view>
#include <vector>
#include "soundstorm/soundstorm.h"

unsigned int load_effect(soundstorm &audio,
                         std::vector<float> const &pcm,
                         float const hdr_scale = 1.0f) {
  /// Register application-owned PCM without copying its storage
  std::string_view const buffer{
    reinterpret_cast<char const*>(pcm.data()),
    pcm.size() * sizeof(float),
  };
  return audio.load(buffer, hdr_scale);
}
void play_example(soundstorm &audio, std::vector<float> const &effect_pcm) {
  /// Play a one-shot to the listener's right
  auto const effect{load_effect(audio, effect_pcm, 0.5f)};
  audio.set_listener_position_and_rotation({0.0f, 0.0f, 0.0f},
                                           quatf::from_euler_angles(0.0f, 0.0f, 0.0f));
  audio.play(effect, {2.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f});
}
```

Constructing `soundstorm audio;` initializes the default output device and starts the mixer. Check `audio.enabled` before setting up playback if initialization may fail. Effects do not need the music streamer, and there is no per-frame mixer-update call. Update the listener from your camera or VR head pose, and update moving sources through `set_position()`.

Use metres for positions if you want the built-in propagation speed and ear spacing to have physical meaning. Avoid placing a source exactly at an ear: the attenuation calculation divides by squared distance. Although the API stores source and listener velocities, the current mixer does not integrate those values or use them for Doppler calculations; the application must actually update positions.

### Moving loops and variable pitch

This is the pattern used for sphereFACE's engines and charging weapons. Here `audio` is a live engine, `engine_effect` is an already loaded effect ID, and `engine` is retained by the game object across updates:

```cpp
soundstorm::soundgroup engine;

audio.play_loop(engine_effect,
                {0.0f, 0.0f, 4.0f},                                            // source position
                {0.0f, 0.0f, 0.0f},                                            // stored velocity
                0.0f,                                                          // initially silent
                0.0f, 0.0f,                                                    // start at zero; no early cutoff
                1.0f,                                                          // normal playback speed
                &engine);

// during later game updates
audio.set_position(engine, {1.0f, 0.0f, 3.0f});
audio.set_volume(engine, 0.7f);
audio.set_seek_speed(engine, 1.5f);                                            // raise pitch and playback speed together

// let the current iteration finish
audio.stop_loop(engine);

// or request an early stop as well
audio.stop(engine);
engine.clear();
```

`seek_start` and `seek_end` are sample-frame offsets, not seconds; `seek_end == 0` means the end of the asset. `seek_speed == 1` is normal playback speed. Break a loop with `stop_loop()` before calling `stop()`: `stop()` alone advances the cursor but leaves the successor/loop intact. Clearing a `soundgroup` alone does not stop playback. Use a fresh or cleared group when starting a new logical sound, because playback appends channel handles to the supplied group.

`replace(group, audio.get_effect(id))` switches the existing instances to another effect. `follow(group, audio.get_effect(id))` schedules an effect to succeed them. The original group still refers to the original instances after a successor takes over.

## Music decks and transitions

Load the music assets and prepare the initial playlists before calling `start_streamer()`. Call it once; use `stop_streamer()` before starting it again. The destructor stops and joins the streaming thread automatically.

This setup uses an intro and repeating loop on deck 0, with an alternative soundtrack on deck 1. All three views refer to application-owned Ogg data that outlives the engine:

```cpp
void prepare_music(soundstorm &audio,
                   std::string_view const intro_ogg,
                   std::string_view const loop_ogg,
                   std::string_view const alternate_ogg) {
  /// Prepare two independent soundtracks before starting the decoder
  auto const intro{audio.music_load(intro_ogg)};
  auto const loop{audio.music_load(loop_ogg)};
  auto const alternate{audio.music_load(alternate_ogg)};

  audio.music_queue(0, intro);                                                 // played once
  audio.music_queue(0, loop);                                                  // final entry repeats
  audio.music_queue(1, alternate);                                             // repeats on the other deck
  audio.set_music_volume(0, 0.8f);
  audio.set_music_volume(1, 0.0f);
  audio.start_streamer();
}
```

Once both soundtracks are running, a world change can call:

```cpp
audio.crossfade_music(2.0f, 0, 1);
```

`crossfade_music()` **swaps the two decks' target volumes** over the specified number of seconds. In this example deck 0 fades to zero and deck 1 to 0.8; calling it again swaps them back. It does not queue tracks, restart them, or always fade to full volume. Fades are linear, not equal-power. Use a positive duration, or `set_music_volume()` for an immediate change. `get_music_volume()` returns the **target** volume, even while a fade is in progress.

For a pause-menu fade that preserves the user's volume setting:

```cpp
auto const saved_volume{audio.get_music_volume(0)};
audio.fade_music_volume(0, 0.0f, 0.25f);

// on resume
audio.fade_music_volume(0, saved_volume, 0.25f);
```

Apply this to each deck as needed. Muting does not pause the music cursor. `set_master_volume()` scales the entire output, including effects and all music decks.

`music_clear(deck)` empties its playlist and asks the decoder thread to close its old Vorbis stream. It does **not** immediately erase already decoded PCM. Silence or fade out the deck before clearing and reusing it, and allow for buffering when timing a newly queued track. Each deck has two buffers of approximately two seconds each; startup and playlist changes are not immediate, sample-scheduled transport operations. The final playlist entry loops, rather than the whole playlist cycling back to its beginning. Prepare matching intro/loop boundaries in the assets for seamless musical joins.

### How the games use it

- **AdvertCity:** one deck carries the meatspace playlist and the other the cyberspace playlist. Both progress continuously, with only the current world's deck audible. Switching worlds triggers a two-second crossfade. Each playlist includes an introductory segment before the main tracks. See its `universe.cpp` and `input/callbacks_input.cpp`.
- **sphereFACE:** tracks are split into separate intro and loop assets, with selections grouped by world layer. Entering a tunnel queues the destination's intro and loop on the other deck, starts a three-second crossfade, and swaps the active/queued deck IDs. On arrival, the game clears the old deck for the next transition. Its pause menu optionally fades both decks out and restores their target levels on resume. See `universe/universe.cpp`, `entity/ship/playership.cpp`, and `universe/loop_pause.cpp`.
- **sphereFACE effects:** ship and rocket engines are persistent positional loops; thrust changes the ship engine's volume. Charging weapons raise loop playback speed as charge increases, and a singularity changes pitch with its remaining lifetime. VR rendering supplies the listener's head position and orientation. See `entity/ship/playership.cpp`, `entity/bullet/rocket.cpp`, `weapon/singularitycannon.cpp`, and `universe/graphics.cpp`.

These filenames refer to the consuming game projects; their assets and game-specific selection logic are not included in this repository.

## Architecture and thread model

The implementation lives in one `soundstorm` class. An effect library holds views of PCM assets; each playing effect creates one `sound` instance per output channel in a shared playing list. A `soundgroup` exposes those instances to the application. Music has a separate asset library and a vector of decks, each containing a playlist, Vorbis state, and left/right ping-pong buffers.

There are three execution contexts in a typical game:

1. **Application thread:** constructs the engine, registers assets, queues music, plays effects, and updates listener and source controls.
2. **PortAudio callback thread:** runs `mixer()`, renders effects with per-ear delay and attenuation, consumes decoded music, advances fades, applies HDR/master scaling, and writes the output buffers. It also retires completed effect instances.
3. **SoundStorm streaming thread:** started explicitly with `start_streamer()`, runs `streamer()` to decode Ogg Vorbis into the inactive deck buffers. It services all decks in one thread and polls refill flags, sleeping about 250 ms between passes at the default settings. Vorbis decoding stays outside the audio callback.

The callback flips buffers and marks the inactive side for refill; the streamer fills it while playback consumes the other side. Output-device changes stop the streamer and output stream, reinitialize the device and buffers, and restart streaming as appropriate.

**The current implementation has no explicit mutex or atomic synchronization for this shared state.** The application, callback, and streamer access shared containers, flags, and playback parameters directly, so the historical game usage and examples above are not a guarantee of C++ thread safety. Calling controls only from the main thread does not eliminate races with the audio threads. A production integration that requires defined concurrent behavior needs synchronization or a command handoff implemented inside the engine. The callback also removes shared objects and can log in debug builds, so it should not be described as allocation-free or strictly realtime-safe.

## Compile-time options

- `SOUNDSTORM_NO_SSE`: use scalar alternatives to the SSE intrinsics in mixing and fading; useful for non-x86 targets.
- `SOUNDSTORM_NO_STREAM_SEEK`: disable the Vorbis seek/tell callbacks and use the non-seekable playlist path. This changes decoder behavior, not the public playback-speed controls.
- `NSOUND`: disable sound output and most loading/playback work. PortAudio initialization remains part of construction; this does not remove the build dependencies.
- `DEBUG_SOUNDSTORM`: enable verbose diagnostics and session statistics, with an additional MemoryStorm dependency. This legacy diagnostic path currently references an obsolete `buffersize` variable in `load()` and needs repair before it can be enabled successfully.

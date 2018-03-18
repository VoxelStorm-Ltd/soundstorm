#ifndef SOUNDSTORM_H_INCLUDED
#define SOUNDSTORM_H_INCLUDED

#ifndef SOUNDSTORM_NO_STREAM_SEEK
  #define SOUNDSTORM_STREAM_SEEK_ENABLE
#endif // SOUNDSTORM_NO_STREAM_SEEK

#include <vector>
#include <list>
#include <queue>
#include <thread>
#include <string_view>
#include <portaudiocpp/PortAudioCpp.hxx>
#include <ogg/os_types.h>
#include <vorbis/vorbisfile.h>
#include "vectorstorm/vector/vector3.h"
#include "vectorstorm/quat/quat.h"

class soundstorm {
  /// VoxelStorm Sound Manager
  ///
  /// define DEBUG_SOUNDSTORM for detailed debugging output
  /// define SOUNDSTORM_NO_SSE to avoid using Intel SSE intrinsics
  /// define NSOUND to disable all sound output
  /// define SOUNDSTORM_NO_STREAM_SEEK to disable stream seeking support, which may have both performance gains and penalties
public:
  enum channel_type : size_t {                                                  // output channels used as array indices
    /// see http://en.wikipedia.org/wiki/Surround_sound#Standard_speaker_channels
    /// and http://xiph.org/vorbis/doc/vorbisfile/ov_read.html
    FRONT_LEFT   = 0,                                                           // standard stereo
    FRONT_RIGHT  = 1,                                                           // standard stereo
    LEFT         = 0,                                                           // default stereo alias for FRONT_LEFT
    RIGHT        = 1,                                                           // default stereo alias for FRONT_RIGHT
    CENTRE       = 2,                                                           // 1.0 mono
    BACK_LEFT    = 4,                                                           // 4.0 quad, 5.0, 6.0 (not surround), 7.0+
    BACK_RIGHT   = 5,                                                           // 4.0 quad, 5.0, 6.0 (not surround), 7.0+
    SUBWOOFER    = 5,                                                           // 5.1 (?)
    CENTRE_LEFT  = 6,                                                           // 7.0, 11.0
    CENTRE_RIGHT = 7,                                                           // 7.0, 11.0
    BACK_CENTRE  = 8,                                                           // 3.0 surr, 4.0 surr, 6.0, 6.0 side
    SIDE_LEFT    = 9,                                                           // 5.0 side+, 7.0+
    SIDE_RIGHT   = 10,                                                          // 5.0 side+, 7.0+
    HIGH_LEFT    = 11,                                                          // 9.0+
    HIGH_RIGHT   = 12                                                           // 9.0+
  };
  struct ear {
    /// A listener located in 3D space and having an orientation
    vec3f position;                                                             // absolute relative position in 3D space
    vec3f orientation = vec3f(0.0f, 0.0f, 1.0f);                                // normalised orientation vector
  };
  struct soundeffect {
    /// Stored uncompressed sound effects in memory
    std::basic_string_view<float> buffer;                                       // the sound sample itself, in 32bit float format
    std::vector<float> envelope;                                                // its volume envelope
    float hdr_scale = 1.0f;                                                     // fraction of max dynamic range, from 0 to 1 (or more to boost)
  };
  struct sound {
    /// A currently playing sound
    std::shared_ptr<soundeffect> effect;                                        // the library effect this is playing
    vec3f position;                                                             // its location in 3D space
    vec3f velocity;                                                             // its cached velocity through the medium
    float volume     = 1.0f;                                                    // fraction of max effect volume, from 0 to 1 (or more to boost)
    float seek       = 0.0f;                                                    // seek position inside the buffer, in buffer frames - floating point!
    float seek_end   = 0.0f;                                                    // where to cut off at before end of sample, 0 means play to the end
    float seek_speed = 1.0f;                                                    // how fast we're playing, can be negative but sound won't finish normally
    std::shared_ptr<sound> next_sound;                                          // sound to play immediately following this, if any
    unsigned int channel = 0;                                                   // which channel it's heard on - one sound per output channel!
    sound(std::shared_ptr<soundeffect> new_effect,
          vec3f const &new_position,
          vec3f const &new_velocity,
          float new_volume     = 1.0f,
          float new_seek       = 0.0f,
          float new_seek_end   = 0.0f,
          float new_seek_speed = 1.0f,
          sound *new_next_sound = nullptr,
          unsigned int new_channel = 0)
      : effect(std::move(new_effect)),
        position(new_position),
        velocity(new_velocity),
        volume(new_volume),
        seek(new_seek),
        seek_end(new_seek_end),
        seek_speed(new_seek_speed),
        next_sound(new_next_sound),
        channel(new_channel) {
      /// Specific constructor
    }
  };
  using soundgroup = std::vector<std::shared_ptr<sound>>;                       // all of the channel components that make up a sound
  struct deck;                                                                  // forward declaration for next struct
  struct music {
    soundstorm *parent = nullptr;                                               // pointer back to the class it belongs to, for passing to callbacks
    unsigned int parent_deck = 0;                                               // what deck it's being played on
    std::string_view buffer;                                                    // where this is stored
    ogg_int64_t seek = 0;                                                       // stream style position offset
  };
  struct deck {
    /// A music playback deck
    float volume           __attribute__((__aligned__(16))) = 1.0;              // how loud to play this current deck, 0-1 (but may exceed 1 for special effects)
    float volume_target    __attribute__((__aligned__(16))) = 1.0;              // what to fade towards, if anything
    float volume_fadespeed __attribute__((__aligned__(16))) = 0.0;              // how fast to fade
    unsigned int id = 0;                                                        // what index into the deck vector this is
    std::unique_ptr<OggVorbis_File> oggfile;                                    // the internal ogg vorbis handle
    std::queue<std::shared_ptr<music>> playlist;                                // what's playing now (front) and what to play next
    std::vector<float> buffer_l[2];                                             // ping-pong buffer pair, left channel
    std::vector<float> buffer_r[2];                                             // ping-pong buffer pair, right channel
    unsigned int buffer_read = 0;                                               // which ping-pong buffer we're reading from
    unsigned int buffer_read_seek = 0;                                          // where in the buffer we've read to
    #ifdef DEBUG_SOUNDSTORM
      static unsigned int constexpr const correct_checkvalue = 123456;
      unsigned int checkvalue = correct_checkvalue;                             // debug check value, this is the nearest we get to runtime type safety
    #endif // DEBUG_SOUNDSTORM
    bool repeat = true;                                                         // at the end of the playlist last entry repeats indefinitely - if not, deck outputs silence
    bool buffer_needs_filled = true;                                            // whether the currently selected write buffer needs to be filled
    bool clear_old_oggfile = false;                                             // whether we have an obsolete oggfile we have to close this round
  };

private:
  float hdr_window_top_min __attribute__((__aligned__(16))) = 1.0f;             // the high dynamic range window's upper limit can't fall below this
  float hdr_window_top     __attribute__((__aligned__(16))) = hdr_window_top_min; // the high dynamic range window's upper limit, may exceed 1
  float hdr_window_bottom  __attribute__((__aligned__(16))) = 0.0f;             // minimum hdr_scale for sounds to get played
  //float hdr_dropback_rate  __attribute__((__aligned__(16))) = 1.0 / samplerate * frames_per_buffer; // amount subtracted per buffer fill
  float hdr_dropback_rate  __attribute__((__aligned__(16))) = 0.995f;           // scaling multiplier per buffer fill

  std::thread streamer_thread;                                                  // thread for the streaming decoder

  portaudio::System *audio_system = nullptr;
  portaudio::Device const *audio_device = nullptr;
  portaudio::AutoSystem audio_system_auto;

  unsigned long frames_per_buffer = 64;                                         // frames per buffer
  unsigned int num_devices = 0;                                                 // number of known devices
  unsigned int channels = 2;                                                    // output channels
  float samplerate = 44100.0f;                                                  // output sample rate
  static float constexpr const speed_of_sound = 343.0f;                         // speed of sound in air, m/s
  static float constexpr const ear_offset = 0.115f;                             // distance of ear from the centre of the head, metres
  static float constexpr const head_shadow_time = 0.000660f;                    // measured max pan head shadow time, seconds (see http://en.wikipedia.org/wiki/Interaural_time_difference#Duplex_theory)
  // how much extra time delay to add to the far sound for head shadow effect at full pan:
  static float constexpr const head_shadow_delay_max = head_shadow_time - (ear_offset / speed_of_sound);
  // how much the opposite ear is shadowed by the head, realistic ~= 6.4dB, we're going for somewhat of an exaggeration:
  static float constexpr const head_shadow_attenuation = 0.9f;
  unsigned int num_decks = 2;                                                   // how many music decks we're currently using
  unsigned int deck_buffer_size = static_cast<unsigned int>(samplerate * 2.0f); // how many pcm frames to buffer for each deck buffer - this is the minimum pre-loaded at one time

  //std::unique_ptr<portaudio::DirectionSpecificStreamParameters> stream_in_params;
  std::unique_ptr<portaudio::DirectionSpecificStreamParameters> stream_out_params;
  std::unique_ptr<portaudio::StreamParameters>                  stream_params;
  std::unique_ptr<portaudio::MemFunCallbackStream<soundstorm>>  stream;

  std::vector<ear> ears;                                                        // the listeners for each output channel
  std::vector<std::shared_ptr<soundeffect>> effect_library;                     // the sound effects
  std::list<std::shared_ptr<sound>> playing;                                    // currently playing sounds
  std::vector<std::string_view> music_library;                                  // the music buffers
  std::vector<deck> decks;                                                      // music decks control what music is currently playing

  float volume_master = 1.0;                                                    // global output volume control, from 0 to 1 (although possible to go outside this)

  vec3f listener_position;                                                      // where the listener is
  quatf listener_rotation;                                                      // which way the listener's facing
  vec3f listener_velocity;                                                      // listener's velocity through the medium

  bool streamer_run = true;                                                     // whether to keep running the streamer

  #ifdef DEBUG_SOUNDSTORM
    float session_max_hdr_window_top    = hdr_window_top;                       // keep track of some session-wide extremes for debugging
    float session_max_hdr_window_bottom = hdr_window_bottom;
    unsigned int session_max_simultaneous_sounds = 0;
    float session_min_distance = 10000.0f;
    float session_max_distance = 0.0f;
    size_t session_music_samples_read = 0;
  #endif // DEBUG_SOUNDSTORM

public:
  bool enabled = false;                                                         // whether to use the sound system - if disabled, all play functions exit early

  soundstorm();
  explicit soundstorm(unsigned int number_of_decks);
  ~soundstorm();

  void init_device();
  void resize_decks();
  void shutdown_device();
  void restart_device();
  void start_streamer();
  void stop_streamer();

  int mixer(void const *buffer_in,
            void *buffer_out,
            unsigned long frames,
            PaStreamCallbackTimeInfo const *time_info,
            PaStreamCallbackFlags status_flags);
  void streamer();
  static size_t ogg_callback_read( void *ptr, size_t size, size_t count, void *datasource);
  static int    ogg_callback_seek( void *datasource, ogg_int64_t offset, int origin);
  static int    ogg_callback_close(void *datasource);
  #ifdef NDEBUG
    static long ogg_callback_tell( void *datasource) __attribute__((__pure__));
  #else
    static long ogg_callback_tell( void *datasource);
  #endif // NDEBUG

  // devices
  unsigned int get_device_default() const;
  unsigned int get_device_current() const;
  void get_device_list(         std::vector<std::pair<unsigned int, std::string>> &target_list) const;
  void get_device_list_out_only(std::vector<std::pair<unsigned int, std::string>> &target_list) const;
  void get_device_list_in_only( std::vector<std::pair<unsigned int, std::string>> &target_list) const;
  void set_device(unsigned int new_device_index);

  // sound system
  unsigned int get_num_decks() const __attribute__((__pure__));
  void set_num_decks(unsigned int new_num_decks);

  // statistics
  double get_cpu_usage() const;
  double get_sample_rate() const;
  double get_time() const;
  void dump_stats() const;
  #ifdef DEBUG_SOUNDSTORM
    void dump_session_report() const;
  #else
    void dump_session_report() const __attribute__((__const__));
  #endif // DEBUG_SOUNDSTORM
  void dump_device_info();

  // state
  vec3f const &get_listener_position() const __attribute__((__const__));
  quatf const &get_listener_rotation() const __attribute__((__const__));
  vec3f const &get_listener_velocity() const __attribute__((__const__));
  void set_listener_position(vec3f const &newposition);
  void set_listener_rotation(quatf    const &newrotation);
  void set_listener_velocity(vec3f const &newvelocity);
  void set_listener_position_and_rotation(vec3f const &newposition, quatf const &newrotation);
  void update_ears();
  float get_master_volume() const __attribute__((__pure__));
  void set_master_volume(float newvolume);

  // library
  std::shared_ptr<soundeffect> get_effect(unsigned int effect_id) const __attribute__((__pure__));
  std::string_view get_music(unsigned int music_id)  const __attribute__((__pure__));
  unsigned int load(std::string_view buffer, float hdr_scale = 1.0);
  unsigned int music_load(std::string_view buffer);

  // playback control
  void play(     unsigned int effect_id, vec3f const &position, vec3f const &velocity, float volume = 1.0f, float seek_start = 0.0f, float seek_end = 0.0f, float seek_speed = 1.0f, soundgroup *thissoundgroup = nullptr);
  void play(     vec3f const &position, vec3f const &velocity, std::shared_ptr<soundeffect> effect,    float volume = 1.0f, float seek_start = 0.0f, float seek_end = 0.0f, float seek_speed = 1.0f, soundgroup *thissoundgroup = nullptr);
  void play_loop(unsigned int effect_id, vec3f const &position, vec3f const &velocity, float volume = 1.0f, float seek_start = 0.0f, float seek_end = 0.0f, float seek_speed = 1.0f, soundgroup *thissoundgroup = nullptr);
  void play_loop(vec3f const &position, vec3f const &velocity, std::shared_ptr<soundeffect> effect,    float volume = 1.0f, float seek_start = 0.0f, float seek_end = 0.0f, float seek_speed = 1.0f, soundgroup *thissoundgroup = nullptr);
  std::shared_ptr<music> music_queue(unsigned int deck_id, unsigned int music_id);
  #ifdef NDEBUG
    float get_music_volume(unsigned int deck_id) __attribute__((__pure__));
  #else
    float get_music_volume(unsigned int deck_id);
  #endif // NDEBUG
  void set_music_volume(unsigned int deck_id, float newvolume);
  void fade_music_volume(unsigned int deck_id, float newvolume, float seconds_to_take);
  void crossfade_music(float seconds_to_take, unsigned int deck_from = 0, unsigned int deck_to = 1);
  void stop(          soundgroup const &thissoundgroup);
  void stop_loop(     soundgroup const &thissoundgroup);
  void replace(       soundgroup const &thissoundgroup, std::shared_ptr<soundeffect> neweffect, float seek_start = 0.0f, float seek_end = 0.0f, float seek_speed = 1.0f);
  void follow(        soundgroup const &thissoundgroup, std::shared_ptr<soundeffect> neweffect, float seek_start = 0.0f, float seek_end = 0.0f, float seek_speed = 1.0f);
  void set_volume(    soundgroup const &thissoundgroup, float newvolume);
  void set_position(  soundgroup const &thissoundgroup, vec3f const &newposition);
  void set_seek_speed(soundgroup const &thissoundgroup, float newspeed);
  void music_clear(unsigned int deck_id);
};

#endif // SOUNDSTORM_H_INCLUDED

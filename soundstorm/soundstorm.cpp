
size_t soundstorm::ogg_callback_read(void *ptr, size_t size, size_t count, void *datasource) {
  /// The interface is identical to that of fread, and identical behaviour is expected
  #ifdef DEBUG_SOUNDSTORM
    //std::cout << "SoundStorm: DEBUG: streamer read requested size " << size << " count " << count << std::endl;
  #endif // DEBUG_SOUNDSTORM
  if(!datasource) {
    #ifdef DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
    #endif // DEBUG_SOUNDSTORM
    return 0;                                                                   // nullptr means we've got nothing playing
  }
  deck *thisdeck = reinterpret_cast<deck*>(datasource);
  #ifdef DEBUG_SOUNDSTORM
    if(thisdeck->playlist.empty()) {
      std::cout << "SoundStorm: DEBUG: streamer read called on deck with empty playlist, this should be checked for in advance." << std::endl;
      return 0;
    }
  #endif // DEBUG_SOUNDSTORM
  music *thismusic = thisdeck->playlist.front();
  unsigned char *target = reinterpret_cast<unsigned char*>(ptr);
  #ifdef DEBUG_SOUNDSTORM
    if(thisdeck->checkvalue != 123456) {
      std::cout << "SoundStorm: DEBUG: streamer read: check value incorrect: " << thisdeck->checkvalue << std::endl;
      return 0;
    }
    if(!thismusic->buffer) {
      std::cout << "SoundStorm: DEBUG: no library music assigned to this playlist entry - this should never happen!" << std::endl;
      return 0;
    }
    if(!thismusic->buffer->buffer) {
      std::cout << "SoundStorm: DEBUG: no buffer assigned to this music - this should never happen!" << std::endl;
      return 0;
    }
  #endif // DEBUG_SOUNDSTORM

  unsigned int const bytes = cast_if_required<unsigned int>(size * count);
  unsigned int i = 0;
  for(; i != bytes; ++i) {
    target[i] = thismusic->buffer->buffer[thismusic->seek];
    //memcpy(ptr, thismusic->buffer->buffer, size * count);
    ++thismusic->seek;
    if(thismusic->seek == thismusic->buffer->buffersize) {
      // advance the playlist
      #ifdef DEBUG_SOUNDSTORM
        std::cout << "SoundStorm: DEBUG: advancing playlist after " << thismusic->seek / 1024 << "KB played" << std::endl;
      #endif // DEBUG_SOUNDSTORM
      thismusic->seek = 0;                                                      // rewind so that we advance to 0 this frame
      if(thisdeck->playlist.size() == 1) {
        // no track queued after this
        if(!thisdeck->repeat) {
          delete thismusic;
          thismusic = nullptr;
          #ifdef DEBUG_SOUNDSTORM
            std::cout << "SoundStorm: DEBUG: streamer finished playlist, not repeating after " << i << " bytes" << std::endl;
          #endif // DEBUG_SOUNDSTORM
          // create a short read
          return i;
        }
        #ifdef DEBUG_SOUNDSTORM
          std::cout << "SoundStorm: DEBUG: streamer finished playlist, repeating" << std::endl;
        #endif // DEBUG_SOUNDSTORM
        // otherwise we just play this track from the start
      } else {
        #ifdef DEBUG_SOUNDSTORM
#include "soundstorm.h"
          std::cout << "SoundStorm: DEBUG: streamer replacing music with top of playlist" << std::endl;
#include <iostream>
        #endif // DEBUG_SOUNDSTORM
#include "platform_defines.h"
        delete thismusic;
#include "cast_if_required.h"
        thisdeck->playlist.pop();                                               // take the current entry off the playlist
#ifndef SOUNDSTORM_NO_SSE
        thismusic = thisdeck->playlist.front();                                 // tell the track to replace itself
  #include <xmmintrin.h>
      }
#endif // SOUNDSTORM_NO_SSE
    }
#ifdef PLATFORM_LINUX
    // NOTE: could probably improve this using a block memcpy or std::copy
  #include <pa_linux_alsa.h>
  }
#endif // PLATFORM_LINUX
  #ifdef DEBUG_SOUNDSTORM
#ifndef NDEBUG
    //std::cout << "SoundStorm: DEBUG: streamer read completed with " << i << " bytes" << std::endl;
  #include <cassert>
  #endif // DEBUG_SOUNDSTORM
#endif // NDEBUG
  return i;

}
soundstorm::soundstorm()

  : soundstorm(2) {
  /// Default wrapper constructor
int soundstorm::ogg_callback_seek(void *datasource, ogg_int64_t offset, int origin) {
}
  /// The interface is identical to that of fseek, and identical behaviour is expected

  if(!datasource) {
soundstorm::soundstorm(unsigned int number_of_decks) try
    #ifdef DEBUG_SOUNDSTORM
  : audio_system_auto(false),                                                   // don't initialise portaudio automatically
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
    num_decks(number_of_decks),                                                 // optional setting of number of decks
    #endif // DEBUG_SOUNDSTORM
    listener_rotation(quatd::from_euler_angles(0.0, 0.0, 0.0)) {
    return 0;                                                                   // nullptr means we've got nothing playing
  /// Specific constructor with integrated try/catch
  }
  // Initialise all global audio setup
  deck *thisdeck = reinterpret_cast<deck*>(datasource);
  try {
  music *thismusic = thisdeck->playlist.front();
    audio_system_auto.initialize();

    audio_system = &portaudio::System::instance();
  switch(origin) {
  } catch(portaudio::PaException const &e) {
  case SEEK_SET:
    std::cout << "SoundStorm: audio_system: PortAudio exception: " << e.paErrorText() << std::endl;
    thismusic->seek = offset;
    return;
    break;
  } catch(portaudio::PaCppException const &e) {
  case SEEK_CUR:
    std::cout << "SoundStorm: audio_system: PortAudioCpp exception: " << e.what() << std::endl;
    thismusic->seek += offset;
    return;
    break;
  } catch(std::exception const &e) {
  case SEEK_END:
    std::cout << "SoundStorm: audio_system: Generic exception: " << e.what() << std::endl;
    #ifdef DEBUG_SOUNDSTORM
    return;
      if(!thismusic->buffer) {
  } catch(...) {
    std::cout << "SoundStorm: audio_system: Unknown exception!" << std::endl;
        std::cout << "SoundStorm: DEBUG: no library music assigned to this playlist entry passed as datasource to ogg_callback_seek - this should never happen!" << std::endl;
    return;
        return -1;
  }
      }
  std::cout << "SoundStorm: " << audio_system->versionText() << " build " << audio_system->version() << std::endl;
    #endif // DEBUG_SOUNDSTORM
  if(!audio_system->exists()) {
    thismusic->seek = thismusic->buffer->buffersize + offset;
    std::cout << "SoundStorm: Error: sound engine claims not to exist." << std::endl;
    break;
    return;
  default:
  }
    return -1;

  }
  // select an appropriate sound device
  return 0;
  audio_device = &audio_system->defaultOutputDevice();
}
  dump_device_info();                                                           // this also updates num_devices

  init_device();                                                                // initialise the currently selected device
int soundstorm::ogg_callback_close(void *datasource __attribute__((__unused__))) {

  /// The interface is identical to that of fclose, and identical behaviour is expected
  set_listener_position_and_rotation(vec3f(0.0f, 0.0f, 0.0f), quatf::from_euler_angles(0.0f, 0.0f, 0.0f)); // initial positions for the ears
  // we do absolutely nothing
} catch(portaudio::PaException const &e) {
  return 0;
  std::cout << "SoundStorm: PortAudio exception: " << e.paErrorText() << std::endl;
}
} catch(portaudio::PaCppException const &e) {

  std::cout << "SoundStorm: PortAudioCpp exception: " << e.what() << std::endl;
long soundstorm::ogg_callback_tell(void *datasource) {
} catch(std::exception const &e) {
  /// The interface is identical to that of ftell, and identical behaviour is expected
  std::cout << "SoundStorm: Generic exception: " << e.what() << std::endl;
  if(!datasource) {
} catch(...) {
    #ifdef DEBUG_SOUNDSTORM
  std::cout << "SoundStorm: Unknown exception!" << std::endl;
}
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;

    #endif // DEBUG_SOUNDSTORM
soundstorm::~soundstorm() {
    return 0;                                                                   // nullptr means we've got nothing playing
  /// Default destructor
  }
  dump_session_report();
  deck *thisdeck = reinterpret_cast<deck*>(datasource);
  stream->abort();                                                              // tell the stream to stop without waiting for the buffers to finish
  return cast_if_required<long>(thisdeck->playlist.front()->seek);
  shutdown_device();
}
  stop_streamer();

  auto playing_backup = playing;                                                // so we can delete these after
unsigned int soundstorm::get_device_default() const {
  playing.clear();
  decks.clear();
  /// Return the number of the default audio device
  for(auto &it : playing_backup) {
  return audio_system->defaultOutputDevice().index();
    delete it;
}
  }

  for(auto &it : effect_library) {
unsigned int soundstorm::get_device_current() const {
    delete it;
  /// Return the number of the currently selected audio device
  }
  return audio_device->index();
  for(auto &it : music_library) {
}
    delete it;

  }
void soundstorm::get_device_list(std::vector<std::pair<unsigned int, std::string>> &target_list) const {
  effect_library.clear();
  /// Populate a vector with a set of strings describing devices, intended to let users choose from a menu
  audio_system->terminate();                                                    // release audio resources
}
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {

    std::stringstream ss;
void soundstorm::init_device() {
    //ss << it.index() << " " << it.name();
  /// Initialise the currently selected audio device and start the stream
    ss << it.name();
  ears.resize(2);                                                               // safe default number of ears, in case we exit initialisation early
    ss << " Ch " << it.maxInputChannels() << "in " << it.maxOutputChannels() << "out " << it.defaultSampleRate() << "Hz";
  if(num_devices == 0) {
    //ss << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
    enabled = false;
    if(it.isSystemDefaultInputDevice()) {
    std::cout << "SoundStorm: No audio devices found!  Cancelling initialisation." << std::endl;
      ss << " [sysdef in]";
    return;
    }
  }
    if(it.isSystemDefaultOutputDevice()) {
  if(channels > static_cast<unsigned int>(audio_device->maxOutputChannels())) {
      ss << " [sysdef out]";
    // clamp number of channels in case we're asking for too many
    }
    channels = audio_device->maxOutputChannels();
  }
    if(it.isHostApiDefaultInputDevice()) {
  if(channels < 2) {
      ss << " [apidef in]";
    channels = 2;                                                               // but also clamp to a minimum of 2, since the engine assumes stereo separation
    }
    enabled = false;
    if(it.isHostApiDefaultOutputDevice()) {
    std::cout << "SoundStorm: Fewer than two channels found on selected device!  Cancelling initialisation." << std::endl;
      ss << " [apidef out]";
    return;
    }
  }
    if(it.isInputOnlyDevice()) {
  #ifdef DEBUG_SOUNDSTORM
      ss << " [in only]";
    std::cout << "SoundStorm: DEBUG: Enabling " << channels << " channels." << std::endl;
    }
  #endif // DEBUG_SOUNDSTORM
    if(it.isOutputOnlyDevice()) {
  ears.resize(channels);
      ss << " [out only]";

    }
  if(stream) {
    if(it.isFullDuplexDevice()) {
    shutdown_device();                                                          // make sure we clean up any already open streams
      ss << " [fd]";
  }
    }

    target_list.emplace_back(it.index(), ss.str());
  // set up the parameters required to open a (Callback)Stream:
  }
  stream_out_params = new portaudio::DirectionSpecificStreamParameters(
}
    *audio_device,                                                              // device
void soundstorm::get_device_list_out_only(std::vector<std::pair<unsigned int, std::string>> &target_list) const {
    channels,                                                                   // output channels
  /// Populate a vector with a set of strings describing devices, listing output devices only
    portaudio::FLOAT32,                                                         // sample data format http://riot.so/portaudiocpp/a00060.html#a30bc71f065706d41a5d9208ea861e4a6
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    false,                                                                      // interleaved
    if(it.isInputOnlyDevice() || it.maxOutputChannels() == 0) {
    audio_device->defaultLowOutputLatency(),                                    // latency
      continue;
    NULL);                                                                      // hostApiSpecificStreamInfo
    }
  stream_params = new portaudio::StreamParameters(
    std::stringstream ss;
    portaudio::DirectionSpecificStreamParameters::null(),                       // input stream parameters
    //ss << it.index() << " " << it.name();
    *stream_out_params,                                                         // output stream parameters
    ss << it.name();
    samplerate,                                                                 // sample rate
    ss << " Ch " << it.maxOutputChannels() << "out " << it.defaultSampleRate() << "Hz";
    frames_per_buffer,                                                          // frames per buffer for a CallbackStream, or the preferred buffer granularity for a BlockingStream.
    //ss << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
    paClipOff | paDitherOff);                                                   // The flags for the stream, default paNoFlag http://portaudio.com/docs/v19-doxydocs/portaudio_8h.html#ad33384abe3754a39f4773f2561773595

    if(it.isSystemDefaultOutputDevice()) {
  stream = new portaudio::MemFunCallbackStream<soundstorm>(
      ss << " [sysdef out]";
    *stream_params,                                                             // stream parameters
    }
    *this,                                                                      // class instance
    if(it.isHostApiDefaultOutputDevice()) {
    &soundstorm::mixer);                                                        // member function to call
      ss << " [apidef out]";

    }
  #ifdef PLATFORM_LINUX
    target_list.emplace_back(it.index(), ss.str());
    int alsacard;
  }
    int const error = PaAlsa_GetStreamOutputCard(stream->paStream(), &alsacard);
}
    if(error != 0) {
void soundstorm::get_device_list_in_only(std::vector<std::pair<unsigned int, std::string>> &target_list) const {
      std::cout << "SoundStorm: Error querying ALSA for stream output card: " << error << std::endl;
  /// Populate a vector with a set of strings describing devices, listing input devices only
    }
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    std::cout << "SoundStorm: Requesting realtime scheduling from ALSA on card " << alsacard << std::endl;
    if(it.isOutputOnlyDevice() || it.maxInputChannels() == 0) {
    PaAlsa_EnableRealtimeScheduling(stream->paStream(), true);
      continue;
  #endif // PLATFORM_LINUX
    }

    std::stringstream ss;
  samplerate = static_cast<float>(stream->sampleRate());                        // cache sample rate
    //ss << it.index() << " " << it.name();
  deck_buffer_size = static_cast<unsigned int>(samplerate * 2.0f);              // update buffer size
    ss << it.name();
  resize_decks();                                                               // needs to happen before mixer starts
  #ifndef NSOUND
    ss << " Ch " << it.maxInputChannels() << "in " << it.defaultSampleRate() << "Hz";
    stream->start();                                                            // start the stream
    //ss << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
  #endif // NSOUND
    if(it.isSystemDefaultInputDevice()) {
  enabled = true;
      ss << " [sysdef in]";
  std::cout << "SoundStorm: Initialised." << std::endl;
    }
}
    if(it.isHostApiDefaultInputDevice()) {
void soundstorm::resize_decks() {
      ss << " [apidef in]";
  /// Resize the decks to the correct number and buffer size, and reset the buffers
    }
  // decks must be initialised before the mixer starts
    target_list.emplace_back(it.index(), ss.str());
  decks.resize(num_decks);
  for(deck &thisdeck : decks) {
  }
    // initialise deck output buffers to zero
}
    thisdeck.buffer_l[0].resize(deck_buffer_size, 0.0f);

    thisdeck.buffer_r[0].resize(deck_buffer_size, 0.0f);
void soundstorm::set_device(unsigned int new_device_index) {
    thisdeck.buffer_l[1].resize(deck_buffer_size, 0.0f);
  /// Switch to a different output device
    thisdeck.buffer_r[1].resize(deck_buffer_size, 0.0f);
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    //thisdeck.buffer_read = 1;                                                   // so that buffer 0 will be pre-filled
    if(it.index() == static_cast<int>(new_device_index)) {
    thisdeck.buffer_read = 0;
      if(audio_device == &it) {
    thisdeck.buffer_needs_filled = true;
        std::cout << "SoundStorm: Switching to already selected device " << new_device_index << std::endl;
  }
      } else {
}
        std::cout << "SoundStorm: Switching to device " << new_device_index << ": " << it.name() << std::endl;
void soundstorm::shutdown_device() {
        audio_device = &it;
  /// Shut down the current output device and free it, in preparation of exit or re-init
        restart_device();
  if(stream) {
      }
    if(!stream->isStopped()) {
      stream->stop();
      return;
    }
    }
    stream->close();
  }
    delete stream;
  std::cout << "SoundStorm: Tried to switch to nonexistent device number " << new_device_index << std::endl;
    stream = nullptr;
}
  }

  delete stream_out_params;
unsigned int soundstorm::get_num_decks() const {
  stream_out_params = nullptr;
  return num_decks;
  delete stream_params;
}
  stream_params = nullptr;
void soundstorm::set_num_decks(unsigned int new_num_decks) {
  enabled = false;
  /// Change the number of decks
  std::cout << "SoundStorm: Shutdown complete." << std::endl;
}
  if(new_num_decks == num_decks) {

    return;                                                                     // exit silently if the change is a noop
void soundstorm::restart_device() {
  }
  /// Shut down and reinitialise the stream with the currently selected device - needed after device change
  std::cout << "SoundStorm: Changing number of decks from " << num_decks << " to " << new_num_decks << std::endl;
  std::cout << "SoundStorm: Restarting device..." << std::endl;
  bool const reinitialise_streamer = streamer_run;
  bool const reinitialise_streamer = streamer_run;
  if(reinitialise_streamer) {                                                   // if the streamer's currently running, shut it down
  if(reinitialise_streamer) {                                                   // if the streamer's currently running, shut it down
    stop_streamer();
    stop_streamer();
  }
  }
  shutdown_device();
  shutdown_device();
  num_decks = new_num_decks;
  init_device();
  init_device();
  if(reinitialise_streamer) {                                                   // if the streamer was running at restart, restart it
  if(reinitialise_streamer) {                                                   // if the streamer was running at restart, restart it
    start_streamer();
  }
    start_streamer();
}
  }

}
void soundstorm::start_streamer() {

  /// Initialise the streamer this must be done after the playlist is initialised
double soundstorm::get_cpu_usage() const {
  std::cout << "SoundStorm: Starting streamer thread..." << std::endl;
  /// Value normally from 0 to 1, but may exceed 1
  if(decks.empty()) {
  return stream->cpuLoad();
    std::cout << "SoundStorm: ERROR: streamer asked to play from non-existant decks." << std::endl;
}
    return;

  }
double soundstorm::get_sample_rate() const {
  //streamer_run = false;
  /// Get measured sample rate of this stream so far
  //streamer();
  return stream->sampleRate();
  //for(deck &thisdeck : decks) {
}
  //  thisdeck.buffer_read = 0;                                                   // so the pre-filled buffer will be played first

  //}
double soundstorm::get_time() const {
  streamer_run = true;
  /// Get the time spent playing the stream up to this point

  return stream->time();
  // start the streaming decoder thread
}
  #ifndef NSOUND

    streamer_thread = new std::thread(std::bind(&soundstorm::streamer, this));
void soundstorm::dump_stats() const {
  #endif // NSOUND
}
  /// Output some measured statistics about the stream

  std::cout << "SoundStorm: Sample rate " << get_sample_rate() << "Hz, CPU " << get_cpu_usage() * 100 << "%, time " << get_time() << std::endl;
void soundstorm::stop_streamer() {
}
  /// Stop the streamer thread if it's running
void soundstorm::dump_session_report() const {
  if(streamer_thread) {
  /// Output debugging statistics collected during the session
    std::cout << "SoundStorm: Stopping streamer thread..." << std::endl;
  #ifdef DEBUG_SOUNDSTORM
    streamer_run = false;                                                       // tell the streamer not to run another cycle
    std::cout << "SoundStorm session report: HDR window max: " << session_max_hdr_window_bottom << " - " << session_max_hdr_window_top << std::endl;
    // available with boost::thread only, not std::thread:
    std::cout << "SoundStorm session report: max simultaneous sounds " << session_max_simultaneous_sounds << std::endl;
    //streamer_thread->interrupt();                                               // and send an interrupt signal for quicker cleanup
    std::cout << "SoundStorm session report: source distance min " << session_min_distance << " max " << session_max_distance << std::endl;
    if(streamer_thread->joinable()) {
  #endif // DEBUG_SOUNDSTORM
      try {
}
        streamer_thread->join();                                                // wait for the streamer thread to finish
void soundstorm::dump_device_info() {
      } catch(...) {                                                            // ignore exceptions
  /// Output info about the devices available and the currently selected device, and update device count
      }
  num_devices = 0;
    }
    delete streamer_thread;
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    streamer_thread = nullptr;
    ++num_devices;
  }
    if(&it == audio_device) {                                                   // mark the currently selected device
}
      std::cout << " *";

    } else {
int soundstorm::mixer(void const *buffer_in __attribute__((__unused__)),
      std::cout << "  ";
                      void *buffer_out,
    }
                      unsigned long frames,
    std::cout << it.index() << " " << it.name();
                      PaStreamCallbackTimeInfo const *time_info __attribute__((__unused__)),
    std::cout << " Ch " << it.maxInputChannels() << "in " << it.maxOutputChannels() << "out " << it.defaultSampleRate() << "Hz";
                      PaStreamCallbackFlags status_flags __attribute__((__unused__))) {
    //std::cout << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
  /// The callback that feeds sample data to the sound stream
    if(it.isSystemDefaultInputDevice()) {
  #ifdef DEBUG_SOUNDSTORM
      std::cout << " [sysdef in]";
    assert(buffer_out != NULL);
    }
    if(playing.size() > session_max_simultaneous_sounds) {
    if(it.isSystemDefaultOutputDevice()) {
      session_max_simultaneous_sounds = static_cast<unsigned int>(playing.size());
      std::cout << " [sysdef out]";
    }
    }
  #endif // DEBUG_SOUNDSTORM
    if(it.isHostApiDefaultInputDevice()) {

      std::cout << " [apidef in]";
  // check and slide the HDR window
    }
  //hdr_window_top    -= hdr_dropback_rate;                                       // first droop by the default amount
    if(it.isHostApiDefaultOutputDevice()) {
  //hdr_window_bottom -= hdr_dropback_rate;
      std::cout << " [apidef out]";
  hdr_window_top    *= hdr_dropback_rate;                                       // first droop by the default amount
    }
  hdr_window_bottom *= hdr_dropback_rate;

    if(it.isInputOnlyDevice()) {
  #ifdef SOUNDSTORM_NO_SSE
      std::cout << " [in only]";
    if(hdr_window_top < hdr_window_top_min) {                                   // clamp
    }
      hdr_window_top = hdr_window_top_min;
    if(it.isOutputOnlyDevice()) {
    }
      std::cout << " [out only]";
    if(hdr_window_bottom < 0.0f) {                                              // clamp
    }
      hdr_window_bottom = 0.0f;
    if(it.isFullDuplexDevice()) {
    }
      std::cout << " [fd]";
  #else
    }
    _mm_store_ss(&hdr_window_top,    _mm_max_ss(_mm_set_ss(hdr_window_top),    _mm_set_ss(hdr_window_top_min))); // SSE intrinsicts: branchless max
    std::cout << std::endl;
    _mm_store_ss(&hdr_window_bottom, _mm_max_ss(_mm_set_ss(hdr_window_bottom), _mm_set_ss(0.0f))); // SSE intrinsicts: branchless max
  }
  #endif // SOUNDSTORM_NO_SSE
}


  float **out = static_cast<float**>(buffer_out);
vec3f const &soundstorm::get_listener_position() const {
  for(unsigned int i = 0; i != frames; ++i) {
  /// Return the current listener world position
    for(unsigned int channel = 0; channel != channels; ++channel) {             // clear all channels
  return listener_position;
      out[channel][i] = 0.0;
}
    }
quatf const &soundstorm::get_listener_rotation() const {
    float max_level __attribute__((__aligned__(16))) = 0.0;                     // keep track of the maximum output level this frame
  /// Return the current listener rotation

  return listener_rotation;
    // play the sound effects
}
    for(auto it = playing.begin(); it != playing.end();) {
vec3f const &soundstorm::get_listener_velocity() const {
      sound &thissound = **it;
      // stereo positioning
  /// Return the current listener world position
      ear const &thisear = ears[thissound.channel];
  return listener_velocity;
      float const distance_sq = (thisear.position - thissound.position).length_sq();
}
      float const distance = std::sqrt(distance_sq);

      #ifdef DEBUG_SOUNDSTORM
void soundstorm::set_listener_position(vec3f const &newposition) {
        if(distance < session_min_distance) {
  /// Update the world position of the listener - also updates the velocity
          session_min_distance = distance;
  listener_velocity = newposition - listener_position;
        }
  listener_position = newposition;
        if(distance > session_max_distance) {
  update_ears();
          session_max_distance = distance;
}
        }
void soundstorm::set_listener_rotation(quatf const &newrotation) {
      #endif // DEBUG_SOUNDSTORM
  /// Update the facing direction of the listener
      float const seek_delay = distance / speed_of_sound;
  listener_rotation = newrotation;
      #ifdef DEBUG_SOUNDSTORM
  update_ears();
        //std::cout << "DEBUG: " << playing.size() << " pos " << vec3i(thissound.position) << " dist " << distance << " ch" << thissound.channel << " delay " << seek_delay << "s" << std::endl;
}
      #endif // DEBUG_SOUNDSTORM
void soundstorm::set_listener_velocity(vec3f const &newvelocity) {
      float const angle_ratio = std::acos(thisear.orientation.dot(thissound.position - thisear.position) / distance) / static_cast<float>(M_PI);
  /// Update the velocity of the listener through the medium
      float const head_shadow_delay = head_shadow_delay_max * angle_ratio;
  listener_velocity = newvelocity;
      float apparent_seek __attribute__((__aligned__(16))) = thissound.seek - ((seek_delay + head_shadow_delay) * samplerate); // rewind to account for time delays
}
      if(apparent_seek >= 0.0f) {                                               // avoid trying to play before the start of the effect
void soundstorm::set_listener_position_and_rotation(vec3f const &newposition, quatf const &newrotation) {
        if(static_cast<size_t>(apparent_seek) >= thissound.effect->buffersize ||
  /// Update both position and orientation in a single call
           (thissound.seek_end != 0.0f && apparent_seek >= thissound.seek_end)) {
  listener_velocity = newposition - listener_position;
          // we've reached the end of this effect or our own seek limit
  listener_position = newposition;
          if(thissound.next_sound) {
  listener_rotation = newrotation;
            if(thissound.next_sound == &thissound) {
  update_ears();
              // this is a one-sound loop, so just rewind to the beginning
}
              thissound.seek = -thissound.seek_speed;                           // rewind one step make sure the first sample is really 0

              apparent_seek = thissound.seek - seek_delay;                      // rewind to account for time delay
void soundstorm::update_ears() {
              #ifdef SOUNDSTORM_NO_SSE
  /// Update the "ear" positions for each channel
                if(apparent_seek < 0.0f) {                                      // avoid trying to play before the start of the effect
  // Note: expects at least 2 ears at any given point!
                  apparent_seek = 0.0f;
  ears[channel_type::LEFT].position.assign(-ear_offset, 0.0, 0.0);
                }
              #else
  ears[channel_type::RIGHT].position.assign(ear_offset, 0.0, 0.0);
                _mm_store_ss(&apparent_seek, _mm_max_ss(_mm_set_ss(apparent_seek), _mm_set_ss(0.0f))); // SSE intrinsicts: branchless max
  for(auto &thisear : ears) {
              #endif // SOUNDSTORM_NO_SSE
    thisear.position.rotate(listener_rotation);
            } else {
    thisear.orientation = thisear.position;
              // we have something else queued up, so replace us with it and destroy the original
    thisear.orientation.normalise();
              sound *oldsound = *it;
    thisear.position += listener_position;
              *it = oldsound->next_sound;
  }
              delete oldsound;
}
            }

          } else {
float soundstorm::get_master_volume() const {
            #ifdef DEBUG_SOUNDSTORM
  return volume_master;
              std::cout << "SoundStorm: DEBUG: finished playing sound at seek point " << apparent_seek << " after " << apparent_seek / samplerate << "s" << std::endl;
}
            #endif // DEBUG_SOUNDSTORM
void soundstorm::set_master_volume(float newvolume) {
            sound *oldsound = *it;
  volume_master = newvolume;
            it = playing.erase(it);
}
            delete oldsound;

            continue;                                                           // we have nothing to contribute to the stream
soundstorm::soundeffect *soundstorm::get_effect(unsigned int effect_id) const {
          }
  /// Look up an effect in the library
        }
  #ifdef NSOUND
        // directional attenuation
    return nullptr;
        float const directional_attenuation = 1.0f - (head_shadow_attenuation * angle_ratio);
  #endif // NSOUND
        float const sample = (thissound.effect->buffer[static_cast<unsigned int>(apparent_seek)] *
  #ifndef NDEBUG
                              thissound.effect->hdr_scale *
    if(effect_id >= effect_library.size()) {
                              thissound.volume *
                              directional_attenuation) / distance_sq;
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with id " << effect_id << " outside library size " << effect_library.size() << "!" << std::endl;
        #ifdef DEBUG_SOUNDSTORM
      return nullptr;
          //std::cout << "DEBUG: " << thissound.channel << " ancos " << anglecos << " anPI " << angle / M_PI << " att " << directional_attenuation << " sam " << sample << std::endl;
    }
        #endif // DEBUG_SOUNDSTORM
    if(!effect_library[effect_id]) {
        out[thissound.channel][i] += sample;
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with id " << effect_id << " returned nullptr!" << std::endl;
        #ifdef SOUNDSTORM_NO_SSE
      return nullptr;
          float const absolute_level = std::abs(out[thissound.channel][i]);
    }
          if(max_level < absolute_level) {                                      // keep track of the maximum output level this frame
  #endif // NDEBUG
            max_level = absolute_level;
  return effect_library[effect_id];
          }
}
        #else

          //_mm_store_ss(&max_level, _mm_max_ss(_mm_set_ss(max_level), _mm_set_ss(std::abs(out[thissound.channel][i])))); // SSE intrinsicts: branchless max
soundstorm::music_buffer *soundstorm::get_music(unsigned int music_id) const {
          // SSE abs() implementation hack, see http://fastcpp.blogspot.co.uk/2011/03/changing-sign-of-float-values-using-sse.html
  /// Look up a music track in the library
          static __m128 const signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000)); // SSE intrinsic bitmask for float sign
  #ifdef NSOUND
          __m128 const absolute_level = _mm_andnot_ps(signmask, _mm_set_ss(out[thissound.channel][i])); // SSE intrinsics: and not
    return nullptr;
          _mm_store_ss(&max_level, _mm_max_ss(_mm_set_ss(max_level), absolute_level)); // SSE intrinsicts: branchless max
  #endif // NSOUND
        #endif // SOUNDSTORM_NO_SSE
  #ifndef NDEBUG
      }
    if(music_id >= music_library.size()) {
      thissound.seek += thissound.seek_speed;
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with music_id " << music_id << " exceeding library size!" << std::endl;
      ++it;                                                                     // only increment here in case we erase instead
      return nullptr;
    }
    }

  #endif // NDEBUG
    // play the music
  return music_library[music_id];
    for(deck &thisdeck : decks) {
}
      // perform any fades we need to

      #ifdef SOUNDSTORM_NO_SSE
unsigned int soundstorm::load(unsigned char const *buffer, size_t buffersize, float hdr_scale) {
        thisdeck.volume += std::min(std::max(thisdeck.volume_target - thisdeck.volume, thisdeck.volume_fadespeed), -thisdeck.volume_fadespeed);
      #else
  /// Load a sound from a buffer into the library, and return its new library id
        static __m128 const signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000)); // SSE intrinsic bitmask for float sign
  #ifdef NSOUND
        //__m128 volume_shift = _mm_sub_ss(_mm_set_ss(thisdeck.volume_target), _mm_set_ss(thisdeck.volume)); // SSE intrinsics: subtract
    return 0;
        __m128 const volume_target = _mm_set_ss(thisdeck.volume_target);
  #endif // NSOUND
        __m128 const volume        = _mm_set_ss(thisdeck.volume);
  unsigned int const effectnum = cast_if_required<unsigned int>(effect_library.size());
        __m128 volume_shift = _mm_sub_ss(volume_target, volume);                // SSE intrinsics: subtract
  soundeffect *thiseffect = new soundeffect;
        __m128 const fadespeed_max = _mm_set_ss(thisdeck.volume_fadespeed);
  thiseffect->buffer = reinterpret_cast<float const*>(buffer);                  // treat the buffer as one of 32bit floats
        __m128 const fadespeed_min = _mm_xor_ps(fadespeed_max, signmask);       // SSE intrinsics: xor (to flip the sign)
  thiseffect->buffersize = buffersize / sizeof(float);                          // convert to our size in samples
        volume_shift = _mm_min_ss(volume_shift, fadespeed_max);                 // SSE intrinsics: branchless min (clamp top)
  thiseffect->hdr_scale = hdr_scale;
        volume_shift = _mm_max_ss(volume_shift, fadespeed_min);                 // SSE intrinsics: branchless max (clamp bottom)
  effect_library.emplace_back(thiseffect);
        _mm_store_ss(&thisdeck.volume, _mm_add_ss(_mm_set_ss(thisdeck.volume), volume_shift)); // SSE intrinsics: add
  #ifdef DEBUG_SOUNDSTORM
      #endif // SOUNDSTORM_NO_SSE
    // do some analysis

    float min = 0.0;
      out[channel_type::LEFT ][i] += thisdeck.buffer_l[thisdeck.buffer_read][thisdeck.buffer_read_seek] * thisdeck.volume;
    float max = 0.0;
      out[channel_type::RIGHT][i] += thisdeck.buffer_r[thisdeck.buffer_read][thisdeck.buffer_read_seek] * thisdeck.volume;
    for(size_t i = 0; i != thiseffect->buffersize; ++i) {
      ++thisdeck.buffer_read_seek;
      if(thiseffect->buffer[i] < min) {
      if(thisdeck.buffer_read_seek == deck_buffer_size) {                       // we've reached the end of this buffer
        min = thiseffect->buffer[i];
        thisdeck.buffer_read = 1 - thisdeck.buffer_read;                        // flip the ping-pongs
      }
        thisdeck.buffer_needs_filled = true;                                    // flag it as needing refilled
      if(thiseffect->buffer[i] > max) {
        thisdeck.buffer_read_seek = 0;                                          // rewind
        max = thiseffect->buffer[i];
        #ifdef DEBUG_SOUNDSTORM
      }
          //std::cout << "SoundStorm: DEBUG: deck " << &thisdeck << " buffer flipped to " << thisdeck.buffer_read << std::endl;
    }
        #endif // DEBUG_SOUNDSTORM
      }
    std::cout << "SoundStorm: DEBUG: loaded effect " << effectnum << " from buffer, size " << buffersize << ", length " << static_cast<float>(buffersize) / samplerate << "s, min " << min << " max " << max << " (" << std::max(std::abs(min), max) * 100 << "% vol)" << std::endl;
    }
  #endif // DEBUG_SOUNDSTORM

  return effectnum;
    // scale the HDR windows for this frame
}
    #if defined(SOUNDSTORM_NO_SSE) || defined(DEBUG_SOUNDSTORM)                 // skip using branchless intrins if we need debugging output

      if(hdr_window_top < max_level) {
unsigned int soundstorm::music_load(unsigned char const *buffer, size_t buffersize) {
        hdr_window_top = max_level;
  /// Load a piece of music from a buffer into the librarym and return its new library id
        //hdr_window_bottom = std::min(0.0f, hdr_window_top - max_level);         // slide up the bottom of the window to match
  #ifdef NSOUND
        #ifdef DEBUG_SOUNDSTORM
    return 0;
          std::cout << "SoundStorm: DEBUG: HDR window raised to " << hdr_window_bottom << " - " << hdr_window_top << std::endl;
  #endif // NSOUND
          if(hdr_window_top > session_max_hdr_window_top) {
  unsigned int const tracknum = cast_if_required<unsigned int>(music_library.size());
            session_max_hdr_window_top = hdr_window_top;
  music_buffer *thismusic = new music_buffer;
            std::cout << "SoundStorm: DEBUG: HDR window top new session max: " << session_max_hdr_window_top << std::endl;
  thismusic->buffer = buffer;
          }
  thismusic->buffersize = buffersize;
          if(hdr_window_bottom > session_max_hdr_window_bottom) {
  music_library.emplace_back(thismusic);
            session_max_hdr_window_bottom = hdr_window_bottom;
  #ifdef DEBUG_SOUNDSTORM
            std::cout << "SoundStorm: DEBUG: HDR window bottom new session max: " << session_max_hdr_window_bottom << std::endl;
    std::cout << "SoundStorm: DEBUG: loaded music " << tracknum << " from buffer, size " << buffersize << std::endl;
          }
  #endif // DEBUG_SOUNDSTORM
        #endif // DEBUG_SOUNDSTORM
  return tracknum;
      }
}
    #else

      _mm_store_ss(&hdr_window_top, _mm_max_ss(_mm_set_ss(hdr_window_top), _mm_set_ss(max_level))); // SSE intrinsicts: branchless max
void soundstorm::play(unsigned int effect_id,
    #endif // SOUNDSTORM_NO_SSE || DEBUG_SOUNDSTORM
                      vec3f const &position,
    float const final_scale = volume_master / hdr_window_top;                   // final global volume control and HDR window scaling
                      vec3f const &velocity,
    for(unsigned int channel = 0; channel != channels; ++channel) {             // scale all channels
                      float volume,
      out[channel][i] *= final_scale;
                      float seek_start,
    }
  }
                      float seek_end,
  return paContinue;
                      float seek_speed,
}
                      soundgroup *thissoundgroup) {

  /// Add a sound effect by id to the currently playing list with the specified parameters
void soundstorm::streamer() {
  play(position, velocity, get_effect(effect_id), volume, seek_start, seek_end, seek_speed, thissoundgroup);
  /// Streaming decoder that checks and fills the deck buffers
}
  #ifdef DEBUG_SOUNDSTORM

    std::cout << "SoundStorm: DEBUG: streamer starting" << std::endl;
void soundstorm::play(vec3f const &position,
  #endif // DEBUG_SOUNDSTORM
                      vec3f const &velocity,
  ov_callbacks callbacks;
                      soundeffect *effect,
  callbacks.read_func  = &soundstorm::ogg_callback_read;
                      float volume,
  //callbacks.seek_func  = &soundstorm::ogg_callback_seek;                        // can be NULL to treat as non-seekable
                      float seek_start,
  callbacks.seek_func  = NULL;
                      float seek_end,
  //callbacks.close_func = &soundstorm::ogg_callback_close;                       // or can just be NULL for no close
                      float seek_speed,
  callbacks.close_func = NULL;
                      soundgroup *thissoundgroup) {
  //callbacks.tell_func  = &soundstorm::ogg_callback_tell;                        // can be NULL to treat as non-seekable
  callbacks.tell_func  = NULL;
  /// Add a sound effect to the currently playing list with the specified parameters

  #ifdef NSOUND
  do {
    return;
    for(auto &thisdeck : decks) {
  #endif // NSOUND
      if(!thisdeck.oggfile) {                                                   // initialise the per-deck ogg decoders
  if(!enabled) {
        if(thisdeck.playlist.empty()) {
    return;
          #ifdef DEBUG_SOUNDSTORM
  }
            std::cout << "SoundStorm: WARNING: deck " << &thisdeck << " given empty playlist" << std::endl;
  // parameters reordered to avoid call ambiguity
          #endif // DEBUG_SOUNDSTORM
  #ifndef NDEBUG
          continue;
    if(!effect) {                                                               // null check only in debug mode
        }
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with null effect!" << std::endl;
        thisdeck.oggfile = new OggVorbis_File;
      return;
        int result = ov_open_callbacks(&thisdeck, thisdeck.oggfile, NULL, 0, callbacks);
    }
        if(result != 0) {
  #endif // NDEBUG
          switch(result) {
          case OV_EREAD:
  for(unsigned int channelnum = 0; channelnum != channels; ++channelnum) {
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks A read from media returned an error: " << result << std::endl;
    sound *thissound = new sound(effect, position, velocity, volume, seek_start, seek_end, seek_speed, nullptr, channelnum);
            break;
    playing.emplace_back(thissound);
          case OV_ENOTVORBIS:
    if(thissoundgroup) {
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Bitstream does not contain any Vorbis data: " << result << std::endl;
      thissoundgroup->emplace_back(thissound);
            break;
    }
          case OV_EVERSION:
  }
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Vorbis version mismatch: " << result << std::endl;
  #ifdef DEBUG_SOUNDSTORM
            break;
    std::cout << "SoundStorm: DEBUG: playing sound at " << position << " volume " << volume << ", " << playing.size() << " sounds total" << std::endl;
          case OV_EBADHEADER:
  #endif // DEBUG_SOUNDSTORM
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Invalid Vorbis bitstream header: " << result << std::endl;
}
            break;

          case OV_EFAULT:
void soundstorm::play_loop(unsigned int effect_id,
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Internal logic fault; indicates a bug or heap/stack corruption: " << result << std::endl;
                           vec3f const &position,
            break;
                           vec3f const &velocity,
          default:
                           float volume,
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: unknown error " << result << std::endl;
                           float seek_start,
            break;
                           float seek_end,
          }
                           float seek_speed,
          return;
        }
                           soundgroup *thissoundgroup) {
        #ifdef DEBUG_SOUNDSTORM
  /// Add a sound effect by id set to repeat indefinitely to the currently playing list with the specified parameters
          char **comment = ov_comment(thisdeck.oggfile, -1)->user_comments;
  return play_loop(position, velocity, get_effect(effect_id), volume, seek_start, seek_end, seek_speed, thissoundgroup);
          while(*comment) {
}
            std::cout << "SoundStorm: deck " << &thisdeck << " Ogg comment: " << comment <<std::endl;

            ++comment;
void soundstorm::play_loop(vec3f const &position,
          }
                           vec3f const &velocity,
          vorbis_info *info = ov_info(thisdeck.oggfile, -1);
                           soundeffect *effect,
          std::cout << "SoundStorm: deck " << &thisdeck << " Bitstream is " << info->channels << " channel, " << info->rate << "Hz" << std::endl;
                           float volume,
          std::cout << "SoundStorm: deck " << &thisdeck << " Decoded length: " << ov_pcm_total(thisdeck.oggfile, -1) << " samples" << std::endl;
                           float seek_start,
          std::cout << "SoundStorm: deck " << &thisdeck << " Encoded by: " << ov_comment(thisdeck.oggfile, -1)->vendor << std::endl;
                           float seek_end,
        #endif // DEBUG_SOUNDSTORM
                           float seek_speed,
      }
                           soundgroup *thissoundgroup) {
      if(thisdeck.buffer_needs_filled) {
  /// Add a sound effect set to repeat indefinitely to the currently playing list with the specified parameters
        thisdeck.buffer_needs_filled = false;                                   // reset the flag first
  #ifdef NSOUND
        unsigned int const buffer_write = 1 - thisdeck.buffer_read;
    return;
        #ifdef DEBUG_SOUNDSTORM
  #endif // NSOUND
          //std::cout << "SoundStorm: DEBUG: deck " << &thisdeck << " buffer " << buffer_write << " refilling..." << std::endl;
  // parameters reordered to avoid call ambiguity
        #endif // DEBUG_SOUNDSTORM
  if(!enabled) {
        for(unsigned int i = 0; i != deck_buffer_size;) {
    return;
          int current_section;                                                  // what the hell is this even used for?
  }
          float **pcm_channels;
  #ifndef NDEBUG
          int samples_read;
          do {
    if(!effect) {                                                               // null check only in debug mode
            samples_read = static_cast<int>(ov_read_float(thisdeck.oggfile, &pcm_channels, deck_buffer_size - i, &current_section));
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with null effect!" << std::endl;
            switch(samples_read) {
      return;
            case 0:                                                             // EOF
    }
              //std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill got EOF " << samples_read << std::endl;
  #endif // NDEBUG
              break;
  bool nullsoundgroup;
            case OV_HOLE:                                                       // indicates there was an interruption in the data. (one of: garbage between pages, loss of sync followed by recapture, or a corrupt page)
  if(thissoundgroup) {
              #ifdef DEBUG_SOUNDSTORM
    nullsoundgroup = false;
                //std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill failed with OV_HOLE " << samples_read << std::endl;
  } else {
              #endif // DEBUG_SOUNDSTORM
    nullsoundgroup = true;
              break;                                                            // this is normal when switching tracks
    thissoundgroup = new soundgroup;
  }
            case OV_EBADLINK:                                                   // indicates that an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.
  play(position, velocity, effect, volume, seek_start, seek_end, seek_speed, thissoundgroup);
              std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill failed with OV_EBADLINK " << samples_read << std::endl;
              break;
  // make each their own successor, looping with the same start and end
            case OV_EINVAL:                                                     // indicates the initial file headers couldn't be read or are corrupt, or that the initial open call for vf failed.
  for(auto const &thissound : *thissoundgroup) {
              std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill failed with OV_EINVAL " << samples_read << std::endl;
    thissound->next_sound = thissound;
              break;
  }
            }
  if(nullsoundgroup) {
          } while(samples_read <= 0);
    delete thissoundgroup;
          for(int s = 0; s != samples_read; ++s) {
    //thissoundgroup = nullptr;
            thisdeck.buffer_l[buffer_write][i + s] = pcm_channels[LEFT ][s];
  }
            thisdeck.buffer_r[buffer_write][i + s] = pcm_channels[RIGHT][s];
}
          }

          #ifdef DEBUG_SOUNDSTORM
soundstorm::music *soundstorm::music_queue(unsigned int deck_id, unsigned int music_id) {
            std::cout << "SoundStorm: DEBUG: streamer read " << samples_read << " bytes of " << deck_buffer_size - i << ", current_section " << current_section << std::endl;
  /// Queue an item from the music library to play next on the specified deck
          #endif // DEBUG_SOUNDSTORM
  #ifdef NSOUND
          i += static_cast<unsigned int>(samples_read);
    return nullptr;
        }
  #endif // NSOUND
        #ifdef DEBUG_SOUNDSTORM
  if(!enabled) {
          //std::cout << "SoundStorm: DEBUG: deck " << &thisdeck << " buffer " << buffer_write << " refilled" << std::endl;
    return nullptr;
        #endif // DEBUG_SOUNDSTORM
  }
      }
  #ifndef NDEBUG
    }
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
    float constexpr const buffer_fill_sleep = 1.0f / 4.0f;                      // sleep for this fraction of buffer fill time to avoid spin-waiting
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned int>(1000.0f * static_cast<float>(deck_buffer_size) / samplerate * 0.5f * buffer_fill_sleep)));
      return nullptr;
  } while(streamer_run);

    }
  for(auto &thisdeck : decks) {                                                 // cleanup
  #endif // NDEBUG
    ov_clear(thisdeck.oggfile);
  music *thismusic = new music;
    delete thisdeck.oggfile;
  thismusic->parent = this;
    thisdeck.oggfile = nullptr;
  thismusic->parent_deck = &decks[deck_id];
  }
  thismusic->buffer = get_music(music_id);
  #ifdef DEBUG_SOUNDSTORM
  decks[deck_id].playlist.emplace(thismusic);
    std::cout << "SoundStorm: DEBUG: streamer finished cleanly" << std::endl;
  return thismusic;
  #endif // DEBUG_SOUNDSTORM
}
}


void soundstorm::set_music_volume(unsigned int deck_id, float newvolume) {
size_t soundstorm::ogg_callback_read(void *ptr, size_t size, size_t count, void *datasource) {
  /// Instantly apply a new volume level to the specified deck
  /// The interface is identical to that of fread, and identical behaviour is expected
  #ifndef NDEBUG
  #ifdef DEBUG_SOUNDSTORM
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
    //std::cout << "SoundStorm: DEBUG: streamer read requested size " << size << " count " << count << std::endl;
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
  #endif // DEBUG_SOUNDSTORM
      return;
  if(!datasource) {
    }
    #ifdef DEBUG_SOUNDSTORM
  #endif // NDEBUG
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
  decks[deck_id].volume = newvolume;
    #endif // DEBUG_SOUNDSTORM
  decks[deck_id].volume_target = newvolume;
    return 0;                                                                   // nullptr means we've got nothing playing
  decks[deck_id].volume_fadespeed = 0.0;
  }
}
  deck *thisdeck = reinterpret_cast<deck*>(datasource);

  #ifdef DEBUG_SOUNDSTORM
    if(thisdeck->playlist.empty()) {
void soundstorm::fade_music_volume(unsigned int deck_id, float newvolume, float seconds_to_take) {
      std::cout << "SoundStorm: DEBUG: streamer read called on deck with empty playlist, this should be checked for in advance." << std::endl;
  /// Slowly fade volume from current setting to new setting, over a given number of seconds
      return 0;
  #ifndef NDEBUG
    }
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
  #endif // DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
  music *thismusic = thisdeck->playlist.front();
      return;
  unsigned char *target = reinterpret_cast<unsigned char*>(ptr);
    }
  #ifdef DEBUG_SOUNDSTORM
  #endif // NDEBUG
    if(thisdeck->checkvalue != 123456) {
  decks[deck_id].volume_fadespeed = std::abs(newvolume - decks[deck_id].volume) / (seconds_to_take * samplerate); // this comes first since we're threaded
      std::cout << "SoundStorm: DEBUG: streamer read: check value incorrect: " << thisdeck->checkvalue << std::endl;
  decks[deck_id].volume_target = newvolume;
      return 0;
}
    }

    if(!thismusic->buffer) {
void soundstorm::crossfade_music(float seconds_to_take, unsigned int deck_from, unsigned int deck_to) {
      std::cout << "SoundStorm: DEBUG: no library music assigned to this playlist entry - this should never happen!" << std::endl;
      return 0;
  /// Convenience wrapper function to crossfade between a pair of decks
    }
  float const volume0 = decks[deck_from].volume_target;
    if(!thismusic->buffer->buffer) {
  float const volume1 = decks[deck_to  ].volume_target;
      std::cout << "SoundStorm: DEBUG: no buffer assigned to this music - this should never happen!" << std::endl;
  fade_music_volume(deck_from, volume1, seconds_to_take);
      return 0;
  fade_music_volume(deck_to,   volume0, seconds_to_take);
    }
}
  #endif // DEBUG_SOUNDSTORM


void soundstorm::stop(soundgroup const &thissoundgroup) {
  unsigned int const bytes = cast_if_required<unsigned int>(size * count);
  /// Make this sound stop immediately
  unsigned int i = 0;
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
  for(; i != bytes; ++i) {
    thissound->seek = static_cast<float>(thissound->effect->buffersize);
    target[i] = thismusic->buffer->buffer[thismusic->seek];
  }
    //memcpy(ptr, thismusic->buffer->buffer, size * count);
}
    ++thismusic->seek;

    if(thismusic->seek == thismusic->buffer->buffersize) {
void soundstorm::stop_loop(soundgroup const &thissoundgroup) {
      // advance the playlist
      #ifdef DEBUG_SOUNDSTORM
  /// Tell this sound not to loop or continue to the next sound
        std::cout << "SoundStorm: DEBUG: advancing playlist after " << thismusic->seek / 1024 << "KB played" << std::endl;
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
      #endif // DEBUG_SOUNDSTORM
    thissound->next_sound = nullptr;
      thismusic->seek = 0;                                                      // rewind so that we advance to 0 this frame
  }
      if(thisdeck->playlist.size() == 1) {
}
        // no track queued after this

        if(!thisdeck->repeat) {
void soundstorm::replace(soundgroup const &thissoundgroup, soundeffect *neweffect, float seek_start, float seek_end, float seek_speed) {
          delete thismusic;
  /// Immediately replace the currently playing sound with a new effect with the specified parameters
          thismusic = nullptr;
  #ifndef NDEBUG
          #ifdef DEBUG_SOUNDSTORM
    if(!neweffect) {                                                            // null check only in debug mode
            std::cout << "SoundStorm: DEBUG: streamer finished playlist, not repeating after " << i << " bytes" << std::endl;
          #endif // DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with null effect!" << std::endl;
          // create a short read
      return;
          return i;
    }
        }
  #endif // NDEBUG
        #ifdef DEBUG_SOUNDSTORM
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
          std::cout << "SoundStorm: DEBUG: streamer finished playlist, repeating" << std::endl;
    thissound->effect = neweffect;
        #endif // DEBUG_SOUNDSTORM
    thissound->seek = seek_start;
        // otherwise we just play this track from the start
    thissound->seek_end = seek_end;
      } else {
    thissound->seek_speed = seek_speed;
        #ifdef DEBUG_SOUNDSTORM
  }
          std::cout << "SoundStorm: DEBUG: streamer replacing music with top of playlist" << std::endl;
}
        #endif // DEBUG_SOUNDSTORM

        delete thismusic;
void soundstorm::follow(soundgroup const &thissoundgroup, soundeffect *neweffect, float seek_start, float seek_end, float seek_speed) {
        thisdeck->playlist.pop();                                               // take the current entry off the playlist
  /// Append another sound to play immediately once this one completes, with the specified parameters
        thismusic = thisdeck->playlist.front();                                 // tell the track to replace itself
      }
  /// (or specify nullptr to cancel a following sound)
    }
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
    // NOTE: could probably improve this using a block memcpy or std::copy
    sound *newsound = new sound(neweffect, thissound->position, thissound->velocity, thissound->volume, seek_start, seek_end, seek_speed, nullptr, thissound->channel);
  }
    thissound->next_sound = newsound;
  #ifdef DEBUG_SOUNDSTORM
  }
    //std::cout << "SoundStorm: DEBUG: streamer read completed with " << i << " bytes" << std::endl;
}
  #endif // DEBUG_SOUNDSTORM

  return i;
void soundstorm::set_volume(soundgroup const &thissoundgroup, float newvolume) {
}
  /// Adjust the volume of this sound

  for(auto const &thissound : thissoundgroup) {
int soundstorm::ogg_callback_seek(void *datasource, ogg_int64_t offset, int origin) {
    thissound->volume = newvolume;
  /// The interface is identical to that of fseek, and identical behaviour is expected
  }
  if(!datasource) {
}
    #ifdef DEBUG_SOUNDSTORM

      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
    #endif // DEBUG_SOUNDSTORM
void soundstorm::set_position(soundgroup const &thissoundgroup, vec3f const &newposition) {
    return 0;                                                                   // nullptr means we've got nothing playing
  /// Adjust the position of this sound
  }
  for(auto const &thissound : thissoundgroup) {
  deck *thisdeck = reinterpret_cast<deck*>(datasource);
    thissound->position = newposition;
  music *thismusic = thisdeck->playlist.front();
  }

}
  switch(origin) {

  case SEEK_SET:
void soundstorm::set_seek_speed(soundgroup const &thissoundgroup, float newspeed) {
    thismusic->seek = offset;
  /// Adjust the playback speed of this sound
    break;
  for(auto const &thissound : thissoundgroup) {
  case SEEK_CUR:
    thissound->seek_speed = newspeed;
    thismusic->seek += offset;
  }
    break;
}
  case SEEK_END:

    #ifdef DEBUG_SOUNDSTORM
      if(!thismusic->buffer) {
void soundstorm::music_clear(unsigned int deck_id) {
        std::cout << "SoundStorm: DEBUG: no library music assigned to this playlist entry passed as datasource to ogg_callback_seek - this should never happen!" << std::endl;
  /// Clear the playlist on the specified deck
        return -1;
  #ifndef NDEBUG
      }
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
    #endif // DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
    thismusic->seek = thismusic->buffer->buffersize + offset;
      return;
    break;
    }
  default:
  #endif // NDEBUG
    return -1;
  while(!decks[deck_id].playlist.empty()) {
  }
    delete decks[deck_id].playlist.front();
  return 0;
}
    decks[deck_id].playlist.pop();

    //decks[deck_id].buffer_read_seek = deck_buffer_size;                         // wind it to the end of the current buffer
int soundstorm::ogg_callback_close(void *datasource __attribute__((__unused__))) {
    //decks[deck_id].buffer_needs_filled = true;
  /// The interface is identical to that of fclose, and identical behaviour is expected
  }
  // we do absolutely nothing
}
  return 0;
}

long soundstorm::ogg_callback_tell(void *datasource) {
  /// The interface is identical to that of ftell, and identical behaviour is expected
  if(!datasource) {
    #ifdef DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
    #endif // DEBUG_SOUNDSTORM
    return 0;                                                                   // nullptr means we've got nothing playing
  }
  deck *thisdeck = reinterpret_cast<deck*>(datasource);
  return cast_if_required<long>(thisdeck->playlist.front()->seek);
}

unsigned int soundstorm::get_device_default() const {
  /// Return the number of the default audio device
  return audio_system->defaultOutputDevice().index();
}

unsigned int soundstorm::get_device_current() const {
  /// Return the number of the currently selected audio device
  return audio_device->index();
}

void soundstorm::get_device_list(std::vector<std::pair<unsigned int, std::string>> &target_list) const {
  /// Populate a vector with a set of strings describing devices, intended to let users choose from a menu
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    std::stringstream ss;
    //ss << it.index() << " " << it.name();
    ss << it.name();
    ss << " Ch " << it.maxInputChannels() << "in " << it.maxOutputChannels() << "out " << it.defaultSampleRate() << "Hz";
    //ss << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
    if(it.isSystemDefaultInputDevice()) {
      ss << " [sysdef in]";
    }
    if(it.isSystemDefaultOutputDevice()) {
      ss << " [sysdef out]";
    }
    if(it.isHostApiDefaultInputDevice()) {
      ss << " [apidef in]";
    }
    if(it.isHostApiDefaultOutputDevice()) {
      ss << " [apidef out]";
    }
    if(it.isInputOnlyDevice()) {
      ss << " [in only]";
    }
    if(it.isOutputOnlyDevice()) {
      ss << " [out only]";
    }
    if(it.isFullDuplexDevice()) {
      ss << " [fd]";
    }
    target_list.emplace_back(it.index(), ss.str());
  }
}
void soundstorm::get_device_list_out_only(std::vector<std::pair<unsigned int, std::string>> &target_list) const {
  /// Populate a vector with a set of strings describing devices, listing output devices only
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    if(it.isInputOnlyDevice() || it.maxOutputChannels() == 0) {
      continue;
    }
    std::stringstream ss;
    //ss << it.index() << " " << it.name();
    ss << it.name();
    ss << " Ch " << it.maxOutputChannels() << "out " << it.defaultSampleRate() << "Hz";
    //ss << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
    if(it.isSystemDefaultOutputDevice()) {
      ss << " [sysdef out]";
    }
    if(it.isHostApiDefaultOutputDevice()) {
      ss << " [apidef out]";
    }
    target_list.emplace_back(it.index(), ss.str());
  }
}
void soundstorm::get_device_list_in_only(std::vector<std::pair<unsigned int, std::string>> &target_list) const {
  /// Populate a vector with a set of strings describing devices, listing input devices only
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    if(it.isOutputOnlyDevice() || it.maxInputChannels() == 0) {
      continue;
    }
    std::stringstream ss;
    //ss << it.index() << " " << it.name();
    ss << it.name();
    ss << " Ch " << it.maxInputChannels() << "in " << it.defaultSampleRate() << "Hz";
    //ss << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
    if(it.isSystemDefaultInputDevice()) {
      ss << " [sysdef in]";
    }
    if(it.isHostApiDefaultInputDevice()) {
      ss << " [apidef in]";
    }
    target_list.emplace_back(it.index(), ss.str());
  }
}

void soundstorm::set_device(unsigned int new_device_index) {
  /// Switch to a different output device
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    if(it.index() == static_cast<int>(new_device_index)) {
      if(audio_device == &it) {
        std::cout << "SoundStorm: Switching to already selected device " << new_device_index << std::endl;
      } else {
        std::cout << "SoundStorm: Switching to device " << new_device_index << ": " << it.name() << std::endl;
        audio_device = &it;
        restart_device();
      }
      return;
    }
  }
  std::cout << "SoundStorm: Tried to switch to nonexistent device number " << new_device_index << std::endl;
}

unsigned int soundstorm::get_num_decks() const {
  return num_decks;
}
void soundstorm::set_num_decks(unsigned int new_num_decks) {
  /// Change the number of decks
  if(new_num_decks == num_decks) {
    return;                                                                     // exit silently if the change is a noop
  }
  std::cout << "SoundStorm: Changing number of decks from " << num_decks << " to " << new_num_decks << std::endl;
  bool const reinitialise_streamer = streamer_run;
  if(reinitialise_streamer) {                                                   // if the streamer's currently running, shut it down
    stop_streamer();
  }
  shutdown_device();
  num_decks = new_num_decks;
  init_device();
  if(reinitialise_streamer) {                                                   // if the streamer was running at restart, restart it
    start_streamer();
  }
}

double soundstorm::get_cpu_usage() const {
  /// Value normally from 0 to 1, but may exceed 1
  return stream->cpuLoad();
}

double soundstorm::get_sample_rate() const {
  /// Get measured sample rate of this stream so far
  return stream->sampleRate();
}

double soundstorm::get_time() const {
  /// Get the time spent playing the stream up to this point
  return stream->time();
}

void soundstorm::dump_stats() const {
  /// Output some measured statistics about the stream
  std::cout << "SoundStorm: Sample rate " << get_sample_rate() << "Hz, CPU " << get_cpu_usage() * 100 << "%, time " << get_time() << std::endl;
}
void soundstorm::dump_session_report() const {
  /// Output debugging statistics collected during the session
  #ifdef DEBUG_SOUNDSTORM
    std::cout << "SoundStorm session report: HDR window max: " << session_max_hdr_window_bottom << " - " << session_max_hdr_window_top << std::endl;
    std::cout << "SoundStorm session report: max simultaneous sounds " << session_max_simultaneous_sounds << std::endl;
    std::cout << "SoundStorm session report: source distance min " << session_min_distance << " max " << session_max_distance << std::endl;
  #endif // DEBUG_SOUNDSTORM
}
void soundstorm::dump_device_info() {
  /// Output info about the devices available and the currently selected device, and update device count
  num_devices = 0;
  for(auto const &it : boost::make_iterator_range(audio_system->devicesBegin(), audio_system->devicesEnd())) {
    ++num_devices;
    if(&it == audio_device) {                                                   // mark the currently selected device
      std::cout << " *";
    } else {
      std::cout << "  ";
    }
    std::cout << it.index() << " " << it.name();
    std::cout << " Ch " << it.maxInputChannels() << "in " << it.maxOutputChannels() << "out " << it.defaultSampleRate() << "Hz";
    //std::cout << " latency in/out " << it.defaultLowInputLatency() << "/" << it.defaultLowOutputLatency() << " to " << it.defaultHighInputLatency() << "/" << it.defaultHighOutputLatency() << " out" << std::endl;
    if(it.isSystemDefaultInputDevice()) {
      std::cout << " [sysdef in]";
    }
    if(it.isSystemDefaultOutputDevice()) {
      std::cout << " [sysdef out]";
    }
    if(it.isHostApiDefaultInputDevice()) {
      std::cout << " [apidef in]";
    }
    if(it.isHostApiDefaultOutputDevice()) {
      std::cout << " [apidef out]";
    }
    if(it.isInputOnlyDevice()) {
      std::cout << " [in only]";
    }
    if(it.isOutputOnlyDevice()) {
      std::cout << " [out only]";
    }
    if(it.isFullDuplexDevice()) {
      std::cout << " [fd]";
    }
    std::cout << std::endl;
  }
}

vec3f const &soundstorm::get_listener_position() const {
  /// Return the current listener world position
  return listener_position;
}
quatf const &soundstorm::get_listener_rotation() const {
  /// Return the current listener rotation
  return listener_rotation;
}
vec3f const &soundstorm::get_listener_velocity() const {
  /// Return the current listener world position
  return listener_velocity;
}

void soundstorm::set_listener_position(vec3f const &newposition) {
  /// Update the world position of the listener - also updates the velocity
  listener_velocity = newposition - listener_position;
  listener_position = newposition;
  update_ears();
}
void soundstorm::set_listener_rotation(quatf const &newrotation) {
  /// Update the facing direction of the listener
  listener_rotation = newrotation;
  update_ears();
}
void soundstorm::set_listener_velocity(vec3f const &newvelocity) {
  /// Update the velocity of the listener through the medium
  listener_velocity = newvelocity;
}
void soundstorm::set_listener_position_and_rotation(vec3f const &newposition, quatf const &newrotation) {
  /// Update both position and orientation in a single call
  listener_velocity = newposition - listener_position;
  listener_position = newposition;
  listener_rotation = newrotation;
  update_ears();
}

void soundstorm::update_ears() {
  /// Update the "ear" positions for each channel
  // Note: expects at least 2 ears at any given point!
  ears[channel_type::LEFT].position.assign(-ear_offset, 0.0, 0.0);
  ears[channel_type::RIGHT].position.assign(ear_offset, 0.0, 0.0);
  for(auto &thisear : ears) {
    thisear.position.rotate(listener_rotation);
    thisear.orientation = thisear.position;
    thisear.orientation.normalise();
    thisear.position += listener_position;
  }
}

float soundstorm::get_master_volume() const {
  return volume_master;
}
void soundstorm::set_master_volume(float newvolume) {
  volume_master = newvolume;
}

soundstorm::soundeffect *soundstorm::get_effect(unsigned int effect_id) const {
  /// Look up an effect in the library
  #ifdef NSOUND
    return nullptr;
  #endif // NSOUND
  #ifndef NDEBUG
    if(effect_id >= effect_library.size()) {
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with id " << effect_id << " outside library size " << effect_library.size() << "!" << std::endl;
      return nullptr;
    }
    if(!effect_library[effect_id]) {
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with id " << effect_id << " returned nullptr!" << std::endl;
      return nullptr;
    }
  #endif // NDEBUG
  return effect_library[effect_id];
}

soundstorm::music_buffer *soundstorm::get_music(unsigned int music_id) const {
  /// Look up a music track in the library
  #ifdef NSOUND
    return nullptr;
  #endif // NSOUND
  #ifndef NDEBUG
    if(music_id >= music_library.size()) {
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with music_id " << music_id << " exceeding library size!" << std::endl;
      return nullptr;
    }
  #endif // NDEBUG
  return music_library[music_id];
}

unsigned int soundstorm::load(unsigned char const *buffer, size_t buffersize, float hdr_scale) {
  /// Load a sound from a buffer into the library, and return its new library id
  #ifdef NSOUND
    return 0;
  #endif // NSOUND
  unsigned int const effectnum = cast_if_required<unsigned int>(effect_library.size());
  soundeffect *thiseffect = new soundeffect;
  thiseffect->buffer = reinterpret_cast<float const*>(buffer);                  // treat the buffer as one of 32bit floats
  thiseffect->buffersize = buffersize / sizeof(float);                          // convert to our size in samples
  thiseffect->hdr_scale = hdr_scale;
  effect_library.emplace_back(thiseffect);
  #ifdef DEBUG_SOUNDSTORM
    // do some analysis
    float min = 0.0;
    float max = 0.0;
    for(size_t i = 0; i != thiseffect->buffersize; ++i) {
      if(thiseffect->buffer[i] < min) {
        min = thiseffect->buffer[i];
      }
      if(thiseffect->buffer[i] > max) {
        max = thiseffect->buffer[i];
      }
    }
    std::cout << "SoundStorm: DEBUG: loaded effect " << effectnum << " from buffer, size " << buffersize << ", length " << static_cast<float>(buffersize) / samplerate << "s, min " << min << " max " << max << " (" << std::max(std::abs(min), max) * 100 << "% vol)" << std::endl;
  #endif // DEBUG_SOUNDSTORM
  return effectnum;
}

unsigned int soundstorm::music_load(unsigned char const *buffer, size_t buffersize) {
  /// Load a piece of music from a buffer into the librarym and return its new library id
  #ifdef NSOUND
    return 0;
  #endif // NSOUND
  unsigned int const tracknum = cast_if_required<unsigned int>(music_library.size());
  music_buffer *thismusic = new music_buffer;
  thismusic->buffer = buffer;
  thismusic->buffersize = buffersize;
  music_library.emplace_back(thismusic);
  #ifdef DEBUG_SOUNDSTORM
    std::cout << "SoundStorm: DEBUG: loaded music " << tracknum << " from buffer, size " << buffersize << std::endl;
  #endif // DEBUG_SOUNDSTORM
  return tracknum;
}

void soundstorm::play(unsigned int effect_id,
                      vec3f const &position,
                      vec3f const &velocity,
                      float volume,
                      float seek_start,
                      float seek_end,
                      float seek_speed,
                      soundgroup *thissoundgroup) {
  /// Add a sound effect by id to the currently playing list with the specified parameters
  play(position, velocity, get_effect(effect_id), volume, seek_start, seek_end, seek_speed, thissoundgroup);
}

void soundstorm::play(vec3f const &position,
                      vec3f const &velocity,
                      soundeffect *effect,
                      float volume,
                      float seek_start,
                      float seek_end,
                      float seek_speed,
                      soundgroup *thissoundgroup) {
  /// Add a sound effect to the currently playing list with the specified parameters
  #ifdef NSOUND
    return;
  #endif // NSOUND
  if(!enabled) {
    return;
  }
  // parameters reordered to avoid call ambiguity
  #ifndef NDEBUG
    if(!effect) {                                                               // null check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with null effect!" << std::endl;
      return;
    }
  #endif // NDEBUG
  for(unsigned int channelnum = 0; channelnum != channels; ++channelnum) {
    sound *thissound = new sound(effect, position, velocity, volume, seek_start, seek_end, seek_speed, nullptr, channelnum);
    playing.emplace_back(thissound);
    if(thissoundgroup) {
      thissoundgroup->emplace_back(thissound);
    }
  }
  #ifdef DEBUG_SOUNDSTORM
    std::cout << "SoundStorm: DEBUG: playing sound at " << position << " volume " << volume << ", " << playing.size() << " sounds total" << std::endl;
  #endif // DEBUG_SOUNDSTORM
}

void soundstorm::play_loop(unsigned int effect_id,
                           vec3f const &position,
                           vec3f const &velocity,
                           float volume,
                           float seek_start,
                           float seek_end,
                           float seek_speed,
                           soundgroup *thissoundgroup) {
  /// Add a sound effect by id set to repeat indefinitely to the currently playing list with the specified parameters
  return play_loop(position, velocity, get_effect(effect_id), volume, seek_start, seek_end, seek_speed, thissoundgroup);
}

void soundstorm::play_loop(vec3f const &position,
                           vec3f const &velocity,
                           soundeffect *effect,
                           float volume,
                           float seek_start,
                           float seek_end,
                           float seek_speed,
                           soundgroup *thissoundgroup) {
  /// Add a sound effect set to repeat indefinitely to the currently playing list with the specified parameters
  #ifdef NSOUND
    return;
  #endif // NSOUND
  // parameters reordered to avoid call ambiguity
  if(!enabled) {
    return;
  }
  #ifndef NDEBUG
    if(!effect) {                                                               // null check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with null effect!" << std::endl;
      return;
    }
  #endif // NDEBUG
  bool nullsoundgroup;
  if(thissoundgroup) {
    nullsoundgroup = false;
  } else {
    nullsoundgroup = true;
    thissoundgroup = new soundgroup;
  }
  play(position, velocity, effect, volume, seek_start, seek_end, seek_speed, thissoundgroup);
  // make each their own successor, looping with the same start and end
  for(auto const &thissound : *thissoundgroup) {
    thissound->next_sound = thissound;
  }
  if(nullsoundgroup) {
    delete thissoundgroup;
    //thissoundgroup = nullptr;
  }
}

soundstorm::music *soundstorm::music_queue(unsigned int deck_id, unsigned int music_id) {
  /// Queue an item from the music library to play next on the specified deck
  #ifdef NSOUND
    return nullptr;
  #endif // NSOUND
  if(!enabled) {
    return nullptr;
  }
  #ifndef NDEBUG
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
      return nullptr;
    }
  #endif // NDEBUG
  music *thismusic = new music;
  thismusic->parent = this;
  thismusic->parent_deck = &decks[deck_id];
  thismusic->buffer = get_music(music_id);
  decks[deck_id].playlist.emplace(thismusic);
  return thismusic;
}

void soundstorm::set_music_volume(unsigned int deck_id, float newvolume) {
  /// Instantly apply a new volume level to the specified deck
  #ifndef NDEBUG
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
      return;
    }
  #endif // NDEBUG
  decks[deck_id].volume = newvolume;
  decks[deck_id].volume_target = newvolume;
  decks[deck_id].volume_fadespeed = 0.0;
}

void soundstorm::fade_music_volume(unsigned int deck_id, float newvolume, float seconds_to_take) {
  /// Slowly fade volume from current setting to new setting, over a given number of seconds
  #ifndef NDEBUG
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
      return;
    }
  #endif // NDEBUG
  decks[deck_id].volume_fadespeed = std::abs(newvolume - decks[deck_id].volume) / (seconds_to_take * samplerate); // this comes first since we're threaded
  decks[deck_id].volume_target = newvolume;
}

void soundstorm::crossfade_music(float seconds_to_take, unsigned int deck_from, unsigned int deck_to) {
  /// Convenience wrapper function to crossfade between a pair of decks
  float const volume0 = decks[deck_from].volume_target;
  float const volume1 = decks[deck_to  ].volume_target;
  fade_music_volume(deck_from, volume1, seconds_to_take);
  fade_music_volume(deck_to,   volume0, seconds_to_take);
}

void soundstorm::stop(soundgroup const &thissoundgroup) {
  /// Make this sound stop immediately
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
    thissound->seek = static_cast<float>(thissound->effect->buffersize);
  }
}

void soundstorm::stop_loop(soundgroup const &thissoundgroup) {
  /// Tell this sound not to loop or continue to the next sound
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
    thissound->next_sound = nullptr;
  }
}

void soundstorm::replace(soundgroup const &thissoundgroup, soundeffect *neweffect, float seek_start, float seek_end, float seek_speed) {
  /// Immediately replace the currently playing sound with a new effect with the specified parameters
  #ifndef NDEBUG
    if(!neweffect) {                                                            // null check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with null effect!" << std::endl;
      return;
    }
  #endif // NDEBUG
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
    thissound->effect = neweffect;
    thissound->seek = seek_start;
    thissound->seek_end = seek_end;
    thissound->seek_speed = seek_speed;
  }
}

void soundstorm::follow(soundgroup const &thissoundgroup, soundeffect *neweffect, float seek_start, float seek_end, float seek_speed) {
  /// Append another sound to play immediately once this one completes, with the specified parameters
  /// (or specify nullptr to cancel a following sound)
  for(auto const &thissound : thissoundgroup) {                                 // do this for each channel
    sound *newsound = new sound(neweffect, thissound->position, thissound->velocity, thissound->volume, seek_start, seek_end, seek_speed, nullptr, thissound->channel);
    thissound->next_sound = newsound;
  }
}

void soundstorm::set_volume(soundgroup const &thissoundgroup, float newvolume) {
  /// Adjust the volume of this sound
  for(auto const &thissound : thissoundgroup) {
    thissound->volume = newvolume;
  }
}

void soundstorm::set_position(soundgroup const &thissoundgroup, vec3f const &newposition) {
  /// Adjust the position of this sound
  for(auto const &thissound : thissoundgroup) {
    thissound->position = newposition;
  }
}

void soundstorm::set_seek_speed(soundgroup const &thissoundgroup, float newspeed) {
  /// Adjust the playback speed of this sound
  for(auto const &thissound : thissoundgroup) {
    thissound->seek_speed = newspeed;
  }
}

void soundstorm::music_clear(unsigned int deck_id) {
  /// Clear the playlist on the specified deck
  #ifndef NDEBUG
    if(deck_id >= decks.size()) {                                               // safety check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
      return;
    }
  #endif // NDEBUG
  while(!decks[deck_id].playlist.empty()) {
    delete decks[deck_id].playlist.front();
    decks[deck_id].playlist.pop();
    //decks[deck_id].buffer_read_seek = deck_buffer_size;                         // wind it to the end of the current buffer
    //decks[deck_id].buffer_needs_filled = true;
  }
}

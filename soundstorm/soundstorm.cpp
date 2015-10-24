#include "soundstorm.h"
#include "platform_defines.h"
#include "cast_if_required.h"
#ifdef PLATFORM_LINUX
  #include "pa_linux_alsa.h"
#endif // PLATFORM_LINUX
#ifndef NDEBUG
  #include <cassert>
#endif

soundstorm::soundstorm() try
  : audio_system_auto(false),             // don't initialise portaudio automatically
    listener_rotation(Quatd::fromEulerAngles(0.0, 0.0, 0.0)) {
  /// Default constructor with integrated try/catch
  // Initialise all global audio setup
  try {
    audio_system_auto.initialize();
    audio_system = &portaudio::System::instance();
  } catch(portaudio::PaException const &e) {
    std::cout << "SoundStorm: audio_system: PortAudio exception: " << e.paErrorText() << std::endl;
    return;
  } catch(portaudio::PaCppException const &e) {
    std::cout << "SoundStorm: audio_system: PortAudioCpp exception: " << e.what() << std::endl;
    return;
  } catch(std::exception const &e) {
    std::cout << "SoundStorm: audio_system: Generic exception: " << e.what() << std::endl;
    return;
  } catch(...) {
    std::cout << "SoundStorm: audio_system: Unknown exception!" << std::endl;
    return;
  }
  std::cout << "SoundStorm: " << audio_system->versionText() << " build " << audio_system->version() << std::endl;
  if(!audio_system->exists()) {
    std::cout << "SoundStorm: Error: sound engine claims not to exist." << std::endl;
    return;
  }

  // select an appropriate sound device
  audio_device = &audio_system->defaultOutputDevice();
  dump_device_info();                     // this also updates num_devices
  init_device();                          // initialise the currently selected device

  set_listener_position_and_rotation(Vector3f(0.0f, 0.0f, 0.0f), Quatf::fromEulerAngles(0.0f, 0.0f, 0.0f));   // initial positions for the ears
} catch(portaudio::PaException const &e) {
  std::cout << "SoundStorm: PortAudio exception: " << e.paErrorText() << std::endl;
} catch(portaudio::PaCppException const &e) {
  std::cout << "SoundStorm: PortAudioCpp exception: " << e.what() << std::endl;
} catch(std::exception const &e) {
  std::cout << "SoundStorm: Generic exception: " << e.what() << std::endl;
} catch(...) {
  std::cout << "SoundStorm: Unknown exception!" << std::endl;
}

soundstorm::~soundstorm() {
  /// Default destructor
  dump_session_report();
  stop_streamer();
  auto playing_backup = playing;                // so we can delete these after
  playing.clear();
  decks.clear();
  stream->abort();                              // tell the stream to stop without waiting for the buffers to finish
  for(auto &it : playing_backup) {
    delete it;
  }
  for(auto &it : effect_library) {
    delete it;
  }
  effect_library.clear();
  shutdown_device();
  audio_system->terminate();                    // release audio resources
}

void soundstorm::init_device() {
  /// Initialise the currently selected audio device and start the stream
  ears.resize(2);         // safe default number of ears, in case we exit initialisation early
  if(num_devices == 0) {
    enabled = false;
    std::cout << "SoundStorm: No audio devices found!  Cancelling initialisation." << std::endl;
    return;
  }
  if(channels > static_cast<unsigned int>(audio_device->maxOutputChannels())) {
    // clamp number of channels in case we're asking for too many
    channels = audio_device->maxOutputChannels();
  }
  if(channels < 2) {
    channels = 2;   // but also clamp to a minimum of 2, since the engine assumes stereo separation
    enabled = false;
    std::cout << "SoundStorm: Fewer than two channels found on selected device!  Cancelling initialisation." << std::endl;
    return;
  }
  #ifdef DEBUG_SOUNDSTORM
    std::cout << "SoundStorm: DEBUG: Enabling " << channels << " channels." << std::endl;
  #endif // DEBUG_SOUNDSTORM
  ears.resize(channels);

  if(stream) {
    shutdown_device();      // make sure we clean up any already open streams
  }

  // set up the parameters required to open a (Callback)Stream:
  stream_out_params = new portaudio::DirectionSpecificStreamParameters(
    *audio_device,                                            // device
    channels,                                                 // output channels
    portaudio::FLOAT32,                                       // sample data format http://riot.so/portaudiocpp/a00060.html#a30bc71f065706d41a5d9208ea861e4a6
    false,                                                    // interleaved
    audio_device->defaultLowOutputLatency(),                  // latency
    NULL);                                                    // hostApiSpecificStreamInfo
  stream_params = new portaudio::StreamParameters(
    portaudio::DirectionSpecificStreamParameters::null(),     // input stream parameters
    *stream_out_params,                                       // output stream parameters
    samplerate,                                               // sample rate
    frames_per_buffer,                                        // frames per buffer for a CallbackStream, or the preferred buffer granularity for a BlockingStream.
    paClipOff | paDitherOff);                                 // The flags for the stream, default paNoFlag http://portaudio.com/docs/v19-doxydocs/portaudio_8h.html#ad33384abe3754a39f4773f2561773595

  stream = new portaudio::MemFunCallbackStream<soundstorm>(
    *stream_params,                                           // stream parameters
    *this,                                                    // class instance
    &soundstorm::mixer);                                      // member function to call

  #ifdef PLATFORM_LINUX
    int alsacard;
    int const error = PaAlsa_GetStreamOutputCard(stream->paStream(), &alsacard);
    if(error != 0) {
      std::cout << "SoundStorm: Error querying ALSA for stream output card: " << error << std::endl;
    }
    std::cout << "SoundStorm: Requesting realtime scheduling from ALSA on card " << alsacard << std::endl;
    PaAlsa_EnableRealtimeScheduling(stream->paStream(), true);
  #endif // PLATFORM_LINUX

  samplerate = static_cast<float>(stream->sampleRate());                        // cache sample rate
  deck_buffer_size = static_cast<unsigned int>(samplerate * 2.0f);              // update buffer size
  resize_decks();                                                               // needs to happen before mixer starts
  #ifndef NSOUND
    stream->start();                                          // start the stream
  #endif // NSOUND
  enabled = true;
  std::cout << "SoundStorm: Initialised." << std::endl;
}
void soundstorm::resize_decks() {
  /// Resize the decks to the correct number and buffer size, and reset the buffers
  // decks must be initialised before the mixer starts
  decks.resize(numdecks);
  for(deck &thisdeck : decks) {
    // initialise deck output buffers to zero
    thisdeck.buffer_l[0].resize(deck_buffer_size, 0.0f);
    thisdeck.buffer_r[0].resize(deck_buffer_size, 0.0f);
    thisdeck.buffer_l[1].resize(deck_buffer_size, 0.0f);
    thisdeck.buffer_r[1].resize(deck_buffer_size, 0.0f);
    //thisdeck.buffer_read = 1;             // so that buffer 0 will be pre-filled
    thisdeck.buffer_read = 0;
    thisdeck.buffer_needs_filled = true;
  }
}
void soundstorm::shutdown_device() {
  /// Shut down the current output device and free it, in preparation of exit or re-init
  if(stream) {
    if(!stream->isStopped()) {
      stream->stop();
    }
    stream->close();
    delete stream;
    stream = nullptr;
  }
  delete stream_out_params;
  stream_out_params = nullptr;
  delete stream_params;
  stream_params = nullptr;
  enabled = false;
  std::cout << "SoundStorm: Shutdown complete." << std::endl;
}

void soundstorm::restart_device() {
  /// Shut down and reinitialise the stream with the currently selected device - needed after device change
  std::cout << "SoundStorm: Restarting device..." << std::endl;
  bool const reinitialise_streamer = streamer_run;
  if(reinitialise_streamer) {           // if the streamer's currently running, shut it down
    stop_streamer();
  }
  shutdown_device();
  init_device();
  if(reinitialise_streamer) {           // if the streamer was running at restart, restart it
    start_streamer();
  }
}

void soundstorm::start_streamer() {
  /// Initialise the streamer this must be done after the playlist is initialised
  std::cout << "SoundStorm: Starting streamer thread..." << std::endl;
  if(decks.empty()) {
    std::cout << "SoundStorm: ERROR: streamer asked to play from non-existant decks." << std::endl;
    return;
  }
  //streamer_run = false;
  //streamer();
  //for(deck &thisdeck : decks) {
  //  thisdeck.buffer_read = 0;             // so the pre-filled buffer will be played first
  //}
  streamer_run = true;

  // start the streaming decoder thread
  #ifndef NSOUND
    streamer_thread = new std::thread(std::bind(&soundstorm::streamer, this));
  #endif // NSOUND
}

void soundstorm::stop_streamer() {
  /// Stop the streamer thread if it's running
  if(streamer_thread) {
    std::cout << "SoundStorm: Stopping streamer thread..." << std::endl;
    streamer_run = false;                         // tell the streamer not to run another cycle
    // available with boost::thread only, not std::thread:
    //streamer_thread->interrupt();                 // and send an interrupt signal for quicker cleanup
    if(streamer_thread->joinable()) {
      try {
        streamer_thread->join();                  // wait for the streamer thread to finish
      } catch(...) {                              // ignore exceptions
      }
    }
    delete streamer_thread;
    streamer_thread = nullptr;
  }
}

int soundstorm::mixer(void const *buffer_in __attribute__((__unused__)),
                      void *buffer_out,
                      unsigned long frames,
                      PaStreamCallbackTimeInfo const *time_info __attribute__((__unused__)),
                      PaStreamCallbackFlags status_flags __attribute__((__unused__))) {
  /// The callback that feeds sample data to the sound stream
  #ifdef DEBUG_SOUNDSTORM
    assert(buffer_out != NULL);
    if(playing.size() > session_max_simultaneous_sounds) {
      session_max_simultaneous_sounds = playing.size();
    }
  #endif

  // check and slide the HDR window
  //hdr_window_top    -= hdr_dropback_rate;       // first droop by the default amount
  //hdr_window_bottom -= hdr_dropback_rate;
  hdr_window_top    *= hdr_dropback_rate;       // first droop by the default amount
  hdr_window_bottom *= hdr_dropback_rate;

  #ifdef SOUNDSTORM_NO_SSE
    if(hdr_window_top < hdr_window_top_min) {   // clamp
      hdr_window_top = hdr_window_top_min;
    }
    if(hdr_window_bottom < 0.0f) {              // clamp
      hdr_window_bottom = 0.0f;
    }
  #else
    _mm_store_ss(&hdr_window_top,    _mm_max_ss(_mm_set_ss(hdr_window_top),    _mm_set_ss(hdr_window_top_min)));  // SSE intrinsicts: branchless max
    _mm_store_ss(&hdr_window_bottom, _mm_max_ss(_mm_set_ss(hdr_window_bottom), _mm_set_ss(0.0f)));                // SSE intrinsicts: branchless max
  #endif // SOUNDSTORM_NO_SSE

  float **out = static_cast<float**>(buffer_out);
  for(unsigned int i = 0; i != frames; ++i) {
    for(unsigned int channel = 0; channel != channels; ++channel) {   // clear all channels
      out[channel][i] = 0.0;
    }
    float max_level __attribute__((__aligned__(16))) = 0.0;           // keep track of the maximum output level this frame

    // play the sound effects
    for(auto it = playing.begin(); it != playing.end();) {
      sound &thissound = **it;
      // stereo positioning
      ear const &thisear = ears[thissound.channel];
      float const distance_sq = (thisear.position - thissound.position).lengthSq();
      float const distance = std::sqrt(distance_sq);
      #ifdef DEBUG_SOUNDSTORM
        if(distance < session_min_distance) {
          session_min_distance = distance;
        }
        if(distance > session_max_distance) {
          session_max_distance = distance;
        }
      #endif // DEBUG_SOUNDSTORM
      float const seek_delay = distance / speed_of_sound;
      #ifdef DEBUG_SOUNDSTORM
        //std::cout << "DEBUG: " << playing.size() << " pos " << Vector3i(thissound.position) << " dist " << distance << " ch" << thissound.channel << " delay " << seek_delay << "s" << std::endl;
      #endif // DEBUG_SOUNDSTORM
      float const angle_ratio = std::acos(thisear.orientation.dotProduct(thissound.position - thisear.position) / distance) / static_cast<float>(M_PI);
      float const head_shadow_delay = head_shadow_delay_max * angle_ratio;
      float apparent_seek __attribute__((__aligned__(16))) = thissound.seek - ((seek_delay + head_shadow_delay) * samplerate);    // rewind to account for time delays
      if(apparent_seek >= 0.0f) {                            // avoid trying to play before the start of the effect
        if(apparent_seek >= thissound.effect->buffersize ||
           (thissound.seek_end != 0.0f && apparent_seek >= thissound.seek_end)) {
          // we've reached the end of this effect or our own seek limit
          if(thissound.next_sound) {
            if(thissound.next_sound == &thissound) {
              // this is a one-sound loop, so just rewind to the beginning
              thissound.seek = -thissound.seek_speed;         // rewind one step make sure the first sample is really 0
              apparent_seek = thissound.seek - seek_delay;    // rewind to account for time delay
              #ifdef SOUNDSTORM_NO_SSE
                if(apparent_seek < 0.0f) {  // avoid trying to play before the start of the effect
                  apparent_seek = 0.0f;
                }
              #else
                _mm_store_ss(&apparent_seek, _mm_max_ss(_mm_set_ss(apparent_seek), _mm_set_ss(0.0f)));  // SSE intrinsicts: branchless max
              #endif // SOUNDSTORM_NO_SSE
            } else {
              // we have something else queued up, so replace us with it and destroy the original
              sound *oldsound = *it;
              *it = oldsound->next_sound;
              delete oldsound;
            }
          } else {
            #ifdef DEBUG_SOUNDSTORM
              std::cout << "SoundStorm: DEBUG: finished playing sound at seek point " << apparent_seek << " after " << apparent_seek / samplerate << "s" << std::endl;
            #endif // DEBUG_SOUNDSTORM
            it = playing.erase(it);
            continue;                               // we have nothing to contribute to the stream
          }
        }
        // directional attenuation
        float const directional_attenuation = 1.0f - (head_shadow_attenuation * angle_ratio);
        float const sample = (thissound.effect->buffer[static_cast<unsigned int>(apparent_seek)] *
                              thissound.effect->hdr_scale *
                              thissound.volume *
                              directional_attenuation) / distance_sq;
        #ifdef DEBUG_SOUNDSTORM
          //std::cout << "DEBUG: " << thissound.channel << " ancos " << anglecos << " anPI " << angle / M_PI << " att " << directional_attenuation << " sam " << sample << std::endl;
        #endif // DEBUG_SOUNDSTORM
        out[thissound.channel][i] += sample;
        #ifdef SOUNDSTORM_NO_SSE
          float const absolute_level = std::abs(out[thissound.channel][i]);
          if(max_level < absolute_level) { // keep track of the maximum output level this frame
            max_level = absolute_level;
          }
        #else
          //_mm_store_ss(&max_level, _mm_max_ss(_mm_set_ss(max_level), _mm_set_ss(std::abs(out[thissound.channel][i]))));  // SSE intrinsicts: branchless max
          // SSE abs() implementation hack, see http://fastcpp.blogspot.co.uk/2011/03/changing-sign-of-float-values-using-sse.html
          static __m128 const signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));                  // SSE intrinsic bitmask for float sign
          __m128 const absolute_level = _mm_andnot_ps(signmask, _mm_set_ss(out[thissound.channel][i])); // SSE intrinsics: and not
          _mm_store_ss(&max_level, _mm_max_ss(_mm_set_ss(max_level), absolute_level));                  // SSE intrinsicts: branchless max
        #endif // SOUNDSTORM_NO_SSE
      }
      thissound.seek += thissound.seek_speed;
      ++it;                                         // only increment here in case we erase instead
    }

    // play the music
    for(deck &thisdeck : decks) {
      // perform any fades we need to
      #ifdef SOUNDSTORM_NO_SSE
        thisdeck.volume += std::min(std::max(thisdeck.volume_target - thisdeck.volume, thisdeck.volume_fadespeed), -thisdeck.volume_fadespeed);
      #else
        static __m128 const signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));                        // SSE intrinsic bitmask for float sign
        //__m128 volume_shift = _mm_sub_ss(_mm_set_ss(thisdeck.volume_target), _mm_set_ss(thisdeck.volume));  // SSE intrinsics: subtract
        __m128 const volume_target = _mm_set_ss(thisdeck.volume_target);
        __m128 const volume        = _mm_set_ss(thisdeck.volume);
        __m128 volume_shift = _mm_sub_ss(volume_target, volume);                                            // SSE intrinsics: subtract
        __m128 const fadespeed_max = _mm_set_ss(thisdeck.volume_fadespeed);
        __m128 const fadespeed_min = _mm_xor_ps(fadespeed_max, signmask);                                   // SSE intrinsics: xor (to flip the sign)
        volume_shift = _mm_min_ss(volume_shift, fadespeed_max);                                             // SSE intrinsics: branchless min (clamp top)
        volume_shift = _mm_max_ss(volume_shift, fadespeed_min);                                             // SSE intrinsics: branchless max (clamp bottom)
        _mm_store_ss(&thisdeck.volume, _mm_add_ss(_mm_set_ss(thisdeck.volume), volume_shift));              // SSE intrinsics: add
      #endif // SOUNDSTORM_NO_SSE

      out[channel_type::LEFT ][i] += thisdeck.buffer_l[thisdeck.buffer_read][thisdeck.buffer_read_seek] * thisdeck.volume;
      out[channel_type::RIGHT][i] += thisdeck.buffer_r[thisdeck.buffer_read][thisdeck.buffer_read_seek] * thisdeck.volume;
      ++thisdeck.buffer_read_seek;
      if(thisdeck.buffer_read_seek == deck_buffer_size) {           // we've reached the end of this buffer
        thisdeck.buffer_read = 1 - thisdeck.buffer_read;            // flip the ping-pongs
        thisdeck.buffer_needs_filled = true;                        // flag it as needing refilled
        thisdeck.buffer_read_seek = 0;                              // rewind
        #ifdef DEBUG_SOUNDSTORM
          //std::cout << "SoundStorm: DEBUG: deck " << &thisdeck << " buffer flipped to " << thisdeck.buffer_read << std::endl;
        #endif // DEBUG_SOUNDSTORM
      }
    }

    // scale the HDR windows for this frame
    #if defined(SOUNDSTORM_NO_SSE) || defined(DEBUG_SOUNDSTORM)     // skip using branchless intrins if we need debugging output
      if(hdr_window_top < max_level) {
        hdr_window_top = max_level;
        //hdr_window_bottom = std::min(0.0f, hdr_window_top - max_level);   // slide up the bottom of the window to match
        #ifdef DEBUG_SOUNDSTORM
          std::cout << "SoundStorm: DEBUG: HDR window raised to " << hdr_window_bottom << " - " << hdr_window_top << std::endl;
          if(hdr_window_top > session_max_hdr_window_top) {
            session_max_hdr_window_top = hdr_window_top;
            std::cout << "SoundStorm: DEBUG: HDR window top new session max: " << session_max_hdr_window_top << std::endl;
          }
          if(hdr_window_bottom > session_max_hdr_window_bottom) {
            session_max_hdr_window_bottom = hdr_window_bottom;
            std::cout << "SoundStorm: DEBUG: HDR window bottom new session max: " << session_max_hdr_window_bottom << std::endl;
          }
        #endif // DEBUG_SOUNDSTORM
      }
    #else
      _mm_store_ss(&hdr_window_top, _mm_max_ss(_mm_set_ss(hdr_window_top), _mm_set_ss(max_level)));  // SSE intrinsicts: branchless max
    #endif // SOUNDSTORM_NO_SSE || DEBUG_SOUNDSTORM
    float const final_scale = volume_master / hdr_window_top;         // final global volume control and HDR window scaling
    for(unsigned int channel = 0; channel != channels; ++channel) {   // scale all channels
      out[channel][i] *= final_scale;
    }
  }
  return paContinue;
}

void soundstorm::streamer() {
  /// Streaming decoder that checks and fills the deck buffers
  #ifdef DEBUG_SOUNDSTORM
    std::cout << "SoundStorm: DEBUG: streamer starting" << std::endl;
  #endif // DEBUG_SOUNDSTORM
  ov_callbacks callbacks;
  callbacks.read_func  = &soundstorm::ogg_callback_read;
  //callbacks.seek_func  = &soundstorm::ogg_callback_seek;          // can be NULL to treat as non-seekable
  callbacks.seek_func  = NULL;
  //callbacks.close_func = &soundstorm::ogg_callback_close;         // or can just be NULL for no close
  callbacks.close_func = NULL;
  //callbacks.tell_func  = &soundstorm::ogg_callback_tell;          // can be NULL to treat as non-seekable
  callbacks.tell_func  = NULL;

  do {
    for(auto &thisdeck : decks) {
      if(!thisdeck.oggfile) {                                       // initialise the per-deck ogg decoders
        if(thisdeck.playlist.empty()) {
          #ifdef DEBUG_SOUNDSTORM
            std::cout << "SoundStorm: WARNING: deck " << &thisdeck << " given empty playlist" << std::endl;
          #endif // DEBUG_SOUNDSTORM
          continue;
        }
        thisdeck.oggfile = new OggVorbis_File;
        int result = ov_open_callbacks(&thisdeck, thisdeck.oggfile, NULL, 0, callbacks);
        if(result != 0) {
          switch(result) {
          case OV_EREAD:
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks A read from media returned an error: " << result << std::endl;
            break;
          case OV_ENOTVORBIS:
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Bitstream does not contain any Vorbis data: " << result << std::endl;
            break;
          case OV_EVERSION:
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Vorbis version mismatch: " << result << std::endl;
            break;
          case OV_EBADHEADER:
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Invalid Vorbis bitstream header: " << result << std::endl;
            break;
          case OV_EFAULT:
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: Internal logic fault; indicates a bug or heap/stack corruption: " << result << std::endl;
            break;
          default:
            std::cout << "SoundStorm: ERROR: deck " << &thisdeck << " ov_open_callbacks: unknown error " << result << std::endl;
            break;
          }
          return;
        }
        #ifdef DEBUG_SOUNDSTORM
          char **comment = ov_comment(thisdeck.oggfile, -1)->user_comments;
          while(*comment) {
            std::cout << "SoundStorm: deck " << &thisdeck << " Ogg comment: " << comment <<std::endl;
            ++comment;
          }
          vorbis_info *info = ov_info(thisdeck.oggfile, -1);
          std::cout << "SoundStorm: deck " << &thisdeck << " Bitstream is " << info->channels << " channel, " << info->rate << "Hz" << std::endl;
          std::cout << "SoundStorm: deck " << &thisdeck << " Decoded length: " << ov_pcm_total(thisdeck.oggfile, -1) << " samples" << std::endl;
          std::cout << "SoundStorm: deck " << &thisdeck << " Encoded by: " << ov_comment(thisdeck.oggfile, -1)->vendor << std::endl;
        #endif // DEBUG_SOUNDSTORM
      }
      if(thisdeck.buffer_needs_filled) {
        thisdeck.buffer_needs_filled = false;                           // reset the flag first
        unsigned int const buffer_write = 1 - thisdeck.buffer_read;
        #ifdef DEBUG_SOUNDSTORM
          //std::cout << "SoundStorm: DEBUG: deck " << &thisdeck << " buffer " << buffer_write << " refilling..." << std::endl;
        #endif // DEBUG_SOUNDSTORM
        for(unsigned int i = 0; i != deck_buffer_size;) {
          int current_section;        // what the hell is this even used for?
          float **pcm_channels;
          int samples_read;
          do {
            samples_read = static_cast<int>(ov_read_float(thisdeck.oggfile, &pcm_channels, deck_buffer_size - i, &current_section));
            switch(samples_read) {
            case 0:               // EOF
              //std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill got EOF " << samples_read << std::endl;
              break;
            case OV_HOLE:         // indicates there was an interruption in the data. (one of: garbage between pages, loss of sync followed by recapture, or a corrupt page)
              #ifdef DEBUG_SOUNDSTORM
                //std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill failed with OV_HOLE " << samples_read << std::endl;
              #endif // DEBUG_SOUNDSTORM
              break;              // this is normal when switching tracks
            case OV_EBADLINK:     // indicates that an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.
              std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill failed with OV_EBADLINK " << samples_read << std::endl;
              break;
            case OV_EINVAL:       // indicates the initial file headers couldn't be read or are corrupt, or that the initial open call for vf failed.
              std::cout << "SoundStorm: ERROR: streamer: deck " << &thisdeck << " fill failed with OV_EINVAL " << samples_read << std::endl;
              break;
            }
          } while(samples_read <= 0);
          for(int s = 0; s != samples_read; ++s) {
            thisdeck.buffer_l[buffer_write][i + s] = pcm_channels[LEFT ][s];
            thisdeck.buffer_r[buffer_write][i + s] = pcm_channels[RIGHT][s];
          }
          #ifdef DEBUG_SOUNDSTORM
            std::cout << "SoundStorm: DEBUG: streamer read " << samples_read << " bytes of " << deck_buffer_size - i << ", current_section " << current_section << std::endl;
          #endif // DEBUG_SOUNDSTORM
          i += static_cast<unsigned int>(samples_read);
        }
        #ifdef DEBUG_SOUNDSTORM
          //std::cout << "SoundStorm: DEBUG: deck " << &thisdeck << " buffer " << buffer_write << " refilled" << std::endl;
        #endif // DEBUG_SOUNDSTORM
      }
    }
    float constexpr buffer_fill_sleep = 1.0f / 4.0f;                            // sleep for this fraction of buffer fill time to avoid spin-waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<unsigned int>(1000.0f * static_cast<float>(deck_buffer_size) / samplerate * 0.5f * buffer_fill_sleep)));
  } while(streamer_run);

  for(auto &thisdeck : decks) {     // cleanup
    ov_clear(thisdeck.oggfile);
    delete thisdeck.oggfile;
    thisdeck.oggfile = nullptr;
  }
  #ifdef DEBUG_SOUNDSTORM
    std::cout << "SoundStorm: DEBUG: streamer finished cleanly" << std::endl;
  #endif // DEBUG_SOUNDSTORM
}

size_t soundstorm::ogg_callback_read(void *ptr, size_t size, size_t count, void *datasource) {
  /// The interface is identical to that of fread, and identical behaviour is expected
  #ifdef DEBUG_SOUNDSTORM
    //std::cout << "SoundStorm: DEBUG: streamer read requested size " << size << " count " << count << std::endl;
  #endif // DEBUG_SOUNDSTORM
  if(!datasource) {
    #ifdef DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
    #endif // DEBUG_SOUNDSTORM
    return 0;                         // nullptr means we've got nothing playing
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
      thismusic->seek = 0;                                          // rewind so that we advance to 0 this frame
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
          std::cout << "SoundStorm: DEBUG: streamer replacing music with top of playlist" << std::endl;
        #endif // DEBUG_SOUNDSTORM
        delete thismusic;
        thisdeck->playlist.pop();                                     // take the current entry off the playlist
        thismusic = thisdeck->playlist.front();                       // tell the track to replace itself
      }
    }
    // NOTE: could probably improve this using a block memcpy or std::copy
  }
  #ifdef DEBUG_SOUNDSTORM
    //std::cout << "SoundStorm: DEBUG: streamer read completed with " << i << " bytes" << std::endl;
  #endif // DEBUG_SOUNDSTORM
  return i;
}

int soundstorm::ogg_callback_seek(void *datasource, ogg_int64_t offset, int origin) {
  /// The interface is identical to that of fseek, and identical behaviour is expected
  if(!datasource) {
    #ifdef DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
    #endif // DEBUG_SOUNDSTORM
    return 0;                         // nullptr means we've got nothing playing
  }
  deck *thisdeck = reinterpret_cast<deck*>(datasource);
  music *thismusic = thisdeck->playlist.front();

  switch(origin) {
  case SEEK_SET:
    thismusic->seek = offset;
    break;
  case SEEK_CUR:
    thismusic->seek += offset;
    break;
  case SEEK_END:
    #ifdef DEBUG_SOUNDSTORM
      if(!thismusic->buffer) {
        std::cout << "SoundStorm: DEBUG: no library music assigned to this playlist entry passed as datasource to ogg_callback_seek - this should never happen!" << std::endl;
        return -1;
      }
    #endif // DEBUG_SOUNDSTORM
    thismusic->seek = thismusic->buffer->buffersize + offset;
    break;
  default:
    return -1;
  }
  return 0;
}

int soundstorm::ogg_callback_close(void *datasource __attribute__((__unused__))) {
  /// The interface is identical to that of fclose, and identical behaviour is expected
  // we do absolutely nothing
  return 0;
}

long soundstorm::ogg_callback_tell(void *datasource) {
  /// The interface is identical to that of ftell, and identical behaviour is expected
  if(!datasource) {
    #ifdef DEBUG_SOUNDSTORM
      std::cout << "SoundStorm: DEBUG: datasource passed as nullptr to " << __PRETTY_FUNCTION__ << std::endl;
    #endif // DEBUG_SOUNDSTORM
    return 0;                         // nullptr means we've got nothing playing
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
    if(&it == audio_device) {         // mark the currently selected device
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

Vector3f const &soundstorm::get_listener_position() const {
  /// Return the current listener world position
  return listener_position;
}
Quatf const &soundstorm::get_listener_rotation() const {
  /// Return the current listener rotation
  return listener_rotation;
}
Vector3f const &soundstorm::get_listener_velocity() const {
  /// Return the current listener world position
  return listener_velocity;
}

void soundstorm::set_listener_position(Vector3f const &newposition) {
  /// Update the world position of the listener - also updates the velocity
  listener_velocity = newposition - listener_position;
  listener_position = newposition;
  update_ears();
}
void soundstorm::set_listener_rotation(Quatf const &newrotation) {
  /// Update the facing direction of the listener
  listener_rotation = newrotation;
  update_ears();
}
void soundstorm::set_listener_velocity(Vector3f const &newvelocity) {
  /// Update the velocity of the listener through the medium
  listener_velocity = newvelocity;
}
void soundstorm::set_listener_position_and_rotation(Vector3f const &newposition, Quatf const &newrotation) {
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
  thiseffect->buffer = reinterpret_cast<float const*>(buffer);    // treat the buffer as one of 32bit floats
  thiseffect->buffersize = buffersize / sizeof(float);            // convert to our size in samples
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
                      Vector3f const &position,
                      Vector3f const &velocity,
                      float volume,
                      float seek_start,
                      float seek_end,
                      float seek_speed,
                      soundgroup *thissoundgroup) {
  /// Add a sound effect by id to the currently playing list with the specified parameters
  play(position, velocity, get_effect(effect_id), volume, seek_start, seek_end, seek_speed, thissoundgroup);
}

void soundstorm::play(Vector3f const &position,
                      Vector3f const &velocity,
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
    if(!effect) {   // null check only in debug mode
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
                           Vector3f const &position,
                           Vector3f const &velocity,
                           float volume,
                           float seek_start,
                           float seek_end,
                           float seek_speed,
                           soundgroup *thissoundgroup) {
  /// Add a sound effect by id set to repeat indefinitely to the currently playing list with the specified parameters
  return play_loop(position, velocity, get_effect(effect_id), volume, seek_start, seek_end, seek_speed, thissoundgroup);
}

void soundstorm::play_loop(Vector3f const &position,
                           Vector3f const &velocity,
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
    if(!effect) {   // null check only in debug mode
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
    thissoundgroup = nullptr;
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
    if(deck_id >= decks.size()) {   // safety check only in debug mode
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
    if(deck_id >= decks.size()) {   // safety check only in debug mode
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
    if(deck_id >= decks.size()) {   // safety check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
      return;
    }
  #endif // NDEBUG
  decks[deck_id].volume_fadespeed = std::abs(newvolume - decks[deck_id].volume) / (seconds_to_take * samplerate);  // this comes first since we're threaded
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
  for(auto const &thissound : thissoundgroup) {                                       // do this for each channel
    thissound->seek = static_cast<float>(thissound->effect->buffersize);
  }
}

void soundstorm::stop_loop(soundgroup const &thissoundgroup) {
  /// Tell this sound not to loop or continue to the next sound
  for(auto const &thissound : thissoundgroup) {                                       // do this for each channel
    thissound->next_sound = nullptr;
  }
}

void soundstorm::replace(soundgroup const &thissoundgroup, soundeffect *neweffect, float seek_start, float seek_end, float seek_speed) {
  /// Immediately replace the currently playing sound with a new effect with the specified parameters
  #ifndef NDEBUG
    if(!neweffect) {   // null check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with null effect!" << std::endl;
      return;
    }
  #endif // NDEBUG
  for(auto const &thissound : thissoundgroup) {                                       // do this for each channel
    thissound->effect = neweffect;
    thissound->seek = seek_start;
    thissound->seek_end = seek_end;
    thissound->seek_speed = seek_speed;
  }
}

void soundstorm::follow(soundgroup const &thissoundgroup, soundeffect *neweffect, float seek_start, float seek_end, float seek_speed) {
  /// Append another sound to play immediately once this one completes, with the specified parameters
  /// (or specify nullptr to cancel a following sound)
  for(auto const &thissound : thissoundgroup) {                                       // do this for each channel
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

void soundstorm::set_position(soundgroup const &thissoundgroup, Vector3f const &newposition) {
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
    if(deck_id >= decks.size()) {   // safety check only in debug mode
      std::cout << "SoundStorm: Error: Called " << __PRETTY_FUNCTION__ << " with deck_id " << deck_id << " exceeding available decks!" << std::endl;
      return;
    }
  #endif // NDEBUG
  while(!decks[deck_id].playlist.empty()) {
    delete decks[deck_id].playlist.front();
    decks[deck_id].playlist.pop();
  }
}

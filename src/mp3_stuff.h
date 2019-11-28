//#define MINIMP3_ONLY_MP3
//#define MINIMP3_ONLY_SIMD
//#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL
//#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"


#define MP3_BUFFER_SIZE 10000
static mp3dec_t mp3d;
uint8_t mp3_buffer[MP3_BUFFER_SIZE];
volatile uint32_t mp3_buffer_read_position = 0;
volatile uint32_t mp3_buffer_write_position = 0;
volatile uint32_t mp3_buffer_available = 0;
short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
volatile int pcm_samples_available = 0;
volatile int pcm_position = 0;
volatile int mp3_lock = 0;
volatile int mp3_decode_stopped = 1;
mp3dec_frame_info_t mp3_info;



// uint8_t opus_buffer[5000];
// int opus_buffer_position = 0;
// uint8_t opus_buffer_available = 0;
// int16_t pcm_buffer[2000];
// int pcm_buffer_position = 0;
// int pcm_space = 10000;


void mp3_setup() {
  Serial.println("mp3 initialized");
  scratch = new mp3dec_scratch_t();
  mp3dec_init(&mp3d);
}

void decode_mp3_loop() {
    if (!mp3_decode_stopped && !pcm_samples_available) {
        
        //Serial.println("attempting to decode");
        //Serial.println(mp3_buffer_read_position);
        //Serial.println(mp3_buffer_available);
        int samples = mp3dec_decode_frame(&mp3d, &mp3_buffer[mp3_buffer_read_position], mp3_buffer_available, pcm, &mp3_info);
        
        //int samples = opus_decode(opusDecoder, &mp3_buffer[mp3_buffer_read_position], mp3_buffer_available, pcm, MINIMP3_MAX_SAMPLES_PER_FRAME, 0);

        //Serial.println("mp3 frame decoded");
        int bytes = mp3_info.frame_bytes;
        //int bytes = mp3_buffer_available;
        //Serial.print("frame bytes: ");
        //Serial.println(bytes);
        while (mp3_lock) {}
        mp3_lock = 1;
        
        mp3_buffer_available -= bytes;
        memmove(&mp3_buffer[0], &mp3_buffer[mp3_buffer_read_position+bytes], mp3_buffer_available);
        mp3_buffer_read_position = 0;
        mp3_buffer_write_position -= bytes;
        pcm_samples_available+=samples;
        pcm_position=0;

        mp3_lock = 0;
        
    }
}

void mp3_loop() {
   while (pcm_samples_available) {
    
    if (i2s_write_sample_nb((((uint16_t)pcm[pcm_position])<<16)|((uint16_t)pcm[pcm_position+1])) == ESP_OK) {
      pcm_position+=2;
      pcm_samples_available--;
    }
    
    if (i2s_stopped) {
      i2s_start((i2s_port_t)i2s_num);
      i2s_stopped = 0;
    }

  }

  if (!i2s_stopped && mp3_buffer_available < 1000) {
      i2s_stop((i2s_port_t)i2s_num);
      i2s_stopped = 1;
      mp3_decode_stopped = 1;
  }

  if (mp3_buffer_available > (MP3_BUFFER_SIZE*3)/4) {
      mp3_decode_stopped = 0;
  }

  decode_mp3_loop();

}
#define OS_TICKS_PER_SEC 1000

#include "opus.h"
OpusDecoder * opusDecoder;

#define NUM_OPUS_BUFFERS 100
#define OPUS_BUFFER_SIZE 500
#define OPUS_FRAME_SIZE 10 //in milliseconds, valid values are: 2.5, 5, 10, 20
uint8_t opus_buffer[NUM_OPUS_BUFFERS][OPUS_BUFFER_SIZE];
int opus_buffer_size[NUM_OPUS_BUFFERS] = {0};
int opus_buffer_sequence[NUM_OPUS_BUFFERS] = {0};
volatile int opus_buffer_write = 0;
volatile int opus_buffer_read = -1;
#define PCM_BUFFER_SIZE 10000
short pcm[PCM_BUFFER_SIZE];
uint8_t * pcm_bytes = (uint8_t*)&pcm[0];
volatile int pcm_max = PCM_BUFFER_SIZE;
volatile int pcm_max_byte = PCM_BUFFER_SIZE*2;
volatile int pcm_write_position = 0;
volatile int pcm_bytes_available = 0;
volatile int pcm_position = 0;
volatile int pcm_byte_position = 0;
volatile int opus_lock = 0;
volatile int opus_decode_stopped = 1;
volatile int opus_buffer_available = 0;
volatile int opus_buffer_max = 0;
volatile int opus_buffer_min = 10000;
volatile int zero_packet_count = 0;
volatile int zero_packet_total = 0;
volatile int opus_error_cnt = 0;
volatile int last_opus_buffer_read = 0;
volatile int current_opus_sequence = 0;

volatile uint32_t out_of_order = 0;
volatile uint16_t sequence_max = 0;
volatile uint16_t packet_per_second = 0;
volatile uint16_t udp_checks_per_second = 0;
volatile uint32_t pcm_bytes_written = 0;

volatile uint32_t debug_time = 0;
volatile uint32_t debug_millis = 0;


volatile uint8_t udp_buffer_available = 0;
uint8_t udp_buffer_available_old = 0;

uint32_t udp_loop_time = 0;
uint32_t udp_loop_cnt = 0;
uint32_t opus_loop_time = 0;
uint32_t loop_task_time = 0;
uint32_t i2s_task_time = 0;
uint32_t loop_task_cnt = 0;
uint32_t i2s_task_cnt = 0;

uint32_t i2s_write_breaks = 0;
uint32_t pcm_sample_loop = 0;

volatile int good_cnt = 0;
int bad_cnt = 0;
int offset = 0;




void opus_setup() {
  //Serial.println("opus initialized");
  int myError;
  opusDecoder = opus_decoder_create(48000, 2, &myError);
}

void decode_opus_loop() {


        //default to 10ms frame, 480 samples
        //this will be updated dynamically from incoming packets
        static int samples_this_frame = 480;

        //adjust forward if we fall behind
        if (sequence_max - current_opus_sequence > 4) {
            if (sequence_max - current_opus_sequence > 100) {
                current_opus_sequence = sequence_max-6;
            } else {
                current_opus_sequence++;
                Serial.print("B");
            }
        }
        //slowly remove our offset (if any) if we have a good signal (no missed packets)
        if (offset < 0 && good_cnt > 25) {
            offset++;
            good_cnt = 0;
            Serial.print("+");
        }
        
    if ( (!opus_decode_stopped) && (!pcm_bytes_available) ) {

        //this is our sequence number
        int my_sequence = current_opus_sequence+offset;
        //this is our Opus buffer location
        int my_buffer = (current_opus_sequence+offset)%NUM_OPUS_BUFFERS;
        //this is the size of the data to be processed
        int buffer_size = opus_buffer_size[my_buffer];

        //increment our sequence number for the next time 'round
        current_opus_sequence++;
        uint8_t *buffer_ptr = NULL;


        if (opus_buffer_sequence[my_buffer] < my_sequence) {
            //don't process this packet, it's old
            static int zpc = 0;
            zpc++;
            zero_packet_count++;
            zero_packet_total++;
            bad_cnt++;
            good_cnt = 0;
            if (zpc > 2) {
                //Serial.print("S");
                //i2s_stop((i2s_port_t)I2S_NUM);
                //i2s_stopped = 1;
                //zpc=0;
            }
            buffer_size=0;
        } else {
            //do process this packet
            good_cnt++;
            bad_cnt=0;
            zero_packet_count = 0;
            buffer_ptr = &opus_buffer[my_buffer][0];
        }

        //get the number of samples that are in this packet
        if (buffer_size) {
            int samples_new_frame = opus_packet_get_nb_samples(buffer_ptr, buffer_size, 48000);
            if (samples_new_frame>0) {
                samples_this_frame = samples_new_frame;
            } else {
                Serial.println("SKIPPED!");
                return;
            }
        }

        //decode the opus packet into raw PCM
        int samples = opus_decode(opusDecoder, buffer_ptr, buffer_size, &pcm[pcm_write_position], samples_this_frame, 0);
        if (samples > 0) {
            pcm_write_position+=samples*2;
            pcm_bytes_available+=samples*4;
        } else {
            opus_error_cnt++;
        }
        
        //check if we have reached the end of PCM memory
        if (pcm_write_position > PCM_BUFFER_SIZE-samples_this_frame*4) {
            pcm_max_byte = pcm_write_position*2;
            pcm_write_position = 0;
        }
        

        
    }
}


void opus_i2s_loop() {
    static uint32_t last_time = 0;
    static uint pcm_processed = 0;
    if (pcm_bytes_available>0) {
        //we have PCM data available
        while (pcm_bytes_available>0) {
            pcm_processed = 1;
            pcm_sample_loop++;
            if (pcm_byte_position >= pcm_max_byte) {
                pcm_byte_position = 0;
            }

            uint32_t bytes_to_write = _min(pcm_bytes_available, pcm_max_byte - pcm_byte_position);

            if (!i2s_ready && i2s_stopped) {
                i2s_ready = 1;
            }

            //write PCM data to the i2s buffer
            int bytes_written = i2s_write_nb(&pcm_bytes[pcm_byte_position],bytes_to_write);
            //Serial.print(bytes_written);
            //Serial.print(" ");

            if (bytes_written > 0) {
                pcm_byte_position+=bytes_written;
                pcm_bytes_available-=bytes_written;
                pcm_bytes_written+=bytes_written;
            }

            
        } 
    } else {
            vTaskDelay(1);
    }

}


void opus_i2s_task(void *pvParameters)
{
    for(;;) {
        // if (i2s_task_cnt == 0) {
        //     vTaskDelay(1);
        // }

        opus_i2s_loop();
        
    }
}

void opus_loop() {


    if (opus_buffer_available > 2 && opus_decode_stopped) {
        offset = 0;
        opus_decode_stopped = 0;
    }

    if (zero_packet_count > 100) {
        opus_decode_stopped = 1;
        sequence_max = 0;
        zero_packet_count = 0;
        current_opus_sequence = 0;
        opus_buffer_available=0;
        offset=0;
    }


    if (!i2s_stopped && opus_decode_stopped && !pcm_bytes_available ) {
        i2s_stopped = 1;
        i2s_ready = 0;
        //i2s_stop((i2s_port_t)I2S_NUM);
        i2s_zero_dma_buffer((i2s_port_t)I2S_NUM);
        opus_decoder_ctl(opusDecoder, OPUS_RESET_STATE);
        //Serial.print("S");
        stop_cnt++;
        for (int i = 0; i < NUM_OPUS_BUFFERS; i++) { //zero out the opus buffers
            opus_buffer_sequence[i]=0;
            opus_buffer_size[i]=0;
        }
    }



    if (i2s_stopped && i2s_ready && opus_buffer_available > 0) {
        //Serial.print("s");
        //i2s_start((i2s_port_t)I2S_NUM);
        i2s_stopped = 0;
    }


  decode_opus_loop();


}
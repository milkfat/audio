
#include "AsyncUDP.h"
//AsyncUDP udp;
AsyncUDP udp2;
//WiFiUDP udp;

int32_t udp_packet_cnt;

#define NUM_UDP_BUFFERS 30
#define UDP_BUFFER_SIZE 300
uint8_t udp_buffers[NUM_UDP_BUFFERS][UDP_BUFFER_SIZE];
int16_t udp_buffer_size[NUM_UDP_BUFFERS];
uint8_t udp_buffer_read_position = 0;
uint8_t udp_buffer_write_position = 0;
//uint8_t udp_buffer_available = 0;

void udp_listener_setup() {

    //udp.begin(4322);

    //listen for RAW pcm packets
    // if(udp.listen(1234)) {
    //     Serial.print("UDP Listening on IP: ");
    //     Serial.println(WiFi.localIP());
    //     udp.onPacket([](AsyncUDPPacket packet) {
    //         //Serial.write(packet.data(), packet.length());
    //         udp_packet_cnt++;
    //         uint32_t * ptr = reinterpret_cast<uint32_t*>(packet.data());
    //         static int buffer_position = 0;

           
    //         for (int i = 0; i < packet.length()/4; i++) {
    //             memcpy8(&sbuffer[buffer_position], ptr, 4);
    //             ptr++;
    //             buffer_position++;
    //             buffer_position %= SBUFFER_SIZE;
    //             buffer_available++;
    //         }

    //         // if(buffer_available>1472) {
    //         //     i2s_stop((i2s_port_t)I2S_NUM);
    //         //     buffer_position+=1472;
    //         //     buffer_position %= SBUFFER_SIZE;
    //         //     buffer_available-=1472;
    //         //     i2s_start((i2s_port_t)I2S_NUM);
    //         // }
    //     });
    // }

    // //listen for mp3 data
    // if(udp2.listen(1235)) {
    //     Serial.print("UDP Listening on IP: ");
    //     Serial.println(WiFi.localIP());
    //     udp2.onPacket([](AsyncUDPPacket packet) {
    //         while (mp3_lock) {vTaskDelay(100);}
    //         mp3_lock = 1;
    //         //Serial.print("packet received");
    //         //Serial.print(" ");
    //         //Serial.write(packet.data(), packet.length());
    //         if(mp3_buffer_write_position < 19000) {
    //         memcpy8(&mp3_buffer[mp3_buffer_write_position], packet.data(), packet.length());
    //         mp3_buffer_write_position+=packet.length();
    //         mp3_buffer_available+=packet.length();
    //         }
    //         //Serial.println(mp3_buffer_available);
    //         mp3_lock = 0;
    //         udp_packet_cnt++;
    //     });
    // }


    //listen for opus data
    

    if(udp2.listen(1236)) {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp2.onPacket([](AsyncUDPPacket packet) {
            memcpy(&udp_buffers[udp_buffer_write_position], packet.data(), packet.length());
            udp_buffer_size[udp_buffer_write_position] = packet.length();
            udp_buffer_available++;
            udp_buffer_write_position++;
            udp_buffer_write_position%=NUM_UDP_BUFFERS;
            packet_per_second++;
        });
    }



}

void udp_loop() {

    // int message = udp.parsePacket();

    // udp_checks_per_second++;
    
    // if (message) {
    //     udp.read(udp_buffers[udp_buffer_write_position], message);
    //     udp_buffer_size[udp_buffer_write_position] = message;
    //     udp_buffer_available++;
    //     udp_buffer_write_position++;
    //     udp_buffer_write_position%=NUM_UDP_BUFFERS;
    //     if (udp.available()) {
    //         udp.flush();
    //     }
    // }

}

void parse_udp_packets() {
    while (udp_buffer_available) {
        udp_buffer_available--;
        if(udp_buffer_size[udp_buffer_read_position] <= 12 || udp_buffer_size[udp_buffer_read_position] >= UDP_BUFFER_SIZE-12) {    
            udp_buffer_size[udp_buffer_read_position] = 0;
            udp_buffer_read_position++;
            udp_buffer_read_position%=NUM_UDP_BUFFERS;
            continue;
        };
        
        uint16_t sequence = udp_buffers[udp_buffer_read_position][2]*256 + udp_buffers[udp_buffer_read_position][3];
        int my_opus_buffer = sequence%NUM_OPUS_BUFFERS;


        if (sequence > opus_buffer_sequence[my_opus_buffer]) { //write newer packets into the buffer
            opus_buffer_size[my_opus_buffer] = udp_buffer_size[udp_buffer_read_position]-12;
            opus_buffer_sequence[my_opus_buffer] = sequence;
            memcpy(&(opus_buffer[my_opus_buffer]), &(udp_buffers[udp_buffer_read_position][12]), udp_buffer_size[udp_buffer_read_position]-12);

            if (udp_buffer_available > 2) {
                if (-udp_buffer_available < offset) {
                    Serial.print("-");
                }
                offset = _min(-udp_buffer_available,offset);
                good_cnt = -40-udp_buffer_available;
            }
            // if (opus_buffer_available==0) {
            //     current_opus_sequence = _max(sequence-4,current_opus_sequence);
            // }
            opus_buffer_available++;
        } else {
            udp_buffer_size[udp_buffer_read_position] = 0;
            udp_buffer_read_position++;
            udp_buffer_read_position%=NUM_UDP_BUFFERS;
            continue;
        }
        

        int sequence_diff = sequence - sequence_max;

        if (sequence_diff > 1) {

            sequence_max = sequence;

        } else if (sequence_diff < -100) {
            sequence_max = 0;
            current_opus_sequence = 0;
        }

        if (opus_buffer_read == -1) {
            opus_buffer_read = my_opus_buffer;
        }

        udp_buffer_size[udp_buffer_read_position] = 0;
        udp_buffer_read_position++;
        udp_buffer_read_position%=NUM_UDP_BUFFERS;

    }

}

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

    //listen for UDP packets and copy them to our UDP buffer for later processing
    

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

//process any available packets from the UDP buffer
void parse_udp_packets() {
    while (udp_buffer_available) {
        udp_buffer_available--;
        //check that our UDP packet is at least 12 bytes (opus header) and smaller than our buffer size
        if(udp_buffer_size[udp_buffer_read_position] <= 12 || udp_buffer_size[udp_buffer_read_position] >= UDP_BUFFER_SIZE-12) {    
            udp_buffer_size[udp_buffer_read_position] = 0;
            udp_buffer_read_position++;
            udp_buffer_read_position%=NUM_UDP_BUFFERS;
            continue;
        };
        
        //read the sequence number from the Opus header
        uint16_t sequence = udp_buffers[udp_buffer_read_position][2]*256 + udp_buffers[udp_buffer_read_position][3];

        //this is the destination buffer for this packet
        int my_opus_buffer = sequence%NUM_OPUS_BUFFERS;

        if (sequence > opus_buffer_sequence[my_opus_buffer]) { //write newer packets into the buffer
            opus_buffer_size[my_opus_buffer] = udp_buffer_size[udp_buffer_read_position]-12;
            opus_buffer_sequence[my_opus_buffer] = sequence;
            memcpy(&(opus_buffer[my_opus_buffer]), &(udp_buffers[udp_buffer_read_position][12]), udp_buffer_size[udp_buffer_read_position]-12);

            //error correcting stuff
            // if (udp_buffer_available > 2) {
            //     if (-udp_buffer_available < offset) {
            //         Serial.print("-");
            //     }
            //     offset = _min(-udp_buffer_available,offset);
            //     good_cnt = -40-udp_buffer_available;
            // }
            // if (opus_buffer_available==0) {
            //     current_opus_sequence = _m ax(sequence-4,current_opus_sequence);
            // }
            opus_buffer_available++;
        } else {
            //this packet is old, let's ignore it
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

        //set opus_buffer_read if needed
        if (opus_buffer_read == -1) {
            opus_buffer_read = my_opus_buffer;
        }

        //clear this UDP buffer and increment the read position
        udp_buffer_size[udp_buffer_read_position] = 0;
        udp_buffer_read_position++;
        udp_buffer_read_position%=NUM_UDP_BUFFERS;

    }

}
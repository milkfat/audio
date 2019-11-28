
//AsyncUDP udp;
//AsyncUDP udp2;
WiFiUDP udp;

int32_t udp_packet_cnt;

void udp_listener_setup() {

    udp.begin(4322);

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
    

    // if(udp2.listen(1236)) {
    //     Serial.print("UDP Listening on IP: ");
    //     Serial.println(WiFi.localIP());
    //     udp2.onPacket([](AsyncUDPPacket packet) {
    //             //Serial.print("packet received");
    //             //Serial.print(" ");
    //             //Serial.write(packet.data(), packet.length());
    //             uint16_t sequence = *(uint8_t*)(packet.data()+2)*256 + *(uint8_t*)(packet.data()+3);
    //             packet_per_second++;
    //             if (sequence > sequence_max) {
    //                 sequence_max = sequence;
    //             } else {
    //                 out_of_order++;
    //             }

    //             int my_opus_buffer = sequence%NUM_OPUS_BUFFERS;
    //             int my_previous_opus_buffer = (sequence-1)%NUM_OPUS_BUFFERS;
    //             //Serial.print("sequence: ");
    //             //Serial.println(sequence);
    //             opus_buffer_size[my_opus_buffer] = packet.length()-12;
                
    //             memcpy8(&opus_buffer[my_opus_buffer], packet.data()+12, packet.length()-12);
    //             //Serial.println(sequence);
    //             opus_buffer_available++;
    //             opus_buffer_read = _max(opus_buffer_read,my_previous_opus_buffer);
    //     });
    // }



}

void udp_loop() {
    int cnt = 3;
    while (cnt--) {

        int message = udp.parsePacket();

        if (message) {
            uint8_t header[12];
            udp.read(header,12);
            uint16_t sequence = header[2]*256 + header[3];
            int my_opus_buffer = sequence%NUM_OPUS_BUFFERS;


            if (sequence > opus_buffer_sequence[my_opus_buffer]) { //write newer packets into the buffer
                opus_buffer_size[my_opus_buffer] = message-12;
                opus_buffer_sequence[my_opus_buffer] = sequence;
                udp.read(opus_buffer[my_opus_buffer], message-12);
                if (opus_buffer_available==0) {
                    current_opus_sequence = sequence;
                }
                opus_buffer_available++;
            } else {
                if (udp.available()) {
                    udp.flush();
                }
                continue;
            }
            
            if (udp.available()) {
                udp.flush();
            }

            int sequence_diff = sequence - sequence_max;

            if (sequence_diff > 1) {

                sequence_max = sequence;

            } else if (sequence_diff < -100) {
                sequence_max = 0;
                current_opus_sequence = 0;
            }

            //Serial.print("sequence: ");
            //Serial.println(sequence);
            //Serial.println(sequence);
            if (opus_buffer_read == -1) {
                opus_buffer_read = my_opus_buffer;
            }

            

        } else {
            break;
        }
    }
}
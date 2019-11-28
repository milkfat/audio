
//AsyncUDP udp;
//AsyncUDP udp2;
//WiFiUDP udp;
WiFiServer tcp(4321);

int32_t tcp_packet_cnt;

void tcp_listener_setup() {

    tcp.begin();

}

void tcp_loop() {
    static WiFiClient tcp_client = tcp.available();
    if (!tcp_client.connected()) {
        tcp_client = tcp.available();
        if (tcp_client) {
            Serial.println("CONNECTION!");
        }
    } else {
        if (tcp_client.available()) {
            static int sequence = 0;
            static int my_opus_buffer = 0;
            static uint8_t * my_opus_buffer_ptr = &opus_buffer[0][0];
            static int bytes_read = 0;
            static int segment_table = 0;
            static uint8_t page_segments[256] = {0};
            static uint16_t current_packet_size = 0;
            static uint8_t page_segment = 0;
            static uint8_t page_segment_new = 1;
            static uint8_t multipage = 0;

            while (tcp_client.available()) {
                uint8_t c = tcp_client.read();
                static uint8_t ogg_header_bytes = 0;
                static uint8_t ogg_read_header = 0;
                static uint16_t ogg_read_packet = 0;


                if (segment_table) { //read bytes from the segment table
                    //each byte represents the amount of data in each page segment (packet)
                    // Serial.print(c);
                    // Serial.print(", ");
                    page_segments[page_segment] = c;
                    segment_table--;
                    //tcp_client.read(&page_segments[page_segment+1], segment_table);  //read multiple bytes
                    //segment_table-=segment_table;
                    if (segment_table) {
                        page_segment++;
                    } else {
                        page_segment = 0;
                        //Serial.println();
                    }
                    
                    continue;
                } else if (page_segments[page_segment]) { //read bytes from each page segment (packet)
                    // Serial.print(c);
                    // Serial.print(" ");
                    if (page_segment_new) {

                        if (multipage) {//this page segment is a continuation of the previous page segment
                            multipage=0;
                            page_segment_new=0;
                        } else {//do some stuff in preparation for each new packet
                            current_packet_size=0;
                            sequence++;
                            my_opus_buffer = sequence%NUM_OPUS_BUFFERS;

                            my_opus_buffer_ptr = &opus_buffer[my_opus_buffer][0];
                            page_segment_new = 0;
                            // Serial.print(my_opus_buffer);
                            // Serial.print(" ");
                            // Serial.print(page_segments[page_segment]);
                            // Serial.print(" ");
                        }

                        if (page_segments[page_segment] == 255) {multipage=1;} //page segments >=255 will continue to the next page segment
                    }
                    
                    *my_opus_buffer_ptr = c;
                    my_opus_buffer_ptr++;
                    page_segments[page_segment]--;
                    current_packet_size++;

                    //tcp_client.read(my_opus_buffer_ptr, page_segments[page_segment]); //read multiple bytes
                    //my_opus_buffer_ptr+=page_segments[page_segment];
                    //page_segments[page_segment]-=page_segments[page_segment];



                    if (!page_segments[page_segment]) { //we have reached the end of this page segment

                        
                        page_segment++;
                        page_segment_new = 1;

                        if (multipage && page_segments[page_segment] == 0) {
                            multipage = 0;
                            page_segment++;
                        }

                        if (!multipage) {
                            opus_buffer_size[my_opus_buffer]=current_packet_size;
                            opus_buffer_sequence[my_opus_buffer]=sequence;
                            sequence_max = sequence;
                            if (opus_buffer_available==0) {
                                current_opus_sequence = sequence;
                            }
                            opus_buffer_available++;
                            // Serial.print("pack:");
                            // Serial.print(current_packet_size);
                            // Serial.print(" ");
                        }

                        //Serial.print(".");


                        // if (page_segments[page_segment]) {
                        //     Serial.println();
                        //     Serial.print("PACKET: ");
                        // }
                        continue;
                    }

                }

                if (ogg_read_header) {
                    //read bytes from the OGG packet header
                    // Serial.print(c);
                    // Serial.print(" ");
                    ogg_read_header--;
                    if (!ogg_read_header) {
                        //Serial.print(".");
                        segment_table = c;
                        // Serial.println();
                        // Serial.println(segment_table);
                        // Serial.println();
                        page_segment = 0;
                        // Serial.println(" ");
                        // Serial.print("PAGE SEGMENTS: ");
                        // Serial.println(segment_table);
                        // Serial.print("SEGMENT_TABLE: ");
                    }
                    continue;
                }


                //monitor the stream for the "OggS" header identifier
               
                switch (ogg_header_bytes) {
                    case 0:
                        if(c == 'O') {ogg_header_bytes++;}
                        break;
                    case 1:
                        (c == 'g' ? ogg_header_bytes++ : ogg_header_bytes = 0);
                        break;
                    case 2:
                        (c == 'g' ? ogg_header_bytes++ : ogg_header_bytes = 0);
                        break;
                    case 3:
                        if (c == 'S') {
                            ogg_read_header = 23;
                            // Serial.println();
                            // Serial.print("OGG HEADER: ");
                        }
                        ogg_header_bytes = 0;
                        
                        break;
                    default:
                        ogg_header_bytes = 0;
                        break;
                }
                
                //check for first byte of "OggS" header identifier
                
                //bytes_read++;
                //packet_per_second++;
                //if (tcp_client.peek() == magic_byte) {break;}

            }

            
        }
            
    }
}
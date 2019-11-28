#include "WiFi.h"
#include "WiFiUdp.h"

const char * ssid = "moonraker";
const char * password = "pastrylikesyou";


#include "arduino.h"
#include "driver/i2s.h"
#include <FastLED.h>

#include "filter.h"


int32_t cnt;
#include "i2s_stuff.h"
//#include "mp3_stuff.h"
#include "opus_stuff.h"
//#include "raw_pcm_stuff.h"
#include "udp_stuff.h"
#include "tcp_stuff.h"

void myloop();

void loop_task(void *pvParameters)
{
    for(;;) {
        myloop();
    }
}

void opus_tcp_task(void *pvParameters)
{
    for(;;) {
        tcp_loop();
        vTaskDelay(1);
    }
}

void setup()
{

    Serial.begin(115200);
    //mp3_setup();
    opus_setup();

    WiFi.mode(WIFI_STA);
    //WiFi.mode(WIFI_AP);
    WiFi.setSleep(false); //disable power saving mode to speed up response times
    //WiFi.softAP(ssid, password);
    delay(500);
    WiFi.begin(ssid, password);
    delay(500);
    int wifi_result = WiFi.waitForConnectResult();

    if (wifi_result != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        Serial.println(wifi_result);
        while(1) {
            delay(1000);
        }
    }
    Serial.println(WiFi.localIP());

    i2s_setup();
    
    udp_listener_setup();
    tcp_listener_setup();


    TaskHandle_t I2STask;
    xTaskCreatePinnedToCore(
      opus_i2s_task, /* Function to implement the task */
      "i2s_task", /* Name of the task */
      1000,  /* Stack size in bytes */
      NULL,  /* Task input parameter */
      1,  /* Priority of the task */
      &I2STask,  /* Task handle. */
      0); /* Core where the task should run */

TaskHandle_t TCPTask;
    xTaskCreatePinnedToCore(
      opus_tcp_task, /* Function to implement the task */
      "tcp_task", /* Name of the task */
      4000,  /* Stack size in bytes */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &TCPTask,  /* Task handle. */
      0); /* Core where the task should run */


    TaskHandle_t LoopTask;
    xTaskCreatePinnedToCore(
      loop_task, /* Function to implement the task */
      "loop_task", /* Name of the task */
      20000,  /* Stack size in bytes */
      NULL,  /* Task input parameter */
      3,  /* Priority of the task */
      &LoopTask,  /* Task handle. */
      1); /* Core where the task should run */


    vTaskDelete(NULL); 
}
void myloop() {
    static uint32_t time_asdf = 0;
    if (millis() > time_asdf) {
      //Serial.println(uxTaskGetNumberOfTasks());
      Serial.print("buf avail: ");
      Serial.println(opus_buffer_available);
    //   Serial.print("buf avail cnt: ");
    //   int temp_cnt = 0;
    //   for (int i = 0; i < NUM_OPUS_BUFFERS; i++) {
    //       Serial.print(opus_buffer_size[i]);
    //       Serial.print(" ");
    //       if (opus_buffer_size[i]>0) {
    //           temp_cnt++;
    //       }
    //   }
    //   Serial.println();
    //   Serial.println(temp_cnt);
      //Serial.print("pcm b avail: ");
      //Serial.println(pcm_bytes_available);
      Serial.print("seq max: ");
      Serial.println(sequence_max);
      Serial.print("pcm b writ: ");
      Serial.println(pcm_bytes_written);
      //Serial.print("bytes/s: ");
      //Serial.println(packet_per_second);
      Serial.print("z packet: ");
      Serial.println(zero_packet_total);
      Serial.print("stop: ");
      Serial.println(stop_cnt);
      Serial.print("opus error count: ");
      Serial.println(opus_error_cnt);
      Serial.print("current opus sequence: ");
      Serial.println(current_opus_sequence);
      Serial.print("max opus sequence: ");
      Serial.println(sequence_max);
      Serial.print("offset: ");
      Serial.println(offset);
      opus_buffer_max = 0;
      opus_buffer_min = 10000;
      udp_loop_time = 0;
      udp_loop_cnt = 0;
      opus_loop_time = 0;
      loop_task_time = 0;
      i2s_task_time = 0;
      pcm_bytes_written = 0;
      packet_per_second = 0;
    
        Serial.println(" ");
        time_asdf = millis()+1000;
    }
    if (Serial.available()) {
        uint8_t c = Serial.read();
        int current_gain;
        switch (c) {
            case 'u':
                opus_decoder_ctl(opusDecoder, OPUS_GET_GAIN(&current_gain));
                opus_decoder_ctl(opusDecoder, OPUS_SET_GAIN(current_gain+256));
                Serial.print(current_gain);
                Serial.print(" ");
                break;
            case 'd':
                opus_decoder_ctl(opusDecoder, OPUS_GET_GAIN(&current_gain));
                opus_decoder_ctl(opusDecoder, OPUS_SET_GAIN(current_gain-256));
                Serial.print(current_gain);
                Serial.print(" ");
                break;
            default:
                break;
        }
    }
    udp_loop();
    static int cnt = 0;


    opus_loop();

}
void loop()
{
    //mp3_loop();
    //raw_pcm_loop();

    // static uint32_t time1 = 0;
    // if (millis() > time1) {
    //   time1 = millis()+1000;
    //   //Serial.print(step);
    //   //Serial.print(" ");
    //   Serial.print(udp_packet_cnt);
    //   Serial.print(" ");
    //   Serial.print(buffer_available);
    //   Serial.print(" ");
    //   Serial.print(stop_cnt);
    //   Serial.print(" ");
    //   Serial.println(cnt);
    //   cnt = 0;
    //   udp_packet_cnt = 0;
    //   Serial.println("MP3 info:");
    //   Serial.println(mp3_info.frame_bytes);
    //   Serial.println(mp3_info.channels);
    //   Serial.println(mp3_info.hz);
    //   Serial.println(mp3_info.layer);
    //   Serial.println(mp3_info.bitrate_kbps);
    //   Serial.println();
    // }
    


}
void sin_loop() {
  static float freq = 440.f;
  float freq_samples = 44100.f/freq;
  uint16_t step = 65536.f/freq_samples;
  static int samples = 0;
  samples++;
  static uint32_t pos = 0;
  pos+=step;

  int32_t val = sin16(pos);
  val -= UINT16_MAX/2;
  val /= 32;
  val += UINT16_MAX/2;
  uint32_t data = val;
  data<<=16;

  static float freq2 = 440.f;
  float freq_samples2 = 44100.f/freq2;
  uint16_t step2 = 65536.f/freq_samples2;
  static int samples2 = 0;
  samples2++;
  static uint32_t pos2 = 0;
  pos2+=step2;

  int32_t val2 = sin16(pos2);
  val2 -= UINT16_MAX/2;
  val2 /= 32;
  val2 += UINT16_MAX/2;
  uint32_t data2 = val2;


  data += data2;
  i2s_write_sample_nb(data); 

  
  if (Serial.available()) {
    while (Serial.available()) {
      uint16_t myChar = Serial.read();
      //Serial.println(myChar);
      if (myChar == 65) {
        freq *= 1.05946274244;
      }
      if (myChar == 66) {
        freq *= 0.9438746262;
      }
      if (myChar == 67) {
        freq2 *= 1.05946274244;
      }
      if (myChar == 68) {
        freq2 *= 0.9438746262;
      }
    }
    Serial.println(freq);
  }
  
}

volatile int i2s_ready = 0;
volatile int i2s_stopped = 1;
volatile int stop_cnt = 0;

//i2s configuration 
#define I2S_NUM 0 // i2s port number
i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
     .sample_rate = 48000,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
     .dma_buf_count = 6,
     .dma_buf_len = 120,   //Interrupt level 1
     .use_apll = true
    };
    
i2s_pin_config_t pin_config = {
    .bck_io_num = 26, //this is BCK pin
    .ws_io_num = 25, // this is LRCK pin
    .data_out_num = 22, // this is DATA output pin
    .data_in_num = -1   //Not used
};

/* write bytes to I2S */
int i2s_write_nb(void * sample, uint32_t size){
  size_t bytes_written;
  i2s_write((i2s_port_t)I2S_NUM, (const char *)sample, size, &bytes_written, portMAX_DELAY);
  return bytes_written;
}

/* write sample to I2S */
int i2s_write_sample_nb(uint32_t sample){
  size_t bytes_written;
  return i2s_write((i2s_port_t)I2S_NUM, (const char *)&sample, 4, &bytes_written, portMAX_DELAY);
}

void i2s_setup() {

    //initialize i2s with configurations above
    i2s_driver_install((i2s_port_t)I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin((i2s_port_t)I2S_NUM, &pin_config);
    //set sample rates of i2s to sample rate of wav file
    i2s_set_sample_rates((i2s_port_t)I2S_NUM, 48000); 
    
    i2s_zero_dma_buffer((i2s_port_t)I2S_NUM);
    i2s_start((i2s_port_t)I2S_NUM);
  //i2s_driver_uninstall((i2s_port_t)I2S_NUM); //stop & destroy i2s driver 
}
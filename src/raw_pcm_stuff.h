
#define SBUFFER_SIZE (1472*10)
uint32_t sbuffer[SBUFFER_SIZE] = {0};
int32_t buffer_available = 0;

void raw_pcm_loop() {
    if (buffer_available && !i2s_stopped) {
        int cont = 1;
        while (buffer_available && cont) {
        static int buffer_position = 0;
        //uint32_t filtered_sample = filter_sample(sbuffer[buffer_position]);
        //cont = i2s_write_sample_nb(filtered_sample);
        cont = i2s_write_sample_nb(sbuffer[buffer_position]);
        buffer_position++;
        buffer_position %= SBUFFER_SIZE;
        buffer_available--;
        cnt++;
        }
    } else if (buffer_available>1472*8) {
        i2s_start((i2s_port_t)i2s_num);
        i2s_stopped = 0;
    } else if (!buffer_available && !i2s_stopped) {
        i2s_stop((i2s_port_t)i2s_num);
        i2s_stopped = 1;
        stop_cnt++;
    }
}
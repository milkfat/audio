ESP32 UDP/TCP streaming audio thing:

Currently limited to 2 channels of 16-bit 48000hz audio.

Supported audio coding formats:
    Opus
        RTP over UDP (low latency)
        OggOpus over TCP (high quality)
    MP3
        UDP
    PCM
        UDP


Some example ffmpeg commands:

Opus:
 UDP:
./ffmpeg -re -vn -i cool_music.mp3 -acodec libopus -frame_duration 10 -application lowdelay -b:a 256k -map 0:a -f rtp -rtpflags skip_rtcp "udp:192.168.1.100:1236"

 TCP:
./ffmpeg -re -vn -i cool_music.mp3 -acodec libopus -frame_duration 20 -application audio -b:a 192k -map 0:a -f ogg -page_duration 10000 "tcp:192.168.1.100:4321"


PCM:
./ffmpeg -re -vn -i cool_music.mp3 -ac 2 -ar 48000 -f s16le "udp:192.168.1.100:1234"


MP3
./ffmpeg -re -vn -i cool_music.mp3 -ac 2 -ar 48000 -f mp3 "udp://192.168.17.10:1235"


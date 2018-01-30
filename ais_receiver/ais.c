#include "ais_rx.h"

int main(void)
{
    bool repeated_message = false;
    ais_message_t ais[MAX_ALLOWABLE_MESSAGES];
    int fd1, message_count, last_count;
    do {
        sleep(1);
        fd1 = socket_init(51999);
    }while(fd1 == -1);
    printf("Connected!\n");
    load_configuration();
    printf("Total messages to be computed: %d\n", ais_config.total_messages);
    for(message_count = 0; message_count < ais_config.total_messages; message_count++) {
        ais[message_count].fd = fd1;
        ais[message_count].d.seqnr = 0;
        read_ais_message(&ais[message_count]);
        if(strlen(ais[message_count].d.nmea) > 0) {
            repeated_message = false;
            for(last_count = 0; last_count < message_count; last_count++) {
                if(strcmp(ais[message_count].d.nmea, ais[last_count].d.nmea) == 0) {
                    //printf("This message is repeated!\n");
                    message_count--;
                    repeated_message = true;
                    break;
                }
            }
            if(!repeated_message) {
                printf("Received AIS message: !%s\r\n", ais[message_count].d.nmea);
            }
        }
    }
}

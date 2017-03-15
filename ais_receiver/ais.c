#include "socket_utils.h"
#include "ais_rx.h"


int main(void)
{
    int fd1 = socket_init(51999);
    ais_message_t ais;
    ais.fd = fd1;
    ais.d.seqnr = 0;
    
    while(1){
        /* read up to 21 */
        read_ais_message(&ais);
    }
}
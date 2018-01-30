#ifndef __AIS_RX_H__
#define __AIS_RX_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ais_config.h"

#include "socket_utils.h"

#define     NCHK_LEN                 3
#define     MAX_AIS_PACKET_TYPE      4


#define     MAX_ALLOWABLE_MESSAGES   50

typedef struct demod_state_s{
    uint8_t     *rbuffer;
    char        nmea[128];
    int         seqnr;
}demod_state_t;

typedef struct ais_message_s{
    demod_state_t   d;
    int             fd;
    uint8_t         bytebuffer[128];
    uint8_t         byte_cnt;
    uint8_t         bitbuffer[1024];
    uint16_t        bit_cnt;
}ais_message_t;

int read_ais_message(ais_message_t *ais);

#endif

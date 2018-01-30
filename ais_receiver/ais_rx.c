#include "ais_rx.h"

void program_exit(char * str)
{
    printf("%s\r\n", str);
    exit(-1);
}


void unpack_byte(uint8_t * bit_array, uint8_t * byte_array, int bit_cnt, int byte_cnt)
{
    int j;
    for (j = 0; j < 8; j++){
        bit_array[bit_cnt+j] = (byte_array[byte_cnt]>>j)&0x1;
    }
}

unsigned long protodec_henten(int from, int size, unsigned char *frame)
{
    int i = 0;
    unsigned long tmp = 0;

    for (i = 0; i < size; i++)
        tmp |= (frame[from + i]) << (size - 1 - i);

    return tmp;
}

void protodec_generate_nmea(demod_state_t *d, int bufferlen, int fillbits)
{
    int senlen;
    int pos;
    int k, letter;
    int m;
    unsigned char sentences, sentencenum, nmeachk;
    char nchk[NCHK_LEN];
    int serbuffer_l;
    int ipcbuffer_l;

    //6bits to nmea-ascii. One sentence len max 82char
    //inc. head + tail.This makes inside datamax 62char multipart, 62 single
    senlen = 61;        //this is normally not needed.For testing only. May be fixed number
    if (bufferlen <= (senlen * 6)) {
        sentences = 1;
    } else {
        sentences = bufferlen / (senlen * 6);
        //sentences , if overflow put one more
        if (bufferlen % (senlen * 6) != 0)
            sentences++;
    };
    sentencenum = 0;
    pos = 0;
    do {
        k = 13;     //leave room for nmea header
        while (k < senlen + 13 && bufferlen > pos) {
            letter = protodec_henten(pos, 6, d->rbuffer);
            // 6bit-to-ascii conversion by IEC
            if (letter < 40)
                letter = letter + 48;
            else
                letter = letter + 56;
            d->nmea[k] = letter;
            pos += 6;
            k++;
        }

        //set nmea trailer with 00 checksum (calculate later)
        d->nmea[k] = 44;
        d->nmea[k + 1] = 48;
        d->nmea[k + 2] = 42;
        d->nmea[k + 3] = 48;
        d->nmea[k + 4] = 48;
        d->nmea[k + 5] = 0;
        sentencenum++;

        // printout one frame starts here
        //AIVDM,x,x,,, - header comes here first

        d->nmea[0] = 65;
        d->nmea[1] = 73;
        d->nmea[2] = 86;
        d->nmea[3] = 68;
        d->nmea[4] = 77;
        d->nmea[5] = 44;
        d->nmea[6] = 48 + sentences;
        d->nmea[7] = 44;
        d->nmea[8] = 48 + sentencenum;
        d->nmea[9] = 44;

        //if multipart message it needs sequential id number
        if (sentences > 1) {
            d->nmea[10] = d->seqnr + 48;
            d->nmea[11] = 44;
            d->nmea[12] = 44;
            //and if the last of multipart we need to show fillbits at trailer
            if (sentencenum == sentences) {
                d->nmea[k + 1] = 48 + fillbits;
            }
        } else {    //else put channel A & no seqnr to keep equal lenght (foo!)
            d->nmea[10] = 44;
            d->nmea[11] = 65;
            d->nmea[12] = 44;
        }
        //calculate xor checksum in hex for nmea[0] until nmea[m]='*'(42)
        nmeachk = d->nmea[0];
        m = 1;
        while (d->nmea[m] != 42) {  //!="*"
            nmeachk = nmeachk ^ d->nmea[m];
            m++;
        }
        // convert calculated checksum to 2 digit hex there are 00 as base
        // so if only 1 digit put it to later position to get 0-header 01,02...
        nchk[0] = 0;
        nchk[1] = 0;
        snprintf(nchk, NCHK_LEN, "%X", nmeachk);
        if (nchk[1] == 0) {
            d->nmea[k + 4] = nchk[0];
        } else {
            d->nmea[k + 3] = nchk[0];
            d->nmea[k + 4] = nchk[1];
        }
        //In final. Add header "!" and trailer <cr><lf>
        // here it could be sent to /dev/ttySx
    } while (sentencenum < sentences);
}


static void remove_trailing_spaces(char *s, int len)
{
    int i;

    s[len] = 0;
    for (i = len-1; i >= 0; i--) {
        if (s[i] == ' ' || s[i] == 0)
            s[i] = 0;
        else
            i = -1;
    }
}

/*
 *  Decode 6-bit ASCII to normal 8-bit ASCII
 */

void protodec_decode_sixbit_ascii(char sixbit, char *name, int pos)
{
    if (sixbit >= 1 && sixbit <= 31) {
        name[pos] = sixbit + 64;
        return;
    }

    if (sixbit >= 32 && sixbit <= 63) {
        name[pos] = sixbit;
        return;
    }

    name[pos] = ' ';
}

/*
 *  decode position packets (types 1,2,3)
 */

void protodec_pos(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    int longitude, latitude;
    unsigned short course, sog, heading;
    char rateofturn, navstat;

    longitude = protodec_henten(61, 28, d->rbuffer);
    if (((longitude >> 27) & 1) == 1)
        longitude |= 0xF0000000;

    latitude = protodec_henten(38 + 22 + 29, 27, d->rbuffer);
    if (((latitude >> 26) & 1) == 1)
        latitude |= 0xf8000000;

    course = protodec_henten(38 + 22 + 28 + 28, 12, d->rbuffer);
    sog = protodec_henten(50, 10, d->rbuffer);
    rateofturn = protodec_henten(38 + 2, 8, d->rbuffer);
    navstat = protodec_henten(38, 2, d->rbuffer);
    heading = protodec_henten(38 + 22 + 28 + 28 + 12, 9, d->rbuffer);

    /*printf(" lat %.6f lon %.6f course %.0f speed %.1f rateofturn %d navstat %d heading %d\n",
        (float) latitude / 600000.0,
        (float) longitude / 600000.0,
        (float) course / 10.0, (float) sog / 10.0,
        rateofturn, navstat, heading);*/
}

void protodec_4(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    unsigned long day, hour, minute, second, year, month;
    int longitude, latitude;
    float longit, latit;

    year = protodec_henten(40, 12, d->rbuffer);
    month = protodec_henten(52, 4, d->rbuffer);
    day = protodec_henten(56, 5, d->rbuffer);
    hour = protodec_henten(61, 5, d->rbuffer);
    minute = protodec_henten(66, 6, d->rbuffer);
    second = protodec_henten(72, 6, d->rbuffer);

    longitude = protodec_henten(79, 28, d->rbuffer);
    if (((longitude >> 27) & 1) == 1)
        longitude |= 0xF0000000;
    longit = ((float) longitude) / 10000.0 / 60.0;

    latitude = protodec_henten(107, 27, d->rbuffer);
    if (((latitude >> 26) & 1) == 1)
        latitude |= 0xf8000000;
    latit = ((float) latitude) / 10000.0 / 60.0;

    /*printf(" date %ld-%ld-%ld time %02ld:%02ld:%02ld lat %.6f lon %.6f\n",
        year, month, day, hour, minute,
        second, latit, longit);*/
}

void protodec_5(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    int pos;
    unsigned long imo;
    char callsign[7];
    char name[21];
    char destination[21];
    unsigned int A, B;
    unsigned char C, D;
    unsigned char draught;
    int k;
    int letter;
    unsigned int shiptype;

    /* get IMO number */
    imo = protodec_henten(40, 30, d->rbuffer);
    //printf("--- 5: mmsi %lu imo %lu\n", mmsi, imo);

    /* get callsign */
    pos = 70;
    for (k = 0; k < 6; k++) {
        letter = protodec_henten(pos, 6, d->rbuffer);
        protodec_decode_sixbit_ascii(letter, callsign, k);
        pos += 6;
    }

    callsign[6] = 0;
    remove_trailing_spaces(callsign, 6);

    /* get name */
    pos = 112;
    for (k = 0; k < 20; k++) {
        letter = protodec_henten(pos, 6, d->rbuffer);
        protodec_decode_sixbit_ascii(letter, name, k);
        pos += 6;
    }
    name[20] = 0;
    remove_trailing_spaces(name, 20);

    /* get destination */
    pos = 120 + 106 + 68 + 8;
    for (k = 0; k < 20; k++) {
        letter = protodec_henten(pos, 6, d->rbuffer);
        protodec_decode_sixbit_ascii(letter, destination, k);
        pos += 6;
    }
    destination[20] = 0;
    remove_trailing_spaces(destination, 20);

    /* type of ship and cargo */
    shiptype = protodec_henten(232, 8, d->rbuffer);

    /* dimensions and reference GPS position */
    A = protodec_henten(240, 9, d->rbuffer);
    B = protodec_henten(240 + 9, 9, d->rbuffer);
    C = protodec_henten(240 + 9 + 9, 6, d->rbuffer);
    D = protodec_henten(240 + 9 + 9 + 6, 6, d->rbuffer);
    draught = protodec_henten(294, 8, d->rbuffer);
    // printf("Length: %d\nWidth: %d\nDraught: %f\n",A+B,C+D,(float)draught/10);

    printf(" name \"%s\" destination \"%s\" type %d length %d width %d draught %.1f",
        name, destination, shiptype,
        A + B, C + D,
        (float) draught / 10.0);
}

/*
 *  6: addressed binary message
 */

void protodec_6(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    int sequence = protodec_henten(38, 2, d->rbuffer);
    unsigned long dst_mmsi = protodec_henten(40, 30, d->rbuffer);
    int retransmitted = protodec_henten(70, 1, d->rbuffer);
    int appid = protodec_henten(72, 16, d->rbuffer);
    int appid_dac = protodec_henten(72, 10, d->rbuffer);
    int appid_fi = protodec_henten(82, 6, d->rbuffer);

    printf(" dst_mmsi %09ld seq %d retransmitted %d appid %d app_dac %d app_fi %d",
        dst_mmsi, sequence, retransmitted, appid, appid_dac, appid_fi);

}

/*
 *  7: Binary acknowledge
 *  13: Safety related acknowledge
 */

void protodec_7_13(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    unsigned long dst_mmsi;
    int sequence;
    int i;
    int pos;

    pos = 40;

    printf(" buflen %d pos+32 %d", bufferlen, pos + 32);
    for (i = 0; i < 4 && pos + 32 <= bufferlen; pos += 32) {
        dst_mmsi = protodec_henten(pos, 30, d->rbuffer);
        sequence = protodec_henten(pos + 30, 2, d->rbuffer);

        printf(" ack %d (to %09ld seq %d)",
            i+1, dst_mmsi, sequence);
        i++;
    }
}

/*
 *  8: Binary broadcast
 */

void protodec_8(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    int appid = protodec_henten(40, 16, d->rbuffer);
    int appid_dac = protodec_henten(40, 10, d->rbuffer);
    int appid_fi = protodec_henten(50, 6, d->rbuffer);

    printf(" appid %d app_dac %d app_fi %d", appid, appid_dac, appid_fi);
}

void protodec_18(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    int longitude, latitude;
    unsigned short course, sog, heading;
    char rateofturn, navstat;

    longitude = protodec_henten(57, 28, d->rbuffer);
    if (((longitude >> 27) & 1) == 1)
        longitude |= 0xF0000000;

    latitude = protodec_henten(85, 27, d->rbuffer);
    if (((latitude >> 26) & 1) == 1)
        latitude |= 0xf8000000;

    course = protodec_henten(112, 12, d->rbuffer);
    sog = protodec_henten(46, 10, d->rbuffer);

    rateofturn = 0; //NOT in B
    navstat = 15;   //NOT in B

    heading = protodec_henten(124, 9, d->rbuffer);
    printf(" lat %.6f lon %.6f course %.0f speed %.1f rateofturn %d navstat %d heading %d\n",
        (float) latitude / 600000.0,
        (float) longitude / 600000.0,
        (float) course / 10.0, (float) sog / 10.0,
        rateofturn, navstat, heading);
}

void protodec_19(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    int pos, k;
    unsigned int A, B;
    unsigned char C, D;
    unsigned int shiptype;
    int letter;
    char name[21];
    /*
     * Class B does not have destination, use "CLASS B" instead
     * (same as ShipPlotter)
     */
    char destination[21] = "CLASS B";

    /* get name */
    pos = 143;
    for (k = 0; k < 20; k++) {
        letter = protodec_henten(pos, 6, d->rbuffer);
        protodec_decode_sixbit_ascii(letter, name, k);
        pos += 6;
    }
    name[20] = 0;
    remove_trailing_spaces(name, 20);
    //printf("Name: '%s'\n", name);

    /* type of ship and cargo */
    shiptype = protodec_henten(263, 8, d->rbuffer);

    /* dimensions and reference GPS position */
    A = protodec_henten(271, 9, d->rbuffer);
    B = protodec_henten(271 + 9, 9, d->rbuffer);
    C = protodec_henten(271 + 9 + 9, 6, d->rbuffer);
    D = protodec_henten(271 + 9 + 9 + 6, 6, d->rbuffer);

    // printf("Length: %d\nWidth: %d\n",A+B,C+D);
    //printf("%09ld %d %d %f", mmsi, A + B, C + D);
    printf(" name \"%s\" type %d length %d  width %d\n", name, shiptype, A+B, C+D);
}

void protodec_20(demod_state_t *d, int bufferlen)
{
    int ofs, slots, timeout, incr;
    int i;
    int pos;

    pos = 40;

    for (i = 0; i < 4 && pos + 30 < bufferlen; pos += 30) {
        ofs = protodec_henten(pos, 12, d->rbuffer);
        slots = protodec_henten(pos + 12, 4, d->rbuffer);
        timeout = protodec_henten(pos + 12 + 4, 3, d->rbuffer);
        incr = protodec_henten(pos + 12 + 4 + 3, 11, d->rbuffer);

        printf(" reserve %d (ofs %d slots %d timeout %d incr %d)",
            i+1, ofs, slots, timeout, incr);
        i++;
    }
}

void protodec_24(demod_state_t *d, int bufferlen, unsigned long mmsi)
{
    int partnr;
    int pos;
    int k, letter;
    unsigned int A, B;
    unsigned char C, D;
    unsigned int shiptype;
    char name[21];
    char callsign[7];
    /*
     * Class B does not have destination, use "CLASS B" instead
     * (same as ShipPlotter)
     */
    const char destination[21] = "CLASS B";

    /* resolve type 24 frame's part A or B */
    partnr = protodec_henten(38, 2, d->rbuffer);

    //printf("(partnr %d type %d): ",partnr, type);
    if (partnr == 0) {
        //printf("(Now in name:partnr %d type %d): ",partnr, type);
        /* get name */
        pos = 40;
        for (k = 0; k < 20; k++) {
            letter = protodec_henten(pos, 6, d->rbuffer);
            protodec_decode_sixbit_ascii(letter, name, k);
            pos += 6;
        }

        name[20] = 0;
        remove_trailing_spaces(name, 20);

        printf(" name \"%s\"", name);

    }

    if (partnr == 1) {
        //printf("(Now in data:partnr %d type %d): ",partnr, type);
        /* get callsign */
        pos = 90;
        for (k = 0; k < 6; k++) {
            letter = protodec_henten(pos, 6, d->rbuffer);
            protodec_decode_sixbit_ascii(letter, callsign, k);
            pos += 6;
        }
        callsign[6] = 0;
        remove_trailing_spaces(callsign, 6);

        /* type of ship and cargo */
        shiptype = protodec_henten(40, 8, d->rbuffer);

        /* dimensions and reference GPS position */
        A = protodec_henten(132, 9, d->rbuffer);
        B = protodec_henten(132 + 9, 9, d->rbuffer);
        C = protodec_henten(132 + 9 + 9, 6, d->rbuffer);
        D = protodec_henten(132 + 9 + 9 + 6, 6, d->rbuffer);

        printf(" callsign \"%s\" type %d length %d width %d",
            callsign, shiptype, A+B, C+D);
    }
}


void protodec_getdata(int bufferlen, demod_state_t *d)
{
    unsigned char type = protodec_henten(0, 6, d->rbuffer);
    if (type < 1 || type > MAX_AIS_PACKET_TYPE /* 4 */)
        return;
    unsigned long mmsi = protodec_henten(8, 30, d->rbuffer);
    int fillbits = 0;
    int k;

    if (bufferlen % 6 > 0) {
        fillbits = 6 - (bufferlen % 6);
        for (k = bufferlen; k < bufferlen + fillbits; k++)
            d->rbuffer[k] = 0;

        bufferlen = bufferlen + fillbits;
    }

    /* generate an NMEA string out of the binary packet */
    protodec_generate_nmea(d, bufferlen, fillbits);

    //multipart message ready. Increase seqnr for next one
    //rolling 1-9. Single msg ready may also increase this, no matter.
    d->seqnr++;
    if (d->seqnr > 9)
        d->seqnr = 0;

    if (type < 1 || type > MAX_AIS_PACKET_TYPE)
        return; // unsupported packet type

    //printf("type %d mmsi %09ld:", type, mmsi);

    switch (type) {
    case 1: /* position packets */
    case 2:
    case 3:
        protodec_pos(d, bufferlen, mmsi);
        break;

    case 4: /* base station position */
        protodec_4(d, bufferlen, mmsi);
        break;

    case 5: /* vessel info */
        protodec_5(d, bufferlen, mmsi);
        break;

    case 6: /* Addressed binary message */
        protodec_6(d, bufferlen, mmsi);
        break;

    case 7: /* Binary acknowledge */
    case 13: /* Safety related acknowledge */
        protodec_7_13(d, bufferlen, mmsi);
        break;

    case 8: /* Binary broadcast message */
        protodec_8(d, bufferlen, mmsi);
        break;

    case 18: /* class B transmitter position report */
        protodec_18(d, bufferlen, mmsi);
        break;

    case 19: /* class B transmitter vessel info */
        protodec_19(d, bufferlen, mmsi);
        break;

    case 24: /* class B transmitter info */
        protodec_24(d, bufferlen, mmsi);
        break;

    case 20:
        protodec_20(d, bufferlen);
        break;

    default:
        break;
    }
}

int read_ais_message(ais_message_t *ais)
{
    int readed;
    readed = read_kiss_from_socket(ais->fd, ais->bytebuffer);
    if ( ! (readed > 0) ){
        program_exit("file descriptor returned error");
    }else{
        /* put each byte sepparatelly */
        memset(ais->bitbuffer, 0, sizeof(ais->bitbuffer));
        ais->bit_cnt = 0;
        for (ais->byte_cnt = 0; ais->byte_cnt < readed; ais->byte_cnt++){
            unpack_byte(ais->bitbuffer, ais->bytebuffer, ais->bit_cnt, ais->byte_cnt);
            ais->bit_cnt += 8;
        }
        /* tell who are you */
        ais->d.rbuffer = ais->bitbuffer;
        protodec_getdata(ais->bit_cnt, &ais->d);
    }
}

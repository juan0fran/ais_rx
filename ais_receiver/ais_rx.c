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
    printf("NMEA: %d sentences with max data of %d ascii chrs\n", sentences, senlen);
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
        printf("NMEA: Drop from loop with k:%d pos:%d senlen:%d bufferlen: %d\n",
            k, pos, senlen, bufferlen);
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
            printf("NMEA: It is multipart (%d/%d), add sequence number (%d) to header\n",
                sentences, sentencenum, d->seqnr);
            d->nmea[10] = d->seqnr + 48;
            d->nmea[11] = 44;
            d->nmea[12] = 44;
            //and if the last of multipart we need to show fillbits at trailer
            if (sentencenum == sentences) {
                printf("NMEA: It is last of multipart (%d/%d), add fillbits (%d) to trailer\n",
                    sentences, sentencenum, fillbits);
                d->nmea[k + 1] = 48 + fillbits;
            }
        } else {    //else put channel A & no seqnr to keep equal lenght (foo!)
            d->nmea[10] = 44;
            d->nmea[11] = 65;
            d->nmea[12] = 44;
        }

        //strcpy(nmea,"!AIVDM,1,1,,,");
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
        printf("!%s\r\n", d->nmea);
    } while (sentencenum < sentences);
}

int read_ais_message(ais_message_t * ais)
{
    int readed;
    readed = read_kiss_from_socket(ais->fd, ais->bytebuffer);
    if ( ! (readed > 0) ){
        program_exit("file descriptor returned error");
    }else{
        if (readed <= 21){
            printf("Readed %d bytes from AIS\r\n", readed);
            /* put each byte sepparatelly */
            memset(ais->bitbuffer, 0, sizeof(ais->bitbuffer));
            ais->bit_cnt = 0;
            for (ais->byte_cnt = 0; ais->byte_cnt < readed; ais->byte_cnt++){
                unpack_byte(ais->bitbuffer, ais->bytebuffer, ais->bit_cnt, ais->byte_cnt);
                ais->bit_cnt += 8;
            }
            /* tell who are you */
            ais->d.rbuffer = ais->bitbuffer;
            protodec_generate_nmea(&ais->d, ais->bit_cnt, (168 - ais->bit_cnt)) ;
            ais->d.seqnr++;
        }else{
            printf("Bad length: %d\r\n", readed);
        }
    }
}
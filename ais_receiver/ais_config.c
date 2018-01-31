/***************************************************************************************************
*  File:        ais_config.c                                                                       *
*  Authors:     Noemí Miguélez (NM)                                                                *
*               Joan Francesc Muñoz Martin (JFM)                                                   *
*                                                                                                  *
*  Creation:    28-01-2018                                                                         *
*  Description: AisConfiguration file reader and parser (source)                                   *
*                                                                                                  *
*  This file is part of a project developed by Nano-Satellite and Payload Laboratory (NanoSat Lab) *
*  at Technical University of Catalonia - UPC BarcelonaTech.                                       *
*                                                                                                  *
*  AIS configuration file reader. Reads a config file from a path and it sets AISConfiguration     *
*                                                                                                  *
* ------------------------------------------------------------------------------------------------ *
*  Changelog:                                                                                      *
*  v#   Date            Author  Description                                                        *
*  0.1  28-01-2018      <NM>   <First version>                                                     *
*  0.2  30-01-2018      <JFM>  <File path is an input now>                                         *
***************************************************************************************************/

#include "ais_config.h"

FILE *fd;
char line[256];
char key[256];
char value[256];
int match;
int i;

static const char *JSON_STRING;

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}


void load_configuration(const char *path, AISConfiguration *ais_config)
{
    int i;
    int r;
    unsigned int length;
    jsmn_parser p;
    jsmntok_t t[128];
    ais_config->total_messages = 15;
    if(path == NULL) {
        return;
    }
    fd = fopen(path, "rt");
    if(fd == NULL) {
        return;
    }
    JSON_STRING=fgets(line, sizeof(line), fd);

    jsmn_init(&p);
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));
    if (r < 0) {
        printf("Failed to parse JSON: %d\n", r);
    }


    if (r < 1 || t[0].type != JSMN_OBJECT) {
        printf("Object expected\n");
    }


    for (i = 1; i < r; i++) {
        if (jsoneq(JSON_STRING, &t[i], "total_messages") == 0) {

            length = t[i+1].end - t[i+1].start;
            char keyString[length + 1];
            memcpy(keyString, &JSON_STRING[t[i+1].start], length);
            keyString[length] = '\0';
            ais_config->total_messages=atoi(keyString);
            fclose(fd);
            return;
            i++;

        } else {
            printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
                    JSON_STRING + t[i].start);
        }
    }

    fclose(fd);
}

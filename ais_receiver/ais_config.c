#include "ais_config.h"
#include "jsmn.h"

AISConfiguration ais_config;
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


void load_configuration(void)
{
    int i;
    int r;
    unsigned int length;
    jsmn_parser p;
    jsmntok_t t[128];
    ais_config.total_messages = 15;
    
    fd = fopen(CONFIG_FILE_SYSTEM, "rt");
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
            ais_config.total_messages=atoi(keyString);


            i++;

        } else {
            printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
                    JSON_STRING + t[i].start);
        }
    }

    fclose(fd);
}

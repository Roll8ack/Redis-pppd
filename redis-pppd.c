#include <pppd/pppd.h>
#include <pppd/chap-new.h>
#include <hiredis/hiredis.h>
#include <string.h>
#include <stdlib.h>

#define COMMAND "GET com.example.pppd."

char pppd_version[] = VERSION;

char *redisHost = NULL;
char *redisPort = NULL;

static option_t options[] = {
    { "redis_host", o_string, &redisHost, "Set Redis Server Ip Address" },
    { "redis_port", o_string, &redisPort, "Set Redis Server Port" },
    { NULL }
};

redisContext *context = NULL;

static int connectRedis (const char* ipAddress, int port) {
    context = redisConnect(ipAddress, port);

    if (context->err) {
        error("Connection error: %s\n", context->errstr);
        return 1;
    }
    return 0;
}

static void freeRedis (void *opaque, int arg) {
    redisFree(context);
    error("Redis context freed.\n");
}

static int pppdChapCheck (void) { return 1; }

static int pppdChapVerify (char *user, char *ourname, int id,
                           struct chap_digest_type *digest,
                           unsigned char *challenge, unsigned char *response,
                           char *message, int message_space) {
    char command [160] = COMMAND;
    char *p = command;
    while (*p) p ++;
    char *q = user;
    while (*q) {
        *p = *q;
        p ++; q ++;
    }

    redisReply *reply = redisCommand(context, command);

    if (reply->type == REDIS_REPLY_NIL) {
        error("User<%s> doesn't exist.\n", user);
        return 0;
    }
    else if (reply->type != REDIS_REPLY_STRING) {
        error("Bad value in redis key: %s.\n", command[4]);
    }
    else if (digest->verify_response(id, user, reply->str, strlen(reply->str),
                                     challenge, response, message, message_space)) {
        return 1;
    }
    else {
        error("Wrong password.\n");
        return 0;
    }
}

static int checkAddressAllowed (unsigned int addr) { return 1; }

void plugin_init(void) {
    chap_check_hook = pppdChapCheck;
    chap_verify_hook = pppdChapVerify;
    allowed_address_hook = checkAddressAllowed;

    if (connectRedis(redisHost, (uint32_t)atoi(redisPort))) {
        error("Failed to load plugin redis-pppd.\n");
        exit(1);
    }

    add_notifier(&exitnotify, freeRedis, NULL);
}

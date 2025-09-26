/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

/**
 * Adding Variables to a Server
 * ----------------------------
 *
 * This tutorial shows how to work with data types and how to add variable nodes
 * to a server. First, we add a new variable to the server. Take a look at the
 * definition of the ``UA_VariableAttributes`` structure to see the list of all
 * attributes defined for VariableNodes.
 *
 * Note that the default settings have the AccessLevel of the variable value as
 * read only. See below for making the variable writable.
 */

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <stdio.h>

static char appURI[255];
static char appName[255];
static char serverCaps[255];
static char port[6];
static bool di = false;

/**
 * Now we change the value with the write service. This uses the same service
 * implementation that can also be reached over the network by an OPC UA client.
 */

static void
writeServiceLevel(UA_Server *server) {
    UA_NodeId myServiceLevelNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVICELEVEL);
    UA_Int32 myLevel = 255;
    UA_Variant serviceLevel;
    UA_Variant_init(&serviceLevel);
    UA_Variant_setScalar(&serviceLevel, &myLevel, &UA_TYPES[UA_TYPES_BYTE]);
    UA_Server_writeValue(server, myServiceLevelNodeId, serviceLevel);
}

static void
writeServerCaps(UA_Server *server) {
    //   splitCaps();
    UA_NodeId myServerCapsNodeId =
        UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERCONFIGURATION_SERVERCAPABILITIES);
    UA_Variant serverCapsVariant;
    UA_Variant_init(&serverCapsVariant);
    const char *delim = ":";
    int len = strcspn(serverCaps, delim);

    UA_String *serverCapabilities =
        (UA_String *)UA_Array_new(len, &UA_TYPES[UA_TYPES_STRING]);
    int count = 0;
    char *token2 = strtok(serverCaps, delim);
    while(token2 != NULL) {
        serverCapabilities[count] = UA_String_fromChars(token2);
        count++;
        printf("%s\n", token2);
        token2 = strtok(NULL, delim);
    }
    UA_Variant_setArray(&serverCapsVariant, serverCapabilities, count,
                        &UA_TYPES[UA_TYPES_STRING]);
    UA_Server_writeValue(server, myServerCapsNodeId, serverCapsVariant);
}

static void
addDINamespace(UA_Server *server) {
    UA_Server_addNamespace(server, "http://opcfoundation.org/UA/DI/");
}
/** It follows the main server code, making use of the above definitions. */

static int
parseArguments(int argc, char *argv[]) {
    /*
     * Read through command-line arguments for options.
     */
    for(int i = 1; i < argc; i += 2) {
        printf("argv[%u] = %s\n", i, argv[i]);
        if(argv[i][0] == '-') {
            if(argv[i][1] == 'a') {
                if(argv[i][2] == 'u') {
                    strcpy(appURI, argv[i + 1]);
                    printf("argv[%u] = %s\n", i + 1, appURI);
                } else if(argv[i][2] == 'n') {
                    strcpy(appName, argv[i + 1]);
                    printf("argv[%u] = %s\n", i + 1, appName);
                }
            } else if(argv[i][1] == 'c') {
                strcpy(serverCaps, argv[i + 1]);
                printf("argv[%u] = %s\n", i + 1, serverCaps);
            } else if(argv[i][1] == 'p') {
                strcpy(port, argv[i + 1]);
                printf("argv[%u] = %s\n", i + 1, port);
            } else if(argv[i][1] == 'd') {
                if(strcmp("ON", argv[i + 1]) == 0)
                    di = true;
                printf("di_ns = %i\n", di);
            } else {
                printf("Invalid option.");
                return 2;
            }
        } else {
            // fp1(argv[i]);
            // fp2(argv[i][1]);
        }
    }
    return 0;
}

int
main(int argc, char *argv[]) {
    if(argc == 1) {
        printf("No options found.");
        return -1;
    }
    if(argc > 11) {
        printf("To many options found.");
        return -1;
    }
    if(parseArguments(argc, argv) > 0) {
        printf("Invalid options.");
        return -1;
    }

    UA_ServerConfig initialConfig;
    memset(&initialConfig, 0, sizeof(UA_ServerConfig));
    UA_ServerConfig_setMinimal(&initialConfig, atoi(port), NULL);

    initialConfig.applicationDescription.applicationUri = UA_String_fromChars(appURI);
    initialConfig.applicationDescription.applicationName =
        UA_LOCALIZEDTEXT_ALLOC("de", appName);
    UA_Server *server = UA_Server_newWithConfig(&initialConfig);

    // UA_Server *server = UA_Server_new();
    // UA_ServerConfig *pConfig = UA_Server_getConfig(server);
    // UA_ServerConfig_setMinimal(pConfig, atoi(port), 0);
    writeServerCaps(server);
    if(di)
        addDINamespace(server);
    writeServiceLevel(server);
    // writeWrongVariable(server);

    UA_Server_runUntilInterrupt(server);
    UA_Server_delete(server);
    return 0;
}

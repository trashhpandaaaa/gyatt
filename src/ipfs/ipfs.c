#include "ipfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Because we're too cool for centralized servers 😎

// Callback for writing HTTP response data
struct memory_struct {
    char *memory;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory_struct *mem = (struct memory_struct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Not enough memory to store IPFS response\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

ipfs_client_t* ipfs_client_init(const char *host, int port) {
    ipfs_client_t *client = malloc(sizeof(ipfs_client_t));
    if (!client) return NULL;

    if (host) {
        strncpy(client->host, host, sizeof(client->host) - 1);
        client->host[sizeof(client->host) - 1] = '\0';
    } else {
        strcpy(client->host, IPFS_DEFAULT_HOST);
    }

    client->port = (port > 0) ? port : IPFS_DEFAULT_PORT;
    client->timeout_ms = 10000; // 10 seconds default timeout

    // Initialize curl globally (once)
    static int curl_initialized = 0;
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = 1;
    }

    return client;
}

void ipfs_client_free(ipfs_client_t *client) {
    if (client) {
        free(client);
    }
}

bool ipfs_is_online(ipfs_client_t *client) {
    CURL *curl;
    CURLcode res;
    char url[512];
    struct memory_struct chunk = {0};

    snprintf(url, sizeof(url), "http://%s:%d%s/version", 
             client->host, client->port, IPFS_API_PATH);

    curl = curl_easy_init();
    if (!curl) return false;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

    res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    free(chunk.memory);

    return (res == CURLE_OK && http_code == 200);
}

char* ipfs_version(ipfs_client_t *client) {
    CURL *curl;
    CURLcode res;
    char url[512];
    struct memory_struct chunk = {0};

    snprintf(url, sizeof(url), "http://%s:%d%s/version", 
             client->host, client->port, IPFS_API_PATH);

    curl = curl_easy_init();
    if (!curl) return NULL;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        free(chunk.memory);
        return NULL;
    }

    return chunk.memory; // Caller must free
}

char* ipfs_add(ipfs_client_t *client, const void *data, size_t size) {
    CURL *curl;
    CURLcode res;
    char url[512];
    struct memory_struct chunk = {0};
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    snprintf(url, sizeof(url), "http://%s:%d%s/add?pin=true", 
             client->host, client->port, IPFS_API_PATH);

    curl = curl_easy_init();
    if (!curl) return NULL;

    chunk.memory = malloc(1);
    chunk.size = 0;

    // Add the data as a file upload
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_BUFFER, "data",
                 CURLFORM_BUFFERPTR, data,
                 CURLFORM_BUFFERLENGTH, size,
                 CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);

    res = curl_easy_perform(curl);
    
    curl_easy_cleanup(curl);
    curl_formfree(formpost);

    if (res != CURLE_OK) {
        free(chunk.memory);
        return NULL;
    }

    // Parse JSON response to extract CID
    // Response format: {"Name":"data","Hash":"QmXXX...","Size":"123"}
    char *hash_start = strstr(chunk.memory, "\"Hash\":\"");
    if (!hash_start) {
        free(chunk.memory);
        return NULL;
    }
    hash_start += 8; // Skip past "Hash":"

    char *hash_end = strchr(hash_start, '"');
    if (!hash_end) {
        free(chunk.memory);
        return NULL;
    }

    size_t cid_len = hash_end - hash_start;
    char *cid = malloc(cid_len + 1);
    if (!cid) {
        free(chunk.memory);
        return NULL;
    }

    memcpy(cid, hash_start, cid_len);
    cid[cid_len] = '\0';

    free(chunk.memory);
    return cid; // Caller must free
}

ipfs_response_t* ipfs_cat(ipfs_client_t *client, const char *cid) {
    CURL *curl;
    CURLcode res;
    char url[768];
    struct memory_struct chunk = {0};

    snprintf(url, sizeof(url), "http://%s:%d%s/cat?arg=%s", 
             client->host, client->port, IPFS_API_PATH, cid);

    curl = curl_easy_init();
    if (!curl) return NULL;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

    res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);

    ipfs_response_t *response = malloc(sizeof(ipfs_response_t));
    if (!response) {
        free(chunk.memory);
        return NULL;
    }

    response->data = chunk.memory;
    response->size = chunk.size;
    response->status_code = http_code;
    response->error = (res != CURLE_OK) ? strdup(curl_easy_strerror(res)) : NULL;

    return response; // Caller must free with ipfs_response_free
}

bool ipfs_pin_add(ipfs_client_t *client, const char *cid) {
    CURL *curl;
    CURLcode res;
    char url[768];
    struct memory_struct chunk = {0};

    snprintf(url, sizeof(url), "http://%s:%d%s/pin/add?arg=%s", 
             client->host, client->port, IPFS_API_PATH, cid);

    curl = curl_easy_init();
    if (!curl) return false;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

    res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    free(chunk.memory);

    return (res == CURLE_OK && http_code == 200);
}

bool ipfs_pin_rm(ipfs_client_t *client, const char *cid) {
    CURL *curl;
    CURLcode res;
    char url[768];
    struct memory_struct chunk = {0};

    snprintf(url, sizeof(url), "http://%s:%d%s/pin/rm?arg=%s", 
             client->host, client->port, IPFS_API_PATH, cid);

    curl = curl_easy_init();
    if (!curl) return false;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

    res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_easy_cleanup(curl);
    free(chunk.memory);

    return (res == CURLE_OK && http_code == 200);
}

char** ipfs_pin_ls(ipfs_client_t *client, size_t *count) {
    CURL *curl;
    CURLcode res;
    char url[512];
    struct memory_struct chunk = {0};

    snprintf(url, sizeof(url), "http://%s:%d%s/pin/ls?type=recursive", 
             client->host, client->port, IPFS_API_PATH);

    curl = curl_easy_init();
    if (!curl) return NULL;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, client->timeout_ms);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        free(chunk.memory);
        *count = 0;
        return NULL;
    }

    // Simple JSON parsing for CIDs (response format: {"Keys":{"QmXXX":{"Type":"recursive"},...}})
    // This is a bit hacky but works for our purposes
    char **cids = malloc(sizeof(char*) * 100); // Max 100 pins for now
    *count = 0;

    char *search = chunk.memory;
    while ((search = strstr(search, "\"Qm")) != NULL) {
        search++; // Skip past the quote
        char *end = strchr(search, '"');
        if (!end) break;

        size_t cid_len = end - search;
        cids[*count] = malloc(cid_len + 1);
        memcpy(cids[*count], search, cid_len);
        cids[*count][cid_len] = '\0';
        (*count)++;

        search = end;
        if (*count >= 100) break; // Safety limit
    }

    free(chunk.memory);
    return cids; // Caller must free with ipfs_free_string_array
}

void ipfs_response_free(ipfs_response_t *response) {
    if (response) {
        free(response->data);
        free(response->error);
        free(response);
    }
}

void ipfs_free_string_array(char **array, size_t count) {
    if (array) {
        for (size_t i = 0; i < count; i++) {
            free(array[i]);
        }
        free(array);
    }
}

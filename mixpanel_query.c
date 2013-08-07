/*
 * query_mixpanel.c
 *
 *  Created on: Jul 31, 2013
 *      Author: joe
 */

#include "mixpanel_query.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define MAX_URL_LENGTH 256
// 4 MP for posts
#define MAX_REQUEST_BODY_LENGTH 4194304
#define MAX_RESULT_LENGTH 256

struct resultbuffer {
    bool failed;
    size_t length;
    char *data;
};

static struct resultbuffer *create_resultbuffer() {
    struct resultbuffer *ret = malloc(sizeof(struct resultbuffer));
    if (NULL == ret) {
        return NULL;
    }
    ret->failed = false;
    ret->length = 0;
    ret->data = NULL;

    return ret;
}

static size_t write_to_resultbuffer(char *ptr, size_t size, size_t nmemb, void *results_ptr) {
    struct resultbuffer *results = results_ptr;
    if (results->failed) {
        return 0;
    }
    size_t new_length = results->length + size * nmemb;
    if (new_length < results->length) { // OVERFLOW
        results->failed = true;
        return 0;
    }
    if (new_length > MAX_RESULT_LENGTH) {
        results->failed = true;
        return 0;
    }
    char * new_data = realloc(results->data, new_length);
    if (NULL == new_data) {
        results->failed = true;
        return 0;
    }
    results->data = new_data;
    memcpy(results->data + results->length, ptr, size * nmemb);
    results->length = new_length;
    return size * nmemb;
}

const curl_write_callback write_callback = write_to_resultbuffer;

static void destroy_resultbuffer(struct resultbuffer *r) {
    free(r->data);
    free(r);
}

int mixpanel_query_init() {
    // MUST be able to be called more than once.
    if (curl_global_init(CURL_GLOBAL_ALL)) {
        fprintf(stderr, "Couldn't initialize cURL library.\n");
        return -1;
    }
    return 0;
}

void mixpanel_query_cleanup() {
    // MUST be able to be called more than once.
    curl_global_cleanup();
}

void mixpanel_query(const char *endpoint_url, const char *request_body){
    CURL *curl;
    CURLcode err;
    struct resultbuffer *resultbuffer = create_resultbuffer();
    if (NULL == resultbuffer) {
        fprintf(stderr, "Couldn't allocate space for results.\n");
        return;
    }
    curl = curl_easy_init();
    if (NULL == curl) {
        fprintf(stderr, "Couldn't allocate cURL\n");
        destroy_resultbuffer(resultbuffer);
        return;
    }
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); // TODO remove
    if ((err = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1)) ||
        (err = curl_easy_setopt(curl, CURLOPT_URL, endpoint_url)) ||
        (err = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body)) ||
        (err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, resultbuffer)) ||
        (err = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback))) {
        fprintf(stderr, "Couldn't configure libcurl: %s\n", curl_easy_strerror(err));
        goto CLEANUP;
    }
    err = curl_easy_perform(curl);
    if (err) {
        fprintf(stderr, "Couldn't make request: %s\n", curl_easy_strerror(err));
        goto CLEANUP;
    }
    if (resultbuffer->failed) {
        fprintf(stderr, "Results of request couldn't be retrieved.\n");
        goto CLEANUP;
    }
    if (resultbuffer->length != 1 || strncmp(resultbuffer->data, "1", resultbuffer->length) != 0) {
        fprintf(stderr, "Server will not track request.\n");
        goto CLEANUP;
    }

    CLEANUP:
    curl_easy_cleanup(curl);
    destroy_resultbuffer(resultbuffer);

    return;
}


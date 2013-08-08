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

struct resultchecker {
    size_t length;
};

static size_t check_for_success(char *ptr, size_t size, size_t nmemb, void *results_ptr) {
    struct resultchecker *results = results_ptr;
    // Success is precisely one character long, "1"
    results->length = results->length + size * nmemb;
    if (results->length != 1) {
        return 0;
    }
    if (ptr[0] != '1') {
        return 0;
    }
    return size * nmemb;
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

int mixpanel_query(const char *endpoint_url, const char *request_body){
    CURL *curl;
    CURLcode err;
    struct resultchecker resultchecker;
    resultchecker.length = 0;
    int returncode = 0;
    curl = curl_easy_init();
    if (NULL == curl) {
        returncode = -1;
        goto cleanup;
    }
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); // TODO remove
    if ((err = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1)) ||
        (err = curl_easy_setopt(curl, CURLOPT_URL, endpoint_url)) ||
        (err = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body)) ||
        (err = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultchecker)) ||
        (err = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, check_for_success))) {
        returncode = -1;
        goto cleanup;
    }
    err = curl_easy_perform(curl);
    if (err) {
        returncode = -1;
        goto cleanup;
    }

cleanup:
    if (NULL != curl) {
        curl_easy_cleanup(curl);
    }
    return returncode;
}


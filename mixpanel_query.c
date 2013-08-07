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
	// Reviewers - could this overflow (assuming it actually represents the
	// size of non-overlapping memory pointed to by *ptr)?
	size_t new_length = results->length + size * nmemb;
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

struct requestinfo {
	char *endpoint_url;
	char *request_body;
};

// TODO what if we're given non-null terminated strings? The checks below
static struct requestinfo *create_requestinfo(const char *endpoint_url, const char *request_body) {
	if (endpoint_url == NULL) {
		fprintf(stderr, "Cannot make a request to a null url\n");
		return NULL;
	}
	size_t endpoint_url_length = strnlen(endpoint_url, MAX_URL_LENGTH + 1);
	if (endpoint_url_length > MAX_URL_LENGTH) {
		fprintf(stderr, "Cannot make a request ot a URL more than %d bytes long\n", MAX_URL_LENGTH);
		return NULL;
	}
	if (request_body == NULL) {
		fprintf(stderr, "Cannot post with a null request body\n");
		return NULL;
	}
	size_t request_body_length = strnlen(request_body, MAX_REQUEST_BODY_LENGTH + 1);
	if (request_body_length > MAX_REQUEST_BODY_LENGTH) {
		fprintf(stderr, "Cannot make a request with body more than %d bytes long\n", MAX_REQUEST_BODY_LENGTH);
		return NULL;
	}
	char *info_request_body = malloc(sizeof(char) * (request_body_length + 1));
	if (info_request_body == NULL) {
		fprintf(stderr, "Can't allocate memory for request body\n");
		return NULL;
	}
	char *info_endpoint_url = malloc(sizeof(char) * (endpoint_url_length + 1));
	if (info_endpoint_url == NULL) {
		fprintf(stderr, "Can't allocate memory for request url\n");
		free(info_request_body);
		return NULL;
	}
	struct requestinfo *ret = malloc(sizeof(struct requestinfo));
	if (ret == NULL) {
		fprintf(stderr, "Can't allocaet memory for requestinfo\n");
		free(info_request_body);
		free(info_endpoint_url);
		return NULL;
	}

	memcpy(info_request_body, request_body, request_body_length + 1);
	memcpy(info_endpoint_url, endpoint_url, endpoint_url_length + 1);
	ret->endpoint_url = info_endpoint_url;
	ret->request_body = info_request_body;
	return ret;
}

static void destroy_requestinfo(struct requestinfo *r) {
	free(r->endpoint_url);
	free(r->request_body);
	free(r);
}

// TODO move back to caller, get rid of requestinfo
static void run_query_thread(void *requestinfo_ptr) {
	struct requestinfo *requestinfo = (struct requestinfo *)requestinfo_ptr;
	CURL *curl;
	CURLcode err;
	struct resultbuffer *resultbuffer = create_resultbuffer();
	if (NULL == resultbuffer) {
		fprintf(stderr, "Couldn't allocate space for results.\n");
		destroy_requestinfo(requestinfo);
		return;
	}
	curl = curl_easy_init();
	if (NULL == curl) {
		fprintf(stderr, "Couldn't allocate cURL\n");
		destroy_resultbuffer(resultbuffer);
		destroy_requestinfo(requestinfo);
		return;
	}
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1); // TODO remove
	if ((err = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1)) ||
		(err = curl_easy_setopt(curl, CURLOPT_URL, requestinfo->endpoint_url)) ||
		(err = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestinfo->request_body)) ||
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
	destroy_requestinfo(requestinfo);

	return;
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
	struct requestinfo *request = create_requestinfo(endpoint_url, request_body);
	run_query_thread(request);
}

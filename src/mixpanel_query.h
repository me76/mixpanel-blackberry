/*
 * query_mixpanel.h
 *
 *  Created on: Jul 31, 2013
 *      Author: joe
 */

#ifndef MIXPANEL_QUERY_H_
#define MIXPANEL_QUERY_H_

/*
 * Must be called to initialize the library. Not thread safe-
 * mixpanel_query_init() must be called from the main thread of your program,
 * before any other threads are running.
 */
int mixpanel_query_init();

/*
 * Called with a completely escaped payload, suitable for POSTing to mixpanel.
 * Thread safe (if init has been called early.) Returns 0 on success.
 * connect_timeout should be provided in seconds, or -1 to use the default
 * libcurl timeout.
 */
int mixpanel_query_with_timeout(const char *endpoint_url, const char *request_body, int connect_timeout);

/*
 * Called with a completely escaped payload, suitable for POSTing to mixpanel.
 * Thread safe (if init has been called early.) Returns 0 on success.
 * Always uses the libcurl default connect timeout.
 */
int mixpanel_query(const char *endpoint_url, const char *request_body);

/*
 * Must be called to clean up library resources. Not thread safe.
 * mixpanel_query_cleanup() should be called after all but one thread
 * in your program have completed.
 */
void mixpanel_query_cleanup();

#endif /* QUERY_MIXPANEL_H_ */

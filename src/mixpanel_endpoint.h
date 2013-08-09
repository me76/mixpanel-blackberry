/*
 * mixpanel_endpoint.h
 *
 *  Created on: Aug 3, 2013
 *      Author: joe
 */

#ifndef MIXPANEL_ENDPOINT_H_
#define MIXPANEL_ENDPOINT_H_

// The underlying numbers are known to
// our data store, so they can't change.
enum mixpanel_endpoint {
    MIXPANEL_ENDPOINT_UNDEFINED = 0,
    MIXPANEL_ENDPOINT_EVENTS = 1,
    MIXPANEL_ENDPOINT_PEOPLE = 2
};

#endif /* MIXPANEL_ENDPOINT_H_ */

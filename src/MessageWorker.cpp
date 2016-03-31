/*
 * MessageWorker.cpp
 *
 *  Created on: Aug 3, 2013
 *      Author: joe
 */

extern "C" {
#include "mixpanel_query.h"
}
#include "MessageWorker.h"

#include <time.h>

#include <iostream>
#include <ctime>

#include <bps/bps.h>
#include <bps/netstatus.h>

#define STORE_RECORDS_MAX 40
#define AUTO_FLUSH_TIMEOUT 7

namespace {

bool is_network_available() {
	bool isOnline = false;

    netstatus_info_t* networkStatus = 0;
    if(netstatus_get_info(&networkStatus) == BPS_SUCCESS) {
        isOnline = networkStatus && netstatus_info_get_availability(networkStatus);
        netstatus_free_info(&networkStatus);
    }

    return isOnline;
}

} //local namespace

namespace mixpanel {
namespace details {

// TODO move to endpoints.h
const char* MessageWorker::EVENTS_ENDPOINT_URL = "https://api.mixpanel.com/track";
const char* MessageWorker::PEOPLE_ENDPOINT_URL = "https://api.mixpanel.com/engage";

MessageWorker::MessageWorker()
    : m_store(), m_store_is_prepared(false) {}

MessageWorker::~MessageWorker() {}

void MessageWorker::message(enum mixpanel_endpoint endpoint, const QString &message) {
    prepareStore();
    if (!m_store.store(endpoint, message)) {
        return;
    }
    int count = -1;
    if (! m_store.count(endpoint, &count)) {
        return;
    }
    if (count >= STORE_RECORDS_MAX) {
        flushEndpoint(endpoint, AUTO_FLUSH_TIMEOUT);
    }
}

void MessageWorker::flush(int connect_timeout) {
    prepareStore();
    flushEndpoint(MIXPANEL_ENDPOINT_EVENTS, connect_timeout);
    flushEndpoint(MIXPANEL_ENDPOINT_PEOPLE, connect_timeout);
}

void MessageWorker::flushEndpoint(enum mixpanel_endpoint endpoint, int connect_timeout) {
    if (! is_network_available()) {
        return;
    }

    QStringList retrieved;
    int last_id = -1;
    do {
        if (! m_store.retrieve(endpoint, STORE_RECORDS_MAX, &retrieved, &last_id)) {
            return;
        }
        if (retrieved.count() == 0) {
            return;
        }
        QString json_payload;
        json_payload = retrieved.join(",");
        json_payload = json_payload.prepend("[");
        json_payload = json_payload.append("]");
        const char *endpoint_url = NULL;
        switch (endpoint) {
        case MIXPANEL_ENDPOINT_EVENTS: // TODO array index here.
            endpoint_url = EVENTS_ENDPOINT_URL;
            break;
        case MIXPANEL_ENDPOINT_PEOPLE:
            endpoint_url = PEOPLE_ENDPOINT_URL;
            break;
        case MIXPANEL_ENDPOINT_UNDEFINED:
            return; // This means there is a programmer bug.
        }
        if (sendData(endpoint_url, json_payload, connect_timeout)) {
            m_store.clearMessagesUptoId(endpoint, last_id);
        }
    } while(retrieved.count() > 0);
}

bool MessageWorker::sendData(const char *endpoint_url, const QString &json, int connect_timeout) {
    // TODO this is super memory intensive...
    QByteArray query_payload = json.toUtf8().toBase64();
    QByteArray query_payload_escaped = QUrl::toPercentEncoding(query_payload);
    QByteArray query_data_array("data=");
    query_data_array.append(query_payload_escaped);
    return 0 == mixpanel_query_with_timeout(endpoint_url, query_data_array, connect_timeout);
}


void MessageWorker::prepareStore() {
    if (!m_store_is_prepared) {
        std::time_t overdue = time(NULL) - (60 * 60 * 24 * 5); // Five days
        if (overdue > 0) { // account for screwy system clocks
            m_store.clearMessagesUptoTime(overdue);
        }
        m_store_is_prepared = true;
    }
}

} /* namespace details */
} /* namespace mixpanel */

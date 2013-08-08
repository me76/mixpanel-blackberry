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
#include <iostream>

#define STORE_RECORDS_MAX 40

namespace mixpanel {
namespace details {

// TODO move to endpoints.h
const char* MessageWorker::EVENTS_ENDPOINT_URL = "https://api.mixpanel.com/track";
const char* MessageWorker::PEOPLE_ENDPOINT_URL = "https://api.mixpanel.com/engage";

MessageWorker::MessageWorker()
    : QObject(0), m_store() {
}

MessageWorker::~MessageWorker() {}

void MessageWorker::message(enum mixpanel_endpoint endpoint, const QString &message) {
    if (!m_store.store(endpoint, message)) {
        return;
    }
    int count = -1;
    if (! m_store.count(endpoint, &count)) {
        return;
    }
    if (count >= STORE_RECORDS_MAX) {
        flushEndpoint(endpoint);
    }
}

void MessageWorker::flush() {
    flushEndpoint(MIXPANEL_ENDPOINT_EVENTS);
    flushEndpoint(MIXPANEL_ENDPOINT_PEOPLE);
}

void MessageWorker::flushEndpoint(enum mixpanel_endpoint endpoint) {
    QStringList retrieved;
    int last_id = -1;
    if (! m_store.retrieve(endpoint, &retrieved, &last_id)) {
        return;
    }
    if (retrieved.count() == 0) {
        return;
    }
    QString json_payload = retrieved.join(",");
    json_payload.prepend("[");
    json_payload.append("]");
    const char *endpoint_url;
    switch (endpoint) {
    case MIXPANEL_ENDPOINT_EVENTS: // TODO array index here.
        endpoint_url = EVENTS_ENDPOINT_URL;
        break;
    case MIXPANEL_ENDPOINT_PEOPLE:
        endpoint_url = PEOPLE_ENDPOINT_URL;
        break;
    }
    if (sendData(endpoint_url, json_payload)) {
        m_store.clearMessagesUptoId(endpoint, last_id);
    }
}

bool MessageWorker::sendData(const char *endpoint_url, const QString &json) {
    // TODO this is super memory intensive...
    QByteArray query_payload = json.toUtf8().toBase64();
    QByteArray query_payload_escaped = QUrl::toPercentEncoding(query_payload);
    QByteArray query_data_array("data=");
    query_data_array.append(query_payload_escaped);
    return 0 == mixpanel_query(endpoint_url, query_data_array);
}

}

} /* namespace mixpanel */

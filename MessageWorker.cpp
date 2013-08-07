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

MessageWorker::MessageWorker()
	: QObject(0), /* <<-- TODO BAD! NEEDS PARENT */ m_store() {
	failed = false;
}

MessageWorker::~MessageWorker() {

}

void MessageWorker::message(enum mixpanel_endpoint endpoint, QString message) {
	if (failed) {
		return;
	}
	if (!m_store.store(endpoint, message)) {
		failed = true;
		return;
	}
	int count = -1;
	if (! m_store.count(endpoint, count)) {
		failed = true;
	}
	if (count >= STORE_RECORDS_MAX) {
		flushEndpoint(endpoint);
	}
}

void MessageWorker::flush() {
	if (failed) {
		return;
	}
	flushEndpoint(MIXPANEL_ENDPOINT_EVENTS);
	flushEndpoint(MIXPANEL_ENDPOINT_PEOPLE);
}

void MessageWorker::flushEndpoint(enum mixpanel_endpoint endpoint) {
	QStringList stored;
	int last_id = -1;
	if (! m_store.retrieve(endpoint, stored, last_id)) {
		failed = true;
		return;
	}
	QString json_payload = stored.join(",");
	json_payload.prepend("[");
	json_payload.append("]");
	QUrl endpoint_url;
	switch (endpoint) {
	case MIXPANEL_ENDPOINT_EVENTS:
		endpoint_url = QUrl("https://api.mixpanel.com/track");
		break;
	case MIXPANEL_ENDPOINT_PEOPLE:
		endpoint_url = QUrl("https://api.mixpanel.com/engage");
		break;
	}
	sendData(endpoint_url, json_payload);
}

void MessageWorker::sendData(const QUrl &endpoint_url, const QString &json) {
	// TODO this is super memory intensive...
	QByteArray query_payload = json.toUtf8().toBase64();
	QByteArray query_payload_escaped = QUrl::toPercentEncoding(query_payload);
	QByteArray query_data_array("data=");
	query_data_array.append(query_payload_escaped);
	const char *url_str = endpoint_url.toString().toAscii();
	// TODO since this is synchronous, check return value and don't clear the records
	// unless you send them ok.
	mixpanel_query(url_str, query_data_array);
}

}

} /* namespace mixpanel */

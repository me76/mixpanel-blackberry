/*
 * Mixpanel.cpp
 *
 *  Created on: Jul 30, 2013
 *      Author: joe
 */

extern "C" {
#include "mixpanel_query.h"
}

#include "mixpanel_endpoint.h"
#include "Mixpanel.h"
#include <iostream>
#include <curl/curl.h>
#include <bb/data/JsonDataAccess>

namespace mixpanel {

using namespace bb::data;

details::MessageThread *Mixpanel::s_thread = NULL;

bool Mixpanel::init() {
	if (mixpanel_query_init()) {
		return false;
	}
	if (NULL == s_thread) {
		s_thread = new details::MessageThread();
	}
	return true;
}

void Mixpanel::cleanup() {
	s_thread->flush(); // TODO CAN'T LIVE HERE. WE HAVE TO WAIT
	mixpanel_query_cleanup();
	if (NULL != s_thread) {
		delete s_thread;
		s_thread = NULL;
	}
}

Mixpanel::Mixpanel(QString token) {
	m_token = token;
}

Mixpanel::~Mixpanel() {}

bool Mixpanel::track(QString event_name, QVariantMap properties) {
	QVariantMap default_properties = getDefaultProperties();
	QVariantMap use_properties = properties;
	use_properties.unite(default_properties); // TODO Backwards?
	QVariantMap event;
	event["event"] = event_name;
	event["properties"] = use_properties;
	JsonDataAccess jda;
	QString json_buffer;
	jda.saveToBuffer(event, &json_buffer);
	if (jda.hasError()) {
		const DataAccessError err = jda.error();
		const QString err_message = err.errorMessage();
		std::cerr << "Couldn't write properties as JSON\n";
		return false;
	}
	s_thread->message(MIXPANEL_ENDPOINT_EVENTS, json_buffer);
	return true;
}

QVariantMap Mixpanel::getDefaultProperties() {
	QVariantMap ret;
	ret["token"] = m_token;
	ret["mp_lib"] = QString("blackberry sketch");
	ret["$os"] = QString("Blackberry 10");
	return ret;
}

} // namespace mixpanel

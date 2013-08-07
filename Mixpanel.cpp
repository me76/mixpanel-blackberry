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
	mixpanel_query_cleanup();
	if (NULL != s_thread) {
		delete s_thread;
		s_thread = NULL;
	}
}

Mixpanel::Mixpanel() {}

Mixpanel::~Mixpanel() {
	// TODO Auto-generated destructor stub
}

bool Mixpanel::track(const QString &event_name, const QVariantMap &properties) {
	QVariantMap default_properties = getDefaultProperties();
	QVariantMap use_properties = properties;
	use_properties.unite(default_properties);
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
	s_thread->flush(); // TODO CAN'T LIVE HERE.
	return true;
}

QVariantMap Mixpanel::getDefaultProperties() { // TODO this is silly
	QVariantMap ret;
	ret["token"] = QString("5d90aa5b938f233f2cea15d8b31bceb7");
	ret["mp_lib"] = QString("blackberry sketch");
	ret["$os"] = QString("Blackberry 10");
	return ret;
}

} // namespace mixpanel

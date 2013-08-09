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
#include <time.h>
#include <curl/curl.h>
#include <bb/data/JsonDataAccess>

namespace mixpanel {

using namespace bb::data;

struct initialization {
    initialization() {
        mixpanel_query_init();
        details::Preferences::deletePreferences();
    }
    ~initialization() {
        mixpanel_query_cleanup();
    }
};

static initialization init;

details::MessageThread Mixpanel::s_thread;
details::Preferences Mixpanel::s_preferences;

const char Mixpanel::VERSION[] = "0.0.4pre";

Mixpanel::Mixpanel(const QString &token)
   : m_token(token) {
}

Mixpanel::~Mixpanel() {}

bool Mixpanel::track(const QString &event_name, const QVariantMap &properties) {
    // Must be reentrant
    QVariantMap default_properties = s_preferences.getSuperProperties(m_token);
    default_properties["token"] = m_token;
    default_properties["time"] = time(NULL);
    default_properties["mp_lib"] = QString("blackberry");
    default_properties["$lib_version"] = QString(VERSION);
    default_properties["$os"] = QString("Blackberry 10+");
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
        return false;
    }
    s_thread.message(MIXPANEL_ENDPOINT_EVENTS, json_buffer);
    return true;
}

void Mixpanel::flush() {
    // Must be reentrant
    s_thread.flush();
}

void Mixpanel::registerSuperProperty(const QString &name, const QVariant &value) {
	// Must be reentrant
	s_preferences.setSuperProperty(m_token, name, value);
}

void Mixpanel::identify(const QString &distinct_id) {
	// Must be reentrant
	s_preferences.setDistinctId(m_token, distinct_id);
}

} // namespace mixpanel

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
    }
    ~initialization() {
        mixpanel_query_cleanup();
    }
};

static initialization init;

details::MessageThread Mixpanel::s_thread;

Mixpanel::Mixpanel(const QString &token)
   : m_token(token) {
}

Mixpanel::~Mixpanel() {}

bool Mixpanel::track(const QString &event_name, const QVariantMap &properties) {
    QVariantMap default_properties = getDefaultProperties();
    default_properties["time"] = time(NULL);
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

QVariantMap Mixpanel::getDefaultProperties() { // TODO use a static map
    QVariantMap ret;
    ret["token"] = m_token;
    ret["mp_lib"] = QString("blackberry sketch");
    ret["$os"] = QString("Blackberry QNX");

    return ret;
}

} // namespace mixpanel

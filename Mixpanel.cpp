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

details::MessageThread Mixpanel::s_thread;

bool Mixpanel::init() {
    if (mixpanel_query_init()) {
        return false;
    }
    return true;
}

void Mixpanel::cleanup() {
    s_thread.flush();
    mixpanel_query_cleanup();
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
    s_thread.message(MIXPANEL_ENDPOINT_EVENTS, json_buffer);
    return true;
}

QVariantMap Mixpanel::getDefaultProperties() {
    QVariantMap ret;
    ret["token"] = m_token;
    ret["mp_lib"] = QString("blackberry sketch");
    ret["$os"] = QString("Blackberry QNX");
    ret["time"] = time(NULL);
    return ret;
}

} // namespace mixpanel

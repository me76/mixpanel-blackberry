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
#include <bps/deviceinfo.h>
#include <bb/data/JsonDataAccess>
#include <bb/device/HardwareInfo>

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
details::Preferences Mixpanel::s_preferences;

const char Mixpanel::VERSION[] = "0.0.5pre";

Mixpanel::Mixpanel(const QString &token)
   : m_token(token), m_auto_props() {
    QVariantMap autoprops;
    bb::device::HardwareInfo info;
    QString val;
    m_auto_props["token"] = m_token;
    m_auto_props["mp_lib"] = QString("blackberry");
    m_auto_props["$lib_version"] = QString(VERSION);
    m_auto_props["$os"] = QString("Blackberry 10+");
    m_auto_props["$physical_keyboard_device"] = info.isPhysicalKeyboardDevice();
    val = info.deviceName();
    if (! val.isEmpty()) {
        m_auto_props["$device_name"] = val;
    }
    val = info.modelName();
    if (! val.isEmpty()) {
        m_auto_props["$model_name"] = val;
    }
    val = info.modelNumber();
    if (! val.isEmpty()) {
        m_auto_props["$model_number"] = val;
    }
    deviceinfo_details_t* data;
    deviceinfo_get_details(&data);
    const char* os_version = deviceinfo_details_get_device_os(data);
    m_auto_props["$os_version"] = QString(os_version);
    deviceinfo_free_details(&data);
}

Mixpanel::~Mixpanel() {}

bool Mixpanel::track(const QString &event_name, const QVariantMap &properties) {
    // Must be reentrant
    QVariantMap default_properties = s_preferences.getSuperProperties(m_token);
    default_properties["time"] = time(NULL);
    QVariantMap use_properties = properties;
    use_properties.unite(default_properties);
    use_properties.unite(m_auto_props);
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

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

#define FLUSH_DEFAULT_TIMEOUT_SECONDS 7

namespace mixpanel {

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

const char Mixpanel::VERSION[] = "0.1.1";

Mixpanel::Mixpanel(const QString &token)
   : m_token(token), m_auto_props() {
    QVariantMap autoprops;
    bb::device::HardwareInfo info;
    QString val;
    m_auto_props["token"] = m_token;
    m_auto_props["mp_lib"] = QString("blackberry");
    m_auto_props["$lib_version"] = QString(VERSION);
    m_auto_props["$os"] = QString("BlackBerry 10+");
    m_auto_props["$physical_keyboard_device"] = info.isPhysicalKeyboardDevice();
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
    const char* os_version = deviceinfo_details_get_device_os_version(data);
    m_auto_props["$os_version"] = QString(os_version);
    deviceinfo_free_details(&data);
}

Mixpanel::~Mixpanel() {}

void Mixpanel::stopTracking() {
    s_thread.stopBlocking();
}

void Mixpanel::enableAutoflush() {
    s_thread.enableAutoflush();
}

void Mixpanel::disableAutoflush() {
    s_thread.disableAutoflush();
}

void Mixpanel::stopTrackingInApplication(bb::cascades::Application *app) {
    int tries = 0;
    int waitMs = 200;
    int maxWaitMs = 1000 * 5; // 5 second wait
    int maxTries = maxWaitMs / waitMs;

    QWaitCondition waitCondition;
    QMutex mutex;
    QMutexLocker lock(&mutex);
    s_thread.stopNonblocking();
    while(tries < maxTries && ! s_thread.isFinished()) {
        int depth = s_thread.getDepth();
        qDebug() << "Stopping in Mixpanel Library, Depth " << depth << " Try " << tries;
        app->extendTerminationTimeout();
        waitCondition.wait(&mutex, waitMs);
        tries = tries + 1;
    }
    lock.unlock();
    s_thread.wait();
}

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
    bb::data::JsonDataAccess jda;
    QString json_buffer;
    jda.saveToBuffer(event, &json_buffer);
    if (jda.hasError()) {
        return false;
    }
    s_thread.message(MIXPANEL_ENDPOINT_EVENTS, json_buffer);
    return true;
}

bool Mixpanel::setupUserProfile(UpdateProfileOperation operation, const QVariantMap& properties) {
    switch(operation) {
        case Add:
            return engage("$add", properties);
        break;
        case Append:
            return engage("$append", properties);
        break;
        case Merge:
            return engage("$union", properties);
        break;
        case Set:
            return engage("$set", properties);
        break;
        case SetOnce:
            return engage("$set_once", properties);
        break;
        default:
            return false;
    }
}

bool Mixpanel::dropProfileProperty(const QString& propertyName) {
    return dropProfileProperties(QStringList(propertyName));
}

bool Mixpanel::dropProfileProperties(const QStringList& propertyNames) {
    return engage("$unset", propertyNames);
}

bool Mixpanel::dropUserProfile() {
    return engage("$delete", QVariant());
}

bool Mixpanel::engage(const QString& operation, const QVariant& properties) {
    // Must be reentrant
    QVariantMap engage_data;
    engage_data["$token"] = m_token;
    engage_data["$distinct_id"] = s_preferences.getDistinctId(m_token);

    if("$set" == operation || "$set_once" == operation) {
        engage_data["$created"] = QDateTime::currentDateTimeUtc().toString("YYYY-MM-DDThh:mm:ss");
    }

    engage_data[operation] = properties;

    bb::data::JsonDataAccess jda;
    QString json_buffer;
    jda.saveToBuffer(engage_data, &json_buffer);
    if(jda.hasError()) {
        return false;
    }

    s_thread.message(MIXPANEL_ENDPOINT_PEOPLE, json_buffer);
    return true;
}

void Mixpanel::flush() {
    // Must be reentrant
    flush_all();
}

void Mixpanel::flush_with_timeout(int connect_timeout) {
    // Must be reentrant
	flush_all_with_timeout(connect_timeout);
}

void Mixpanel::flush_all() {
    s_thread.flush(FLUSH_DEFAULT_TIMEOUT_SECONDS);
}

void Mixpanel::flush_all_with_timeout(int connect_timeout) {
    s_thread.flush(connect_timeout);
}

void Mixpanel::registerSuperProperty(const QString &name, const QVariant &value) {
    // Must be reentrant
    s_preferences.setSuperProperty(m_token, name, value);
}

void Mixpanel::clearSuperProperties() {
    // Must be reentrant
    s_preferences.clearSuperProperties(m_token);
}

void Mixpanel::identify(const QString &distinct_id) {
    // Must be reentrant
    s_preferences.setDistinctId(m_token, distinct_id);
}

} // namespace mixpanel

/*
 * SuperProperties.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: joe
 */

#include "Preferences.h"

#include <QDir>
#include <QUuid>
#include <QReadLocker>
#include <QWriteLocker>

#include <bb/device/HardwareInfo>

namespace mixpanel {
namespace details {

#define MIXPANEL_SETTINGS_PATH "data/mixpanel_properties"
#define MIXPANEL_SUPERPROPERTIES_KEY "superproperties"
#define MIXPANEL_DISTINCT_ID_KEY "distinct_id"
#define MIXPANEL_DEFAULT_ID_KEY "default_id"

bool Preferences::deletePreferences() {
    QFile prefs_file(QDir::homePath() + MIXPANEL_SETTINGS_PATH);
    if (! prefs_file.exists()) {
        return true; // if it never existed in the first place, then it's still deleted
    }
    return prefs_file.remove();
}

Preferences::Preferences()
	: m_settings(QDir::homePath() + MIXPANEL_SETTINGS_PATH),
	  m_settings_lock() {
	if (! m_settings.contains(MIXPANEL_DEFAULT_ID_KEY)) {
		bb::device::HardwareInfo info;
		QString default_id = info.pin();
		if (default_id.isEmpty()) {
			default_id = QString("R") + QUuid::createUuid().toString();
		} else {
			default_id = default_id.prepend('P');
		}
		m_settings.setValue(MIXPANEL_DEFAULT_ID_KEY, default_id);
	}
	m_default_distinct_id = m_settings.value(MIXPANEL_DEFAULT_ID_KEY).toString();
}

Preferences::~Preferences() {}

QVariantMap Preferences::getSuperProperties(const QString &token) {
	// Must be thread safe
	QReadLocker lock(&m_settings_lock);
	m_settings.beginGroup(token);
	QVariantMap ret = m_settings.value(MIXPANEL_SUPERPROPERTIES_KEY).toMap();
	ret["distinct_id"] = m_settings.value(MIXPANEL_DISTINCT_ID_KEY, m_default_distinct_id).toString();
	m_settings.endGroup();
	return ret;
}

void Preferences::setSuperProperty(const QString &token, const QString &name, const QVariant &value) {
	// Must be thread safe
	QWriteLocker lock(&m_settings_lock);
	m_settings.beginGroup(token);
	QVariantMap superprops = m_settings.value(MIXPANEL_SUPERPROPERTIES_KEY).toMap();
	superprops.insert(name, value);
	m_settings.setValue(MIXPANEL_SUPERPROPERTIES_KEY, superprops);
	m_settings.endGroup();
}

void Preferences::setDistinctId(const QString &token, const QString &distinct_id) {
	// Must be thread safe
	QWriteLocker lock(&m_settings_lock);
	m_settings.beginGroup(token);
	m_settings.setValue(MIXPANEL_DISTINCT_ID_KEY, distinct_id);
	m_settings.endGroup();
}

} /* namespace details */
} /* namespace mixpanel */

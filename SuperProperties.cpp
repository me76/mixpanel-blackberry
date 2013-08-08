/*
 * SuperProperties.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: joe
 */

#include "SuperProperties.h"

#include <QDir>
#include <QUuid>

namespace mixpanel {
namespace details {

#define MIXPANEL_SETTINGS_PATH "data/mixpanel_properties.conf"
#define MIXPANEL_SUPERPROPERTIES_KEY "superproperties"
#define MIXPANEL_DISTINCT_ID_KEY "distinct_id"

Preferences::Preferences()
	: m_settings(QDir::homePath() + MIXPANEL_SETTINGS_PATH),
	  m_superproperties(),
	  m_distinct_id(),
	  m_props_lock() {
	m_superproperties = m_settings.value(MIXPANEL_SUPERPROPERTIES_KEY, QVariantMap()).toMap();
	m_distinct_id = m_settings.value(MIXPANEL_DISTINCT_ID_KEY).toString();
	if (m_distinct_id.isEmpty()) {
		QUuid defaultUUID = QUuid::createUuid();
		setDistinctId(defaultUUID.toString());
	}
}

Preferences::~Preferences() {}

QVariantMap Preferences::getSuperProperties() {
	// Must be thread safe
	m_props_lock.lockForRead();
	QVariantMap ret = m_superproperties;
	ret["distinct_id"] = m_distinct_id;
	m_props_lock.unlock();
	return ret;
}

QString Preferences::getDistinctId() {
	// Must be thread safe
	m_props_lock.lockForRead();
	QString ret = m_distinct_id;
	m_props_lock.unlock();
	return ret;
}

void Preferences::setSuperProperty(const QString &name, const QVariant &value) {
	// Must be thread safe
	m_props_lock.lockForWrite();
	m_superproperties.insert(name, value);
	m_settings.setValue(MIXPANEL_SUPERPROPERTIES_KEY, m_superproperties);
	m_props_lock.unlock();
}

void Preferences::setDistinctId(const QString &distinct_id) {
	// Must be thread safe
	m_props_lock.lockForWrite();
	m_distinct_id = distinct_id;
	m_settings.setValue(MIXPANEL_DISTINCT_ID_KEY, m_distinct_id);
	m_props_lock.unlock();
}

} /* namespace details */
} /* namespace mixpanel */

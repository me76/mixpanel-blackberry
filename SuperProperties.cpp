/*
 * SuperProperties.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: joe
 */

#include "SuperProperties.h"

#include <QDir>

namespace mixpanel {
namespace details {

#define MIXPANEL_SETTINGS_PATH "data/mixpanel_properties.conf"
#define MIXPANEL_SUPERPROPERTIES_KEY "superproperties"

SuperProperties::SuperProperties()
	: m_settings(QDir::homePath() + MIXPANEL_SETTINGS_PATH),
	  m_props(),
	  m_props_lock() {
	m_props = m_settings.value(MIXPANEL_SUPERPROPERTIES_KEY, QVariantMap()).toMap();
}

SuperProperties::~SuperProperties() {}

QVariantMap SuperProperties::get() { // TODO should this just update a given map?
	// Must be thread safe
	m_props_lock.lockForRead();
	QVariantMap ret = m_props;
	m_props_lock.unlock();
	return ret;
}

void SuperProperties::set(const QString &name, const QVariant &value) {
	// Must be thread safe
	m_props_lock.lockForWrite();
	m_props.insert(name, value);
	m_settings.setValue(MIXPANEL_SUPERPROPERTIES_KEY, m_props);
	m_props_lock.unlock();
}

} /* namespace details */
} /* namespace mixpanel */

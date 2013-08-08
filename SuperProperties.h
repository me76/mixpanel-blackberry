/*
 * SuperProperties.h
 *
 *  Created on: Aug 8, 2013
 *      Author: joe
 */

#ifndef SUPERPROPERTIES_H_
#define SUPERPROPERTIES_H_

#include <QVariantMap>
#include <QSettings>
#include <QReadWriteLock>

namespace mixpanel {
namespace details {

class Preferences {
public:
	Preferences();
	virtual ~Preferences();
	QVariantMap getSuperProperties();
	void setSuperProperty(const QString &name, const QVariant &value);
	QString getDistinctId();
	void setDistinctId(const QString &distinct_id);
private:
	QSettings m_settings;
	QVariantMap m_superproperties;
	QString m_distinct_id;
	QReadWriteLock m_props_lock;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* SUPERPROPERTIES_H_ */

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

	QVariantMap getSuperProperties(const QString &token) const;
	void setSuperProperty(const QString &token, const QString &name, const QVariant &value);
	void clearSuperProperties(const QString &token);

	void setDistinctId(const QString &token, const QString &distinct_id);
	QString getDistinctId(const QString &token) const;

	static bool deletePreferences();

private:
	mutable QReadWriteLock m_settings_lock;

	QSettings m_settings;
	QString m_default_distinct_id;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* SUPERPROPERTIES_H_ */

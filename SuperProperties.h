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

class SuperProperties {
public:
	SuperProperties();
	virtual ~SuperProperties();
	QVariantMap get();
	void set(const QString &name, const QVariant &value);
private:
	QSettings m_settings;
	QVariantMap m_props;
	QReadWriteLock m_props_lock;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* SUPERPROPERTIES_H_ */

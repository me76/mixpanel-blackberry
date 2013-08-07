/*
 * Mixpanel.h
 *
 *  Created on: Jul 30, 2013
 *      Author: joe
 */

#ifndef MIXPANEL_H_
#define MIXPANEL_H_

#include "MessageThread.h"
#include <QVariantMap>

namespace mixpanel {

class Mixpanel {
public:
	Mixpanel(QString token);
	virtual ~Mixpanel();
	virtual bool track(QString event_name, QVariantMap properties);
	static bool init();
	static void cleanup();
private:
	QVariantMap getDefaultProperties();
	QString m_token;
	static details::MessageThread *s_thread;
};

} // namespace mixpanel

#endif /* MIXPANEL_H_ */

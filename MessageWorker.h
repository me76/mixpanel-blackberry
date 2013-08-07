/*
 * MessageWorker.h
 *
 *  Created on: Aug 3, 2013
 *      Author: joe
 */

#ifndef MESSAGEWORKER_H_
#define MESSAGEWORKER_H_

#include "MessageStore.h"
#include <QObject>

namespace mixpanel {
namespace details {

class MessageWorker: public QObject {
 	Q_OBJECT
public:
	MessageWorker();
	virtual ~MessageWorker();
public slots:
	void message(enum mixpanel_endpoint endpoint, QString message);
	void flush();
private:
	void flushEndpoint(enum mixpanel_endpoint endpoint);
	void sendData(const char *endpoint_url, const QString &json);
	MessageStore m_store;
	bool failed;
	static const char* EVENTS_ENDPOINT_URL;
	static const char* PEOPLE_ENDPOINT_URL;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGEWORKER_H_ */

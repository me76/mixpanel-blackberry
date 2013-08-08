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
    ~MessageWorker();
public slots:
    void message(enum mixpanel_endpoint endpoint, const QString &message);
    void flush();
private:
    MessageWorker(const MessageWorker&);
    MessageWorker& operator=(const MessageWorker&);
    void flushEndpoint(enum mixpanel_endpoint endpoint);
    bool sendData(const char *endpoint_url, const QString &json);
    MessageStore m_store;
    static const char* EVENTS_ENDPOINT_URL; // TODO move to endpoints.h
    static const char* PEOPLE_ENDPOINT_URL;
};

} /* namespace details */
} /* namespace mixpanel */
#endif /* MESSAGEWORKER_H_ */

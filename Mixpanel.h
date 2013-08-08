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

/**
 * The Mixpanel class is the public interface to the Mixpanel tracking library.
 * It is the only user-facing class provided by the Mixpanel library- none of the
 * other classes in the distribution should be referenced directly by client code.
 *
 * To track an event in your application,
 *
 * 1) create a Mixpanel instance using your project token
 *        Mixpanel tracker("MY_PROJECT_TOKEN")
 *
 * 2) call Mixpanel::track on your instance, with an event name
 *        and a (possibly empty) QVariantMap of event properties.
 *
 * The Mixpanel class is reentrant- you can use instances and copies
 * of instances from different threads.
 */

class Mixpanel {
public:
    Mixpanel(const QString &token);
    virtual ~Mixpanel();
    virtual bool track(const QString &event_name, const QVariantMap &properties);
    virtual void flush();
private:
    QVariantMap getDefaultProperties();
    const QString m_token;
    static details::MessageThread s_thread;
};

} // namespace mixpanel

#endif /* MIXPANEL_H_ */

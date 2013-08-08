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
    Mixpanel(const QString &token);
    virtual ~Mixpanel();
    virtual bool track(const QString &event_name, const QVariantMap &properties);
    virtual bool flush();
private:
    Mixpanel(const Mixpanel&);
    Mixpanel& operator=(const Mixpanel&);
    QVariantMap getDefaultProperties();
    const QString m_token;
    static details::MessageThread s_thread;
};

} // namespace mixpanel

#endif /* MIXPANEL_H_ */

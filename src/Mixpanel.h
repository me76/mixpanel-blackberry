/*
 * Mixpanel.h
 *
 *  Created on: Jul 30, 2013
 *      Author: joe
 */

#ifndef MIXPANEL_H_
#define MIXPANEL_H_

#include "MessageThread.h"
#include "Preferences.h"
#include <QVariantMap>
#include <bb/cascades/Application>

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
    /**
     * Create a new instance of the Mixpanel class. token should be your Mixpanel project token
     * (You can find your project token in the settings dialog of your Mixpanel project.)
     *
     * Instances of Mixpanel should be confined to a single thread, but you can use multiple
     * instances in your application. Instances with the same token will share the same
     * distinct_id (if you call identify()) and the same super properties.
     */
    Mixpanel(const QString &token);
    virtual ~Mixpanel();

    /**
     * Send a record of an event to Mixpanel.
     * The event will appear in Mixpanel with the given event name, and will
     * be associated with the given properties. If you have registered superProperties
     * with this token, they will be sent along with the properties you provide
     * in the method arguments
     *
     * The properties map should contain only contain types
     * that are associated with primitive JSON values, QVariantLists and QVariantMaps;
     * otherwise, the event will not be tracked.
     *
     * The event you track will be sent to Mixpanel with a distinct id, either
     * automatically generated by the library or the id you provided with a
     * previous call to identify()
     */
    virtual bool track(const QString &event_name, const QVariantMap &properties);

    /**
     * Send all events waiting in device persistent storage to Mixpanel.
     * By default, the library will store events on the hard drive and send
     * them in batches, to preserve bandwidth and device battery life. If
     * you want to send events immediately (either because you want to
     * observe them immediately in the Mixpanel interface, or because your
     * application is exiting) you can call flush(), which will send all waiting
     * events immediately.
     */
    virtual void flush();

    /**
     * Send all events waiting in device persistent storage to Mixpanel.
     * Differs from flush() in that it takes a connect_timeout argument.
     * The Mixpanel library will use this argument rather than the default
     * connect timeout when attempting to send data. Longer timeouts will
     * work better in uncertain network environments, but shorter timeouts
     * will ensure the application completes in a timely fashion.
     */
    virtual void flush_with_timeout(int connect_timeout);


    /**
     * Assign a distinct id to all events sent by this device. The Mixpanel
     * library will automatically assign a distinct id to all of your events,
     * but you can override this automatic id by calling identify(). All events
     * sent after the call to identify will use the distinct_id you assign.
     * The id you set with identify will be saved to device storage, and
     * persist between invocations of your app.
     *
     * Distinct ids are shared between instances of Mixpanel that use
     * the same token.
     */
    virtual void identify(const QString &distinct_id);

    /**
     * Record properties you would like to send along with every event
     * to Mixpanel. A property with the name and value you provide to
     * registerSuperProperty() will be sent with every subsequent call
     * to track().
     *
     * The properties map should contain only contain types
     * that are associated with primitive JSON values, QVariantLists and QVariantMaps;
     * otherwise, the values will appear as "null" in Mixpanel.
     *
     * Super properties are shared between instances of Mixpanel that use
     * the same token.
     */
    virtual void registerSuperProperty(const QString &name, const QVariant &value);

    /**
     * Clear all properties previously registered with registerSuperProperty.
     * This will clear all super properties for all instances of Mixpanel that
     * use the same token.
     */
    virtual void clearSuperProperties();

    /**
     * Clears the Mixpanel library, waits on outstanding tracking calls,
     * and stops the tracking thread. You should call
     * this method in the main thread of your application before exit.
     * You should not call any methods on any Mixpanel instances after calling
     * stopTracking()
     */
    static void stopTracking();

    /**
     * Clears the Mixpanel library, waits on outstanding tracking calls,
     * and calls extendTerminationTimeout on the given Application until
     * the stop is complete.
     * You should not call any methods on any Mixpanel instances after calling
     * stopTracking()
     */
    static void stopTrackingInApplication(bb::cascades::Application *app);

    /**
     * Turns off auto-flush behavior. By default, the Mixpanel library will
     * automatically attempt to flush contents to it's servers, but if
     * disableAutoflush is set, it will only flush as the result of some
     * call to flush on an instance.
     */
    static void disableAutoflush();

    /**
     * Turns on auto-flush behavior, if it has been previously disabled by
     * a call to disableAutoflush.
     */
    static void enableAutoflush();

    /**
     * Equivalent to calling Mixpanel.flush, but does not require an instance.
     */
    static void flush_all();

    /**
     * Equivalent to calling Mixpanel.flush_with_timeout, but does not require an instance.
     */
    static void flush_all_with_timeout(int connect_timeout);

    /**
     * A string that records the Mixpanel library version.
     */
    static const char VERSION[];

private:
    QVariantMap getDefaultProperties();
    const QString m_token;
    QVariantMap m_auto_props;
    static details::MessageThread s_thread;
    static details::Preferences s_preferences;
};

} // namespace mixpanel

#endif /* MIXPANEL_H_ */

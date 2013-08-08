This is the Mixpanel client library for the Blackberry 10 operating system.

To use  this library in your project

- Add the following to the base .pro file

LIBS += -lbbdata
LIBS += -lcurl

- Drop the "mixpanel" directory into your applications src/ directory

Then you can instantiate instances of the Mixpanel class and track events with:

    #include "mixpanel/Mixpanel.h"

    ...

    Mixpanel tracker("YOUR PROJECT TOKEN");
    QVariantMap event_properties;
    tracker.track("My Event", event_properties);

The Mixpanel methods are reentrant - it's safe to have multiple instances of Mixpanel,
and use each of them from their own thread. It is *not* guaranteed that you can use
the same instance of Mixpanel from multiple threads safely.

registerSuperProperty:

You can set persistent properties that will be sent with every user event
by calling Mixpanel::registerSuperProperty(QString name, QVariant value). The given
arguments will be saved to persistent storage on the device, and sent with
every future call to track().

identify:

The library will generate (and save) a distinct identifier for every install
of your application, and send it with your events. If you would like to use
your own identifier, you can change the identifier stored with the library
by calling Mixpanel::identify(QString distinct_id). The value you provide
will be saved to persistent storage on the device.

flush:

The Mixpanel library sends updates to Mixpanel servers in batches, to preserve
battery life and save bandwidth. You can request that the library send all of the
events being stored by calling Mixpanel::flush(). It is recommended (but not required)
that you call flush() when your application is about to exit- if you don't call flush(),
events that you tracked will be sent the next time the application is started.


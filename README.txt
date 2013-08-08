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


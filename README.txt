This is the Mixpanel client library for the Blackberry 10 operating system.

Please see the file LICENSE in this distribution for licensing terms.

USING A BINARY RELEASE:

To use a binary release in your project, visit the releases page

    https://github.com/mixpanel/mixpanel-blackberry/releases

and download the mixpanel-blackberry.zip file associated with your release.

* Unzip the contents of the archive into a "mixpanel" directory associated with your project.

* Add the following to the base .pro file in your project:

    LIBS += -lbbdata
    LIBS += -lbbdevice
    LIBS += -lcurl
    INCLUDEPATH += mixpanel/src

* Add the following to the individual CONFIG sections in your .pro file

    # Add this to your Device-Debug, Device-Release, and other arm target configs
    LIBS += -L$$quote($$BASEDIR/mixpanel/arm) -lmixpanel

    # Add this to your Simulator-Debug or other simulator target configs
    LIBS += -L$$quote($$BASEDIR/mixpanel/x86) -lmixpanel

USING THE SOURCES AS AN INDEPENDANT PROJECT:

To use use the mixpanel-blackberry sources directly, visit the releases page

    https://github.com/mixpanel/mixpanel-blackberry/releases

and download the source code archive associated with your release.

* Extract the contents of the archive

* In the Momentics IDE, go to

    "Import" > "General" > "Existing Projects into Workspace"

* Select the extracted directory as the root directory of the project

* Check the "Copy projects into workspace" checkbox

* Add the "mixpanel" project as a project reference in your client project.

* Add the following to the client project pro file:

    LIBS += -lbb -lcurl -lbbdata -lbbdevice
    INCLUDEPATH += $$quote(../../src)

USING THE LIBRARY:

Then you can instantiate instances of the Mixpanel class and track events with:

    #include "mixpanel/Mixpanel.h"

    ...

    mixpanel::Mixpanel tracker("YOUR PROJECT TOKEN");
    QVariantMap event_properties;
    tracker.track("My Event", event_properties);

    ...
    // Right before the main thread of your app exits
    // (or inside of a finished() signal handler on your
    // main application thread)
    mixpanel::Mixpanel::stopTracking()

The Mixpanel methods are reentrant - it's safe to have multiple instances of Mixpanel,
and use each of them from their own thread. It is *not* guaranteed that you can use
the same instance of Mixpanel from multiple threads safely.

Tracker.track takes a QString event name, and a QVariantMap of properties
to associate with the event. The map should contain only contain types
that are associated with primitive JSON values, QVariantLists and QVariantMaps;
otherwise, the values will appear as "null" in Mixpanel.

Callers should call stopTracking() just before the main thread of their application
exits. stopTracking() will wait for all outstanding tracking calls to complete,
and stop the internal tracking thread, to insure all data is written to disk or
sent to Mixpanel servers successfully.

OTHER LIBRARY FUNCTIONS:

registerSuperProperty:

You can set persistent properties that will be sent with every user event
by calling Mixpanel::registerSuperProperty(QString name, QVariant value). The given
arguments will be saved to persistent storage on the device, and sent with
every future call to track().

You can remove all previously registered super properties by calling clearSuperProperties().
clearSuperProperties() takes no arguments, and removes all super properties
from memory and persistent storage.

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


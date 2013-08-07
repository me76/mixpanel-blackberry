This is the Mixpanel client library for the Blackberry 10 operating system.

To use  this library in your project

- Add the following to the base .pro file

LIBS += -lbbdata
LIBS += -lcurl

- Drop the "mixpanel" directory into your applications src/ directory

- Add the following code to the main function of your application:

#include "mixpanel/Mixpanel.h"

Q_DECL_EXPORT int main(int argc, char **argv) {
    // Call this as early as possible in your main function.
	mixpanel::Mixpanel::init();
	
	// Your main code goes here, including your
	// program's event loop...
	
	// Call this as late as possible in your application,
	// right before your main function returns.
    mixpanel::Mixpanel::cleanup();
    
    return ret;
}

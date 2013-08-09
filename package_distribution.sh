#!/bin/bash
#
# At this writing, the build for the Mixpanel library
# is manual and IDE driven.
#
# - Import the mixpanel-blackberry directory as a Momentics IDE project
# - Right-click the project, select
#
#        Build Configurations > Build Selected...
#
# - In the pop up dialog, select "Device-Release" and "Simulator-Debug"
# - Click the "Ok" button
# - Run this script
#
rm -rf dist
mkdir dist
mkdir dist/mixpanel
mkdir dist/mixpanel/x86
mkdir dist/mixpanel/arm
mkdir dist/mixpanel/src
cp arm/libmixpanel.a dist/mixpanel/arm/
cp x86/libmixpanel.a dist/mixpanel/x86/
cp -r src/* dist/mixpanel/src
pushd dist
zip -r mixpanel-blackberry.zip mixpanel
popd

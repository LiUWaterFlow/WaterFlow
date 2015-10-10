#! /usr/bin/env bash -l

echo $OSTYPE
if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "OSX system detected - Setting up"

        echo "Copying AntTweak to /usr/local/lib/ (non overwriting)"
        cp -n lib/AntTweak/lib/libAntTweakbar.a /usr/local/lib/libAntTweakbar.a
        cp -n lib/AntTweak/lib/libAntTweakbar.dylib /usr/local/lib/libAntTweakbar.dylib

        echo "Copying the SDL frameworks"
        cp -nR lib/SDL2.framework /Library/Frameworks/SDL2.framework
else
echo "Non OSX system detected, this script does not support your system. Please refer to the wiki on https://github.com/LiUWaterFlow/WaterFlow/wiki for installation instructions for your system."
fi

#! /usr/bin/env bash -l

echo $OSTYPE
if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "OSX system detected - Setting up"
        mkdir obj
        mkdir bin
        echo "Copying AntTweak to /usr/local/lib/ (non overwriting)"
        cp -n lib/AntTweak/libAntTweakbar.a /usr/local/lib/libAntTweakbar.a
        cp -n lib/AntTweak/libAntTweakbar.dylib /usr/local/lib/libAntTweakbar.dylib

        echo "Copying the SDL frameworks, requires sudo"
        sudo cp -R lib/SDL2.framework /Library/Frameworks/
else
echo "Non OSX system detected, this script does not support your system. Please refer to the wiki on https://github.com/LiUWaterFlow/WaterFlow/wiki for installation instructions for your system."
fi

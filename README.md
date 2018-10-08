# klystrack

Klystrack is a chiptune tracker for making chiptune-like music on a modern computer.

1. Download the latest [release](https://github.com/kometbomb/klystrack/releases) or: 
    - [Build your own](https://github.com/kometbomb/klystrack/wiki/HowToCompile).
    - [Get a prebuilt](https://repology.org/metapackage/klystrack/versions).
    - [Install on Linux using the snap](https://snapcraft.io/klystrack) ([repo](https://github.com/kometbomb/klystrack-snap))
    - [Run on Linux using the AppImage](http://sid.ethz.ch/appimage/Klystrack-x86_64.AppImage)
2. Google for a klystrack tutorial or start exploring, it's mostly just like any tracker.

## Notes

### Using JACK as the audio driver

To use JACK as the audio output, start klystrack like this (assuming a global install):

```
SDL_AUDIODRIVER=jack klystrack
```

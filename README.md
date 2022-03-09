# bininside, an app that contains an app that modifies the app

simple-gui-app is a small windows app whose sole purpose is to display the number of times it was launched which is stored in a global variable. 
To update said variable it uses another app, simple-pe-editor.

# Consider last paragraph of this readme before proceeding

After gui-app closes, the following sequence of events occurs:

1. gui-app creates a copy of pe-editor in the temporary folder, creates a new process for it and closes
2. pe-editor waits until gui-app is closed, finds aforementioned global variable in image of gui-app and increments it
3. then, it creates a gui-app process and closes
4. after pe-editor is closed, gui-app removes it

So, basically, this is like carrying a small config variable within an image. Or like simply having a config file but with extra steps and bad design decisions.

## Building

1. Compile bin2hexarr and simple-pe-editor as console subsystem apps
2. Run something like `bin2hexarr simple-pe-editor.exe simple-pe-editor.hexarr`
3. Put `simple-pe-editor.hexarr` to gui-app source folder and compile it as a window subsystem app

No makefile. At all.

# Some information to consider

## Multiple instances

If the image isn't modifiable at the moment, pe-editor will omit the incrementing part and will be removed silently, indistinguishable from if it actually incremented it.

## Setup

The RVA is set to 0xcccccccc by default, this is a reserved value that makes it search first 256 bytes of specified section for global variable.
Global variable *should* be in .data section. Pointing RVA at correct global variable location will do as well.

## Accidentally deleting files
Shouldn't happen, *probably*. gui-app removes a file only when it's the exact byte copy of pe-editor it carries within itself.
pe-editor checks for 0xfadedbee signature before incrementing global variable. Should be sufficient most of the time. This is important 
because, unlike gui-app, pe-editor can't handle unicode. And, since it receives gui-app path as one of its arguments, 
launching gui-app with unicode in its path is ***bad***.

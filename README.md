<pre>
████████╗██╗   ██╗██████╗ ███╗   ██╗████████╗ █████╗ ██████╗ ██╗     ███████╗
╚══██╔══╝██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔══██╗██║     ██╔════╝
   ██║   ██║   ██║██████╔╝██╔██╗ ██║   ██║   ███████║██████╔╝██║     █████╗  
   ██║   ██║   ██║██╔══██╗██║╚██╗██║   ██║   ██╔══██║██╔══██╗██║     ██╔══╝  
   ██║   ╚██████╔╝██║  ██║██║ ╚████║   ██║   ██║  ██║██████╔╝███████╗███████╗
   ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚═════╝ ╚══════╝╚══════╝
</pre>

![staccato](https://user-images.githubusercontent.com/8061625/28444277-ae13bf92-6d81-11e7-8415-5bfe3c6a4f6d.png)

A niche music player, full of features you didn't know you ever wanted.

# Version 0.1 - Staccato

## **How to use:**
Turntable is a music player that plays back music while emulating a record player.
You can drag and drop songs to play from your file explorer onto the program to play them.
Open up the console with ~ in order to change the rpm, queue up more songs, or other features.
Feel free to leave issues in the repository for feature requests, bugs, and anything else that will help make the application better!
Most importantly, Have fun! :D

## **Controls:**
~ -> Opens up the console. This is where you can change rpm and use all of the features.
**Since UI requires an engine refactor, this will be the primary method of input until the next version.**

Spacebar -> Pause/Play the current record.

Left Arrow -> Restart the current song.

Right Arrow -> Skip to the next song. Important for if you want to move to the next song in the playlist while in loop mode.

J -> Toggles whether or not the jacket is on the record.

R -> Debug hotkey to reload the UI. Use only if you're playing around with the UI layout.

N -> Debug hotkey to view the mesh's normals.

U -> Debug hotkey to view the mesh's UVs.

B -> Debug hotkey to view the mesh with a default texture. This one really doesn't do anything anymore.

L -> Debug hotkey to unlock and lock the camera.

## **Console Commands:**

help -> Displays all of the console commands registered.

setrpm <newRPM> -> Changes the current rpm of the record to the new value.

pause -> Pauses the record's playback by setting the RPM to 0.

setvolume <0-100> -> Changes the volume percentage to the input number. Input is clamped from 0 to 100%

use33 -> Swaps out the model of the record to use a 33 and 1/3 RPM record. Slow load time right now as we don't have it threaded ;P

use45 -> Swaps out the model of the record to use a 45 RPM record.

play <filename> (rpm) -> Plays the song under the path <filename> starting at an optional (rpm), or the default record's rpm.
If you've changed the current working directory using cd, it will check for an absolute path, then the relative path.

addtoqueue <filename> -> Adds the song under the path <filename> to the back of the queue.
If you've changed the current working directory using cd, it will check for an absolute path, then the relative path.

playnext <filename> -> Same as addtoqueue, but pushes the song to the front of the queue instead of the back.

printqueue -> Prints the contents of the queue to the console.

loopoff -> Disables song looping. This is the default for the player.

loopon -> Enables song looping. Keep in mind that you'll need to turn this off or hit right arrow to play the next song in the queue.

stop -> Stops the current song and flushes the queue completely.

wigglerpm -> Toggles the wiggling of the RPM, which causes the rpm to fluctuate each frame by a delta of the wigglerate, which is 1 by default.

wigglerpm (wigglerate) -> Changes the wigglerate for the wigglerpm command.

getsongmetadata <filename> -> Prints the metadata of the song at <filename> to the console.

setbackground <background shader name> -> Changes the background to one of the shaders in the folder Data/Shaders/Backgrounds.
Some acceptable options are earthbound, albumart, rainbow, hueshift, and discopixel

playsound <filename> -> Debug command to play a sound effect in an absolute path.

clear -> Clears the console output.

quit -> Closes the application.

runfor -> Debug command to run another command multiple times.

changefont -> Debug command to change the font the console uses.

dir -> List the contents of the current working directory.

ls -> Same as dir.

cd <newDirectory> -> Changes directory to the newDirectory, appending it to the relative path.

loadmesh -> Debug command for loading in a mesh.

## **Development Team:**

**Anthony Cloudy**: Project Lead and Lead Programmer
(Twitter)[https://twitter.com/cloudygamedev]
(Github)[https://github.com/picoriley]

**Jonathan Lyttle**: Programmer
(Github)[https://github.com/picoriley]

**Regan Carver**: Programmer
(Github)[https://github.com/picoriley]

## **Special Thanks & Credits:**

**Heather Tierney**: for modeling and texturing the records and their sleeves.

**Prasanna Ravichandran**: for designing the original logo.

**Ryan See**: for creating some sound effects for the project.

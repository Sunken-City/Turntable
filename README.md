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

# Version 0.3 - Rilassato

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

play <filename> (rpm) -> Plays the song under the path <filename> starting at an optional (rpm), or the default record's rpm.
If you've changed the current working directory using cd, it will check for an absolute path, then the relative path.

addtoqueue <filename> -> Adds the song under the path <filename> to the back of the queue.
If you've changed the current working directory using cd, it will check for an absolute path, then the relative path.

playnext <filename> -> Same as addtoqueue, but pushes the song to the front of the queue instead of the back.

printqueue -> Prints the contents of the queue to the console.

printplaylists -> Prints the contents of your playlists folder.

loadplaylist <name> -> Loads a playlist with specified <name> from your playlists folder. Just the filename, please. No need to put in any '.xml' business here.

saveplaylist <name> -> Takes the current queue and saves it off as a playlist.

loopoff -> Disables song looping. This is the default for the player.

loopon -> Enables song looping. Keep in mind that you'll need to turn this off or hit right arrow to play the next song in the queue.

stop -> Stops the current song and flushes the queue completely.

wigglerpm -> Toggles the wiggling of the RPM, which causes the rpm to fluctuate each frame by a delta of the wigglerate, which is 1 by default.

wigglerpm (wigglerate) -> Changes the wigglerate for the wigglerpm command.

setrpm <rpm> -> Changes the Rotations Per Minute (RPM) of playback. The default record (45 RPM) plays back at 45 RPM. To simulate accidentally playing a 45 on 33RPM,
type in 33 to hear the exact change you would on an actual turntable. This value is unbounded, but effects peter out over a few hundred. Also keep in mind that
you can type in a negative RPM to listen backwards, or a fractional value if you'd like that much precision.

setvolume <0-100> -> Changes the volume percentage to the input number. Input is clamped from 0 to 100%

togglealbumart -> Whenever we load a song, we try to grab the album art from the song's metadata. If it doesn't exist, we procedurally
generate a new album cover based on the metadata from the song. If you'd like us to generate album art instead of attempting to load it from disk,
use this to toggle album art loading off and on.

setmusicroot <full directory path> -> Use this function to set the music root turntable can search for music in. The <full directory path> is
a music folder that contains all of your artist folders. If you have your structure set up as Music -> Artist -> Album, set the music root to
be the "Music" folder.

getmusicroot -> Tells you the currently defined root of your music folder. If you don't have a music root set yet, it'll be blank.

playmea <'song' | 'album'> -> Experimental feature, expect bugs <3. If you have a music root set (see 'setmusicroot') and have a directory
structure that's two deep (Music -> Artist -> Album, for example) then you can have Turntable randomly pick you something to play. If you type
'playmea song', then Turntable will randomly pick an Artist folder, then a random Album folder in that artist's folder, then a random song from
that folder. 'playmea album' will add the whole contents of that random Album folder to the queue. This should work in the reverse (Album -> Artist),
but currently only works for the two-tier music folder structure.

equalizer <frequency 20-22000> <range 1-5> <volume +/-dB> -> Experimental feature, expect bugs <3

use33 -> Swaps out the model of the record to use a 33 and 1/3 RPM record. Slow load time right now as we don't have it threaded ;P

use45 -> Swaps out the model of the record to use a 45 RPM record.

getsongmetadata <filename> -> Prints the metadata of the song at <filename> to the console.

printbackgrounds -> Prints the available background shaders you can use as visualizers for Turntable! It first prints out a list of shaders
that we wrote that are bundled in with Turntable in the folder Data/Shaders/Backgrounds. Then, it will print out a list of shaders inside of
the user-defined shader folder, located under the turntable appdata folder. If you put any shaders in that folder, they'll be printed here.

setbackground <background shader name> -> Changes the background to one of the shaders in the folder Data/Shaders/Backgrounds.
Some acceptable options are earthbound, albumart, rainbow, sunset, and checkersurf. If you'd like to view the available options, use the command
'printbackgrounds' to view the list of available shaders. If you add any user-defined shaders into the appdata folder, you can pick from those as well.
If there is a file in the user folder that matches one of our shaders, we'll load the one you defined instead <3

hideui -> If you want to look at your pretty shader without anything on top, you can type in this command to hide everything except for the
background layer. If you'd like to exit this mode, press any key on the keyboard to return to the console window.

stats -> Prints out your current listening stats! This includes your current level, your title, how much experience you have, how many tokens you have
available to spend (you can't spend any yet, that's a future update), your total number of playcounts you've created listening to music in turntable,
and how long you've listened to music in Turntable in total. The format is Hours:Minutes:Seconds. The leveling is purely cosmetic right now, but we have plans.

nextlevel -> Prints out the amount of experience you need to earn to get to the next level. Keep on listening!

playsound <filename> -> Debug command to play a sound effect in an absolute path.

clear -> Clears the console output.

quit -> Closes the application.

dir -> List the contents of the current working directory.

ls -> Same as dir.

cd <newDirectory> -> Changes directory to the newDirectory, appending it to the relative path.

runfor -> Debug command to run another command multiple times.

changefont -> Debug command to change the font the console uses.

loadmesh -> Debug command for loading in a mesh. Don't waste your time with this one, we barely understand how it works either ;)

printlevels -> Debug command that prints out the the amount of experience you need to earn to get to each of the first 100 levels.
Here's a way to peek at some of the names.

addexp <number of experience points to add> -> Debug command to add exp to your profile.
Go ahead and do this if you want to ruin your experience, you cheater.

## **User-defined shaders:**
This release, we added a system to help add new background shaders in Turntable. We now support *most* of the uniforms that ShaderToy uses.
There are some bugs, but for the moment if you grab a shader that doesn't have any channel usage or multiple passes, there's a good chance you
can use it! If you're down here, you probably know a little about what you're doing, so if you have any bugs in glsl compilation, we'll spit out what
the issue is before crashing, so feel free to modify some shaders to make them work <3

*A few things that definitely don't work:*
- Multi-pass shaders
- Any channel usage beyond iChannel0. iChannel0 is currently supplied a copy of the album art (that's upside down. long story).
- We don't yet pass fractional seconds for dates. Use iTime instead if you want the fractional part of time passed.
- iTime is the time that's passed since the start of the program, not since the shader restarted.
- iChannelTime[1] currently has the time into the song you're listening to, but don't rely on this. Everything else in this uniform is garbage.
- iChannelResolution is also bogus.
- No keyboard support, but mouse should be working to spec.

*If you have a gray screen instead of your shader:*
  Some authors on ShaderToy only output fragColor.rgb or fragColor.xyz, missing the alpha component.
  If they didn't add an alpha component, you need to add a line like 'fragColor.a = 1.0;' to the end of the shader to ensure it renders.

## **Development Team:**

**Anthony Cloudy**: Project Lead and Lead Programmer

[Twitter](https://twitter.com/cloudygamedev)

[Github](https://github.com/picoriley)

**Jonathan Lyttle**: Programmer

[Github](https://github.com/weruder)

## **Special Thanks & Credits:**

**Heather Tierney**: for 3D modeling and texturing the records and their sleeves.

[Portfolio](www.heathertierney.com)

**Prasanna Ravichandran**: for designing the original logo.

[Portfolio](www.prasannar.com)

**Ryan See**: for creating some sound effects for the project.

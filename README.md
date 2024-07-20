# desert-moon
A program to produce a suite of images for boot screens, wallpapers, website banners.

### teaser

![desert-moonrise](https://01micko.github.io/artwork/desert-moonriseFHD.png)

YT vid for mock *plymouth* animation

[desert-moonrise MKIII](https://youtu.be/llRHfF6z-qM)

### still WIP

This program should not be considered stable. While it shouldn't destroy anything
there's no warranty under the terms of the GPL.

More to come.

### why?

Because of debian 13 up coming release.

### but why code and not *gimp*, *magick* or other ?

Glutton for punishment

### build

Just run:

```
make
```

Don't bother to install, everyting should work from `pwd`

If built without error run:

```
./buildwalls
```

Everything will be located in the subdir `ff`

### dependencies

 - cairo
 - pango
 - gcc
 - bash
 - netpbm
 - ppmtolss16
 - ffmpeg (for animation)

### To Do

 - [ ] code clean up
 - [ ] manual page
 - [ ] bug fixes

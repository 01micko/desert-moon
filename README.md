# desert-moon
A program to produce a suite of images for boot screens, wallpapers, website banners and more.

### teaser

![desert-moonrise](https://01micko.github.io/artwork/desert-moonriseFHD.png)

YT vid for mock *plymouth* animation

[desert-moonrise MKIII](https://youtu.be/llRHfF6z-qM)

DebianArt debian 13 art submission

[Desert Moonrise](https://wiki.debian.org/DebianArt/Themes/desert-moonrise)

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

Don't bother to install, everything should work from `pwd`

If built without error run:

```
./buildwalls
```

Everything will be located in the subdir `debian/`

The script `buildwalls` can be edited to add custom fonts and tweaking of the name and logo.

### dependencies

 - cairo (build)
 - pango (build)
 - gcc (build)
 - bash
 - netpbm
 - ppmtolss16
 - ffmpeg (for animation - optional)
 - rsvg
 - Poppl-Laudatio or Prociono or another Serif font
 - [font-logos](https://github.com/Lukas-W/font-logos) or perhaps fontawesome

### To Do

 - [x] code clean up
 - [x] manual page
 - [ ] bug fixes
 - [ ] more error checking

# desert-moon
A program to produce a suite of images for boot screens, wallpapers, plymouth, website banners and more.

### teaser

![desert-moonrise](https://01micko.github.io/artwork/desert-moonriseFHD.png)

YT vid for mock *plymouth* animation

[desert-moonrise MKIII](https://youtu.be/llRHfF6z-qM)

DebianArt debian 13 art submission

[Desert Moonrise](https://wiki.debian.org/DebianArt/Themes/desert-moonrise)

[Plymouth repository](https://github.com/01micko/desert-moonrise-plymouth) - tested
working at FHD and 1366x768 resolutions.

This program should not be considered stable. While it shouldn't destroy anything
there's no warranty under the terms of the GPL.

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

The script `buildwalls` can be edited to add custom fonts and tweaking of the name, distro and logo.

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
 - [font-logos](https://github.com/Lukas-W/font-logos) or perhaps fontawesome or
 other icon font.

### known issues

On some systems, the font logo shows up as the hex code when running `buildwalls`. There is
a work around to alleviate this in a second script called `buildwalls-alt` that has the icon
glyphs pasted into place. Not all distros are covered of course but you can paste them in
yourself if you open up a word processor and use your icon font of choice in that and paste
appropriate characters into the script.

### To Do

 - [x] code clean up
 - [x] manual page
 - [ ] bug fixes
 - [ ] more error checking

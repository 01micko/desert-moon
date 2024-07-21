moonwall(1)

# NAME

moonwall - a wallpaper and associated images generator for linux

# SYNOPSIS

*moonwall* [options...]

# DESCRIPTION

Generate SVG or PNG Linux wallpapers.
The name is as it implies: creates a moon wallpaper.


# OPTIONS

*-n*

	image file name

*-d*

	destination directory: (default: $HOME)

*-L*

	Main label on image - quoted if there are spaces

*-S*

	Second label on image - quoted if there are spaces

*-T*

	Third label on image - quoted if there are spaces

*-l*

	main ttf font family and size - quoted 

*-s*

	second ttf font family and size- quoted 

*-t*

	third ttf font family and size - quoted

*-j*

	main font position - quoted

*-k*

	second font position - quoted

*-m*

	third font position - quoted
	
*-X* 

	floating point sRGBA, quoted, space delimited values for the main
	font color

*-Y*

	floating point sRGBA, quoted, space delimited values for the second
	font color

*-Z*

	floating point sRGBA, quoted, space delimited values for the main
	font color
	Color arg example: -X "0.1 0.2 0.3 0.4"

*-i*

	"/path/to/icon.png x y", for an optional icon

*-e*

	factor for moonrise 0-200, 100 is default

*-c*

	centres icon and/or text

*-x*

	width of image in pixels

*-y*

	height of image in pixels

*-r*

	0 to 2, 0 is default; prints the stars in a random pattern.
	*-r1* uses the default config to place the stars
	*-r2* uses a random pattern but prints that to stdout
	NOTE: only works with "-b0" option as all stars are produced.

*-b*

	0 to 2, 0 is the default; if "-b" is not specified 0 is a moon,
	1 and 2 - see what happens.

*-p*

	save to PNG, SVG is the default.

*-a*

	use a specified color from the config for the foreground.
*-h*

	show help and exit.

*-v*

	how version and exit.

# AUTHOR

Mick Amadio - https://01micko.github.io

# BUGS

Please report any bugs at:

https://github.com/01micko/desert-moon/issues

# NOTES

The main and secondary fonts are hidden behind the foreground.
This is by design, and requests to change will not be considered.
If you need text on the foreground, use the "-T", third font option.

# SEE ALSO

mkwallpaper(1)


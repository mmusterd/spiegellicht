# spiegellicht

Goal of the project is to have a mirror with a phone-controlled ledstrip.
The hardware is:
* NodeMCU board with wifi
* 3 meter (180led) ws2812 led strip
* MAX4466 microphone 

It will initially feature the following modi:
* two-tone backlight, where left and right side of the mirror have a different hue.
* gradient backlight, where the two hues will blend
* simple backlight, where all leds have the same color
* audio mode, where the microscope will act as a VU-meter left and right
* spectrum mode, where the microscope spectrum will result in three bands left and right

The phone app shall allow for the following:
* switch between the modi
* allow setting the left and the right hue, or leaving it random
* Overall lightness of the strip

# technology used


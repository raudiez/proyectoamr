#! /usr/bin/env python
#
# Set an RGB LED to either red, green, or blue.
#
# Usage:
#   sudo python rgb.py [color]
#
# @author Jeff Geerling, 2015

import argparse
import RPi.GPIO as GPIO

# Get RGB colors from command line arguments.
parser = argparse.ArgumentParser(description = 'Add a little color to your life.')
parser.add_argument('color', metavar='color', type=str, nargs=1,
                   help='A color value of red, green, blue, or off.')
args = parser.parse_args()

# LED pin mapping.
red = 18
green = 27
blue = 17

# GPIO Setup.
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

GPIO.setup(red, GPIO.OUT)
GPIO.setup(green, GPIO.OUT)
GPIO.setup(blue, GPIO.OUT)

# Set up colors using PWM so we can control individual brightness.
RED = GPIO.PWM(red, 100)
GREEN = GPIO.PWM(green, 100)
BLUE = GPIO.PWM(blue, 100)
RED.start(0)
GREEN.start(0)
BLUE.start(0)

# Set a color by giving R, G, and B values of 0-255.
def setColor(rgb = []):
    # Convert 0-255 range to 0-100.
    rgb = [(x / 255.0) * 100 for x in rgb]
    RED.ChangeDutyCycle(rgb[0])
    GREEN.ChangeDutyCycle(rgb[1])
    BLUE.ChangeDutyCycle(rgb[2])

setColor(args.rgb)
time.sleep(10)

GPIO.cleanup()

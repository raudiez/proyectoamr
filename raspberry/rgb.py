#! /usr/bin/env python
#
# Set an RGB LED to either red, green, or blue.
#
# Usage:
#   sudo python rgb.py [color]
#
# @author Jeff Geerling, 2015

#import argparse
import time
import RPi.GPIO as GPIO

# Get RGB colors from command line arguments.
#parser = argparse.ArgumentParser(description = 'Light up the world!')
#parser.add_argument('rgb', metavar='0-255', type=int, nargs=3,
#                   help='Red, Green, and Blue color values (0-255).')
#args = parser.parse_args()

# LED pin mapping.
red = 18
green = 27 #Pin 21 en Raspi B. Pin 27 en B+.
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
RED.start(100)
GREEN.start(100)
BLUE.start(100)

# Set a color by giving R, G, and B values of 0-255.
def setColor(rgb = []):
	print rgb
	# Convert 0-255 range to 0-100.
	rgb = [(abs(x-255) / 255.0) * 100 for x in rgb]
	RED.ChangeDutyCycle(rgb[0])
	GREEN.ChangeDutyCycle(rgb[1])
	BLUE.ChangeDutyCycle(rgb[2])

try:
	rgbled = [255,0,0]
	setColor(rgbled)
	time.sleep(5)
except (KeyboardInterrupt, SystemExit):
	raise

GPIO.cleanup()

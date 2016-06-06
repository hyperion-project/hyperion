import hyperion 
import time 
import datetime 
import colorsys
from PIL import Image, ImageDraw, ImageColor
import math

# Get the parameters
sleep_time = float(hyperion.args.get('sleep_time', 1.0))
img_width = int(hyperion.args.get('img_width', 320))
img_height = int(hyperion.args.get('img_height', 240))

hour_color = tuple(hyperion.args.get('hour_color', (0,0,96)))
minute_color = tuple(hyperion.args.get('minute_color', (96,96,0)))
second_color = tuple(hyperion.args.get('second_color', (96,0,0)))

clock_style = hyperion.args.get('clock_style', "hands")

# override for development
#clock_style = "pie_hands"

xcenter = img_width/2
ycenter = img_height/2

# Create image object and drawing object
im = Image.new("RGB",(img_width, img_height))
draw = ImageDraw.Draw(im)

# how long are the hands (when drawn as lines)
hour_len = img_height/3
min_len = img_height/2
sec_len = img_height

# when we use drawpie for hands, how many degrees wide is it?
pie_hand_width = 3

def mainloop():
# Start the write data loop
	while not hyperion.abort():
		if clock_style == "hands":
			clock_hands()
		if clock_style == "pie":
			clock_pie()
		if clock_style == "pie_hands":
			clock_pie_hands()

		hyperion.setImage(img_width, img_height, bytearray(im.tobytes("raw", "RGB", 0, 1)))

		time.sleep( sleep_time )

def clock_hands():
	time_time = datetime.datetime.now().time()
	time_hour = float(time_time.strftime("%I"))	# 12 hr time
	time_min = float(time_time.strftime("%M"))
	time_sec = float(time_time.strftime("%S"))
	time_milli = float(time_time.strftime("%f"))/1000

	time_sec += time_milli/1000
	time_min += time_sec/60
	time_hour += time_min/60
	
	hour_rad = math.radians(90 - (time_hour * 30))
	min_rad = math.radians(90 - (time_min * 6))
	sec_rad = math.radians(90 - (time_sec * 6))

	draw.rectangle( [0,0,img_width,img_height], fill=(0,0,0), outline=(0,0,0) )
	draw.line( (xcenter, ycenter, int(xcenter+hour_len*math.cos(hour_rad)), int(ycenter-hour_len*math.sin(hour_rad))), fill=(hour_color))
	draw.line( (xcenter, ycenter, int(xcenter+min_len*math.cos( min_rad)),   int(ycenter-min_len*math.sin(min_rad)) ), fill=(minute_color))
	draw.line( (xcenter, ycenter, int(xcenter+sec_len*math.cos( sec_rad)),   int(ycenter-sec_len*math.sin(sec_rad)) ), fill=(second_color))
#	draw.setPixel( ( int(xcenter+16*math.cos( sec_rad)), int(ycenter-16*math.sin(sec_rad)) ), second_color)




def clock_pie_hands():
	time_time = datetime.datetime.now().time()
	time_hour = float(time_time.strftime("%I"))	# 12 hr time
	time_min = float(time_time.strftime("%M"))
	time_sec = float(time_time.strftime("%S"))
	time_milli = float(time_time.strftime("%f"))/1000

	time_sec += time_milli/1000
	time_min += time_sec/60
	time_hour += time_min/60
	
	hour_deg = int(time_hour * 30 - 90)
	min_deg  = int(time_min * 6 - 90)
	sec_deg  = int(time_sec * 6 - 90)

	draw.rectangle( [0,0,img_width,img_height], fill=(0,0,0), outline=(0,0,0) )
	draw.pieslice( (-img_width, -img_height, img_width*2, img_height*2), hour_deg-pie_hand_width, hour_deg+pie_hand_width, fill=(hour_color), outline=(hour_color))
	draw.pieslice( (-img_width, -img_height, img_width*2, img_height*2),  min_deg-pie_hand_width, min_deg+pie_hand_width, fill=( minute_color), outline=(minute_color))
	draw.pieslice( (-img_width, -img_height, img_width*2, img_height*2), sec_deg-pie_hand_width, sec_deg+pie_hand_width, fill=( second_color), outline=(second_color))




def clock_pie():
	time_time = datetime.datetime.now().time()
	time_hour = float(time_time.strftime("%I"))	# 12 hr time
	time_min = float(time_time.strftime("%M"))
	time_sec = float(time_time.strftime("%S"))
	time_milli = float(time_time.strftime("%f"))/1000

	time_sec += time_milli/1000
	time_min += time_sec/60
	time_hour += time_min/60
	
	hour_deg = int(time_hour * 30 - 90)
	min_deg  = int(time_min * 6 - 90)
	sec_deg  = int(time_sec * 6 - 90)

	draw.rectangle( [0,0,img_width,img_height], fill=(0,0,0), outline=(0,0,0) )
	draw.pieslice( (-img_width, -img_height, img_width*2, img_height*2),  min_deg, hour_deg, fill=(hour_color), outline=(hour_color))
	draw.pieslice( (-img_width, -img_height, img_width*2, img_height*2), hour_deg, min_deg, fill=( minute_color), outline=(minute_color))
	draw.pieslice( (-img_width, -img_height, img_width*2, img_height*2), sec_deg-pie_hand_width, sec_deg+pie_hand_width, fill=( second_color), outline=(second_color))


if __name__=="__main__":
   mainloop()

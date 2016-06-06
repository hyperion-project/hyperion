import hyperion 
import time 
import datetime 
import colorsys
import gd
import math

# Get the parameters
sleep_time = float(hyperion.args.get('sleep_time', 1.0))
img_width = int(hyperion.args.get('img_width', 320))
img_height = int(hyperion.args.get('img_height', 240))
hour_color = hyperion.args.get('hour_color', (255,0,0))
minute_color = hyperion.args.get('minute_color', (255,0,0))
clock_style = hyperion.args.get('clock_style', "hands")

#img_width = 64
#img_height = 36

xcenter = img_width/2
ycenter = img_height/2
center = (xcenter, ycenter)

im = gd.image((img_width, img_height))
hour_color = im.colorAllocate(hour_color)
min_color = im.colorAllocate(minute_color)
sec_color = im.colorAllocate((255,0,0))
black_color = im.colorAllocate((0,0,0))
red_color = im.colorAllocate((255,0,0))

hour_len = img_height/3
min_len = img_height/2
sec_len = img_height

# Initialize the led data
imageData = bytearray(img_width *img_height * (0,0,0))

def mainloop():
# Start the write data loop
	while not hyperion.abort():
		clock_hands()
		iD_ptr = 0;
#		print datetime.datetime.now().isoformat()
		for y in range (0,img_height):
			for x in range (0,img_width):
				this_pix = im.colorComponents(im.getPixel((x,y)))
				imageData[iD_ptr+0] = this_pix[0]
				imageData[iD_ptr+1] = this_pix[1]
				imageData[iD_ptr+2] = this_pix[2]
				iD_ptr += 3
		print iD_ptr
#		print datetime.datetime.now().isoformat()
		hyperion.setImage(img_width, img_height, imageData)
#		print datetime.datetime.now().isoformat()
#		print ""
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
	
	print time_sec
	hour_rad = math.radians(90 - (time_hour * 30))
	min_rad = math.radians(90 - (time_min * 6))
	sec_rad = math.radians(90 - (time_sec * 6))

	im.rectangle((0,0),(img_width,img_height),black_color, black_color)
	im.line( center, ( int(xcenter+hour_len*math.cos(hour_rad)), int(ycenter-hour_len*math.sin(hour_rad)) ), hour_color)
	im.line( center, ( int(xcenter+min_len*math.cos( min_rad)), int(ycenter-min_len*math.sin(min_rad)) ), min_color)
	im.line( center, ( int(xcenter+sec_len*math.cos( sec_rad)), int(ycenter-sec_len*math.sin(sec_rad)) ), sec_color)
#	im.setPixel( ( int(xcenter+16*math.cos( sec_rad)), int(ycenter-16*math.sin(sec_rad)) ), sec_color)


if __name__=="__main__":
   mainloop()

import hyperion
import time
import colorsys

# Get the parameters
sleepTime = float(hyperion.args.get('sleepTime', 1.0))

colourPattern = []
colourPattern.append( bytearray((int(255), int(0), int(255))) )
colourPattern.append( bytearray((int(0), int(0), int(0))) )
colourPattern.append( bytearray((int(255), int(128), int(128))) )
colourPattern.append( bytearray((int(0), int(0), int(0))) )
colourPattern.append( bytearray((int(0), int(255), int(255))) )
colourPattern.append( bytearray((int(0), int(0), int(0))) )


ledData2 = bytearray()
for i in range(hyperion.ledCount):
	offset = 4;
	ledData2 += colourPattern[(offset+i)%6]

# Start the write data loop
while not hyperion.abort():
	for offset in range(0,6):
		ledData = bytearray()
		for i in range(hyperion.ledCount):
			ledData += colourPattern[(offset+i)%6]
		hyperion.setColor(ledData)
		time.sleep(sleepTime)


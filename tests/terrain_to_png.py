from PIL import Image
import numpy as np
import sys

# Load the heightmap data from the C++ code
sys.argv.append("C:\\Users\\ninja\\source\\repos\\TransportTycoon")
before = np.loadtxt(sys.argv[1] + "/tests/before.map", delimiter=',')
after = np.loadtxt(sys.argv[1] + "/tests/after.map", delimiter=',')

# Normalize the heightmap values between 0 and 255
min_value = np.min(after)
max_value = np.max(after)
before = (before - min_value) / (max_value - min_value) * 255

# Convert the heightmap to unsigned 8-bit integer values
before = before.astype(np.uint8)

# Create an image from the heightmap data
beforeimg = Image.fromarray(before, mode='L')

# Save the image
beforeimg.save(sys.argv[1] + "/tests/before.png")

# Normalize the heightmap values between 0 and 255
after = (after - min_value) / (max_value - min_value) * 255

# Convert the heightmap to unsigned 8-bit integer values
after = after.astype(np.uint8)

# Create an image from the heightmap data
afterimg = Image.fromarray(after, mode='L')

# Save the image
afterimg.save(sys.argv[1] + "/tests/after.png")

finalimg = Image.new("RGB", (afterimg.size[0]*2, afterimg.size[1]))

finalimg.paste(beforeimg, (0,0))
finalimg.paste(afterimg, (beforeimg.size[0], 0))

finalimg.save(sys.argv[1] + "/tests/comparison.png")
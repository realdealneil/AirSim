#gst-launch-1.0.exe dx9screencapsrc x=639 y=0 width=1280 height=720 ! videoconvert ! video/x-raw,format=I420,framerate=30/1 ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=192.168.50.226 port=5004
#gst-launch-1.0 videotestsrc ! video/x-raw,format=GRAY8,width=256,height=144 ! videoconvert ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=192.168.50.226 port=5004
#gst-launch-1.0 videotestsrc ! video/x-raw,format=RGB,width=256,height=144 ! videoconvert ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=192.168.50.226 port=5004

### This works as a test sending pipeline:
#gst-launch-1.0 -v videotestsrc pattern=ball ! x264enc tune=zerolatency ! rtph264pay ! udpsink host=192.168.50.226 port=5004

# Now, send using the correct size and monochrome:
# Note that we convert to grayscale, but then to I420 so that the h264 encoder can accept it:
gst-launch-1.0 -v videotestsrc is-live=true do-timestamp=true ! \
 video/x-raw,format=GRAY8,width=256,height=144 ! \
 videoconvert ! video/x-raw,format=I420,width=256,height=144 ! \
 x264enc tune=zerolatency ! rtph264pay ! \
 udpsink host=192.168.50.226 port=5004

# The receiving pipeline on the Jetson TK1 is as follows:
#gst-launch-1.0 udpsrc port=5004 ! "application/x-rtp" ! rtph264depay ! h264parse ! omxh264dec ! nveglglessink

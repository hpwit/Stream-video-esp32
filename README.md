# Stream-video-esp32
For it to work you will need to install ffmpeg www.ffmpeg.org 
the StreamVideo has been compile on Mac OS x to recompile it use compile.sh

USAGE:
./StreamVideo -i 'inputfile' -a 'address_to_stream_to' -d 'dimension of the panel width x height' 
example:
./StreamVideo -i test.mp4 -a 192.168.1.23 -d 120x34

To modifify the rotation of the image in regards to your panel use the option -w

OPTIONS ARE:
 TOP_LEFT,TOP_RIGHT,DOWN_RIGHT,DOWN_LEFT by default it is set as DOWN_LEFT
 
 example:
./StreamVideo -i test.mp4 -a 192.168.1.23 -d 120x34 -w TOP_LEFT

you can also set the brigthness using the -b option
example:
./StreamVideo -i test.mp4 -a 192.168.1.23 -d 120x34 -w TOP_LEFT -b 120

The input file can also be a URL to an online video.

If you want to stream Youtube video you need to install youtube-dl
brew install youtube-dl
and then do:
./StreamVideo -i $(youtube-dl -f bestvideo -g  https://youtu.be/CZ9Pu9Usk5o) -d 123x48 -a 192.168.1.22

OTHER OPTIONS:
By default the program will resize the video to match you led panel to change this you can use -s scalewidthxscaleheight
For instance you could stream a image double the size of your panel by:
./StreamVideo -i test.mp4 -a 192.168.1.23 -d 120x34 -s 240x68

then you can move around the picture using the option -o

./StreamVideo -i test.mp4 -a 192.168.1.23 -d 120x34 -s 240x68 -o 100x20




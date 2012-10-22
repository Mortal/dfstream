echo "Launching xwininfo to grab window information. Click on your Dwarf Fortress window." >&2
XW="`xwininfo`"
getparam() {
	echo "$XW" | grep "$1" | grep -o '[0-9]*'
}
X=`getparam "Abs.*X"`
Y=`getparam "Abs.*Y"`
W=`getparam Width`
H=`getparam Height`
echo "Dwarf Fortress buffer at $X,$Y width $W height $H" >&2
echo "Launch ffmpeg and output PPM" >&2
ffmpeg -v error -f x11grab -r 20 -s ${W}x${H} -i $DISPLAY+$X,$Y -f rawvideo -vcodec ppm -an - | ./compressor | nc 127.0.0.1 8009

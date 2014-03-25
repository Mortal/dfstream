import sys
import os
import re
import subprocess

cp437 = \
        " ☺☻♥♦♣♠•◘○◙♂♀♪♫☼"
        "►◄↕‼¶§▬↨↑↓→←∟↔▲▼"
        " !\"#$%&'()*+,-./"
        "0123456789:;<=>?"
        "@ABCDEFGHIJKLMNO"
        "PQRSTUVWXYZ[\\]^_"
        "`abcdefghijklmno"
        "pqrstuvwxyz{|}~⌂"
        "ÇüéâäàåçêëèïîìÄÅ"
        "ÉæÆôöòûùÿÖÜ¢£¥₧ƒ"
        "áíóúñÑªº¿⌐¬½¼¡«»"
        "░▒▓│┤╡╢╖╕╣║╗╝╜╛┐"
        "└┴┬├─┼╞╟╚╔╩╦╠═╬╧"
        "╨╤╥╙╘╒╓╫╪┘┌█▄▌▌▄"
        "αßΓπΣσµτΦΘΩδ∞φε∩"
        "≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ "

def get_wininfo():
    print("Running xwininfo...")
    s = subprocess.check_output(('xwininfo',),
            stdin=subprocess.DEVNULL).decode()
    print("")

    def getparam(r):
        o = re.search(r, s)
        return o.group(1)

    x = getparam(r'Absolute upper-left X:[ \t]+(\d+)')
    y = getparam(r'Absolute upper-left Y:[ \t]+(\d+)')
    w = getparam(r'Width:[ \t]+(\d+)')
    h = getparam(r'Height:[ \t]+(\d+)')

    return x, y, w, h

def get_ffmpeg_x11grab_command(wininfo, options=()):
    x, y, w, h = wininfo
    return ('ffmpeg',
            '-v', 'error',
            '-f', 'x11grab',
            '-s', '%sx%s' % (w, h),
            '-i', '%s+%s,%s' % (os.getenv('DISPLAY'), x, y),
            ) + tuple(options) + (
            '-f', 'rawvideo',
            '-vcodec', 'ppm',
            '-an',
            '-')

def fetch_frames(wininfo):
    ffmpeg = subprocess.Popen(get_ffmpeg_x11grab_command(wininfo),
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE)
    compressor = subprocess.Popen(('./compressor',),
            stdin=ffmpeg.stdout,
            stdout=subprocess.PIPE)
    def frames():
        def sized_read(n):
            b = compressor.stdout.read(n)
            if len(b) != n:
                raise IOError("Could only read %d bytes; expected %d"
                        % (len(b), n))
            return b

        def unpack(fmt):
            sz = struct.calcsize(fmt)
            return struct.unpack(fmt, sized_read(sz))

        current_frame = None

        while True:
            s_len, = unpack('!I')
            s = sized_read(s_len).decode()
            canvaswidth, canvasheight, outputcol, \
                outputrow, outputwidth, outputheight = \
                map(int, s.split())

            grid = (np.array(sized_read(outputwidth*outputheight), np.uint8)
                    .reshape((outputheight, outputwidth, 2)))

            if current_frame is None:
                current_frame = np.ndarray((canvasheight, canvaswidth, 2), np.uint8)

            r1 = outputrow
            r2 = r1 + outputheight
            c1 = outputcol
            c2 = c1 + outputwidth
            current_frame[r1:r2, c1:c2, :] = grid

            yield current_frame

    return ffmpeg, compressor, frames

def fetch_frame(wininfo):
    print("Fetching frame with ffmpeg x11grab...")
    s = subprocess.check_output(
            get_ffmpeg_x11grab_command(wininfo, ('-frames', '1')),
            stdin=subprocess.DEVNULL)
    print("")

    print("Parsing frame using ./compressor...")
    p = subprocess.Popen(('./compressor',),
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            )
    stdoutdata, stderrdata = p.communicate(input=s)
    p.wait()
    frame = stdoutdata
    print("")

    print("Converting frame using ./uncompress...")
    p = subprocess.Popen(('./uncompress',),
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            )
    stdoutdata, stderrdata = p.communicate(input=frame)
    p.wait()
    print("")

    return stdoutdata.decode('utf_16_le')

def main():
    wininfo = get_wininfo()

    lines = fetch_frame(wininfo).splitlines()
    print(lines)

    name_match = re.search(r'  (\w.*), "(.*)", .*', lines[0])
    name = name_match.group(1)
    nametr = name_match.group(2)
    print('DWARF: %s' % name)
    words = '\n'.join(s[2:-2] for s in lines[2:-1]).split()
    sentences = [[]]
    for word in words:
        sentences[-1].append(word)
        if word.endswith('.'):
            sentences.append([])
    sentences = sentences[:-1]

    s = ' '.join(' '.join(sentence) for sentence in sentences)
    for attr in BODY_ATTRIBUTES:
        title, description, table = attr
        value = None
        vdesc = None
        for each, interval in table:
            if each in s:
                value = interval
                vdesc = each
                break
        print("%s: %s %s" % (title, vdesc, value))

###############################################################################
# Dwarven body attributes
#
# From http://dwarffortresswiki.org/index.php/DF2012:Attribute#Body_Attributes

# The attribute values given in the tables correspond to default racial averages
# for dwarves only. Modded dwarves or any other creature will report the same
# phrases, but the underlying numbers may be very different. In almost every
# case, a higher attribute value is better, as opposed to personality traits
# where one may prefer higher or lower depending on the situation.

def parse_attribute_table():
    def attribute_table(s):
        lines = [line.split('\t') for line in filter(lambda s: s, s.splitlines())]
        values, descriptions = zip(*lines)
        intervals = [v.split(' - ') if ' - ' in v else v for v in values]
        return sorted(zip(descriptions, intervals), key=lambda o: -len(o[0]))

    return (
('Strength',
"""
Alters the damage done in melee (increases velocity of weapon swings),
increases muscle mass (thicker muscle layer also resists damage more), and
increases how much a creature can carry. Higher strength also increases the
speed with which a creature, even a naked creature, may move.
""",
attribute_table("""
2250 - 5000 	unbelievably strong
2000 - 2249 	mighty
1750 - 1999 	very strong
1500 - 1749 	strong
751 - 1000 	weak
501 - 750 	very weak
251 - 500 	unquestionably weak
0 - 250 	unfathomably weak
""")),
('Agility',
"""
This attribute increases the speed at which a creature works in the same way as
strength -- a creature with maximum agility and strength can move around three
times faster than a creature with minimum agility and strength.
""",
attribute_table("""
1900 - 5000 	amazingly agile
1650 - 1899 	extremely agile
1400 - 1649 	very agile
1150 - 1399 	agile
401 - 650 	clumsy
151 - 400 	quite clumsy
0 - 150 	totally clumsy
NULL 	abysmally clumsy
""")),
('Toughness',
"""
Reduces physical damage.
""",
attribute_table("""
2250 - 5000 	basically unbreakable
2000 - 2249 	incredibly tough
1750 - 1999 	quite durable
1500 - 1749 	tough
751 - 1000 	flimsy
501 - 750 	very flimsy
251 - 500 	remarkably flimsy
0 - 250 	shockingly fragile
""")),
('Endurance',
"""
Reduces the rate at which dwarves become exhausted.
""",
attribute_table("""
2000 - 5000 	absolutely inexhaustible
1750 - 1999 	indefatigable
1500 - 1749 	very slow to tire
1250 - 1499 	slow to tire
501 - 750 	quick to tire
251 - 500 	very quick to tire
1 - 250 	extremely quick to tire
0 	truly quick to tire
""")),
('Recuperation',
"""
Increases the rate of wound healing.
""",
attribute_table("""
2000 - 5000 	possessed of amazing recuperative powers
1750 - 1999 	incredibly quick to heal
1500 - 1749 	quite quick to heal
1250 - 1499 	quick to heal
501 - 750 	slow to heal
251 - 500 	very slow to heal
1 - 250 	really slow to heal
0 	shockingly slow to heal
""")),
('Disease Resistance',
"""
Reduces the risk of disease.
""",
attribute_table("""
2000 - 5000 	virtually never sick
1750 - 1999 	almost never sick
1500 - 1749 	very rarely sick
1250 - 1499 	rarely sick
501 - 750 	susceptible to disease
251 - 500 	quite susceptible to disease
1 - 250 	really susceptible to disease
0 	stunningly susceptible to disease 
""")),
)

BODY_ATTRIBUTES = parse_attribute_table()

if __name__ == '__main__':
    main()


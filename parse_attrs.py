import sys
import os
import re
import subprocess
import struct
import numpy as np

cp437 = (
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
    "≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ")

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
            '-r', '1',
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
            b = sized_read(sz)
            return struct.unpack(fmt, b)

        current_frame = None

        while True:
            s_len, = unpack('!I')
            s = sized_read(s_len)
            nl = s.find(b'\n')
            canvaswidth, canvasheight, outputcol, \
                outputrow, outputwidth, outputheight = \
                map(int, s[:nl].decode().split())
            s = s[nl+1:]
            if len(s) != outputwidth*outputheight*2:
                raise IOError("Expected length %d, got %d"
                        % (outputwidth*outputheight, len(s)))

            grid = (np.array(list(s), np.uint8)
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

def parse_dwarf_attribute(frame):
    lines = frame.splitlines()
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

def dfdecode(b):
    return ''.join(cp437[c] for c in b)

def test_divider(frame, x):
    column = frame[1:-1, x, :]
    chars = column[:, 0]
    colors = column[:, 1]
    if np.count_nonzero(chars) != 0:
        return False
    if np.count_nonzero(colors & 0xF0 != 0x70) != 0:
        return False
    return True

def interpret_first_row(first_row):
    text = first_row[:,0]
    colors = first_row[:,1]
    bgcolors = colors & 0xF0

    title_index = (bgcolors == 0x80)
    paused_index = (bgcolors == 0x20)
    fps_index = (bgcolors == 0x60)
    idlers_index = (bgcolors == 0x30)

    title = dfdecode(text[title_index]).strip()
    paused = dfdecode(text[paused_index]).strip()
    fps = dfdecode(text[fps_index]).strip()
    idlers = dfdecode(text[idlers_index]).strip()

    paused = paused == '*PAUSED*'

    if fps:
        s, n1, n2 = fps.split()
        if s == 'FPS:':
            fps = int(n1)
        else:
            raise Exception("FPS string is [%s]" % s)
    else:
        fps = None

    if idlers:
        s, n = idlers.split()
        if s == 'Idlers:':
            idlers = int(n)
        else:
            raise Exception("Idlers string is [%s]" % s)
    else:
        idlers = None

    return title, paused, fps, idlers

def interpret_frame(frame):
    title, paused, fps, idlers = interpret_first_row(frame[0,:,:])
    o = {
        'title': title,
        'paused': paused,
        'fps': fps,
        'idlers': idlers,
        'window': None,
        'z': None,
    }

    z_str = dfdecode(frame[-5:-1,-1,0]).strip()

    if title == 'Dwarf Fortress' and z_str != '':
        z = int(z_str)
        divider, *_ = tuple(filter(lambda d: test_divider(frame, d),
                (-56, -32, -25))) or (-1,)
        window = frame[1:-1, 1:divider, :]
        o['window'] = window
        o['z'] = z

    return o

def correlate_slices(n, m):
    if n > m:
        for s1, s2 in correlate_slices(m, n):
            yield s2, s1
        return
    # n <= m
    for i in range(1, n):
        yield i - 1, slice(n - i, n), slice(0, i)
    for i in range(n, m):
        yield i - 1, slice(0, n), slice(i, i + n)
    for i in range(0, n - 1):
        yield i - 1 + m, slice(0, n - i), slice(m - n + i, m)

def correlate2d(a, b):
    r1, c1 = a.shape
    r2, c2 = b.shape
    res = np.zeros((r1+r2, c1+c2), dtype=np.uint32)
    for i, i1, i2 in correlate_slices(r1, r2):
        for j, j1, j2 in correlate_slices(c1, c2):
            eq = a[i1, j1] == b[i2, j2]
            #if i == r1 - 1 and j == c1 - 1:
            #    print('\n'.join(''.join(map(str,map(int,row))) for row in eq))
            res[i, j] = np.count_nonzero(eq)
    return res

def recorrelate(prev, cur):
    a = prev.reshape(-1).view(np.uint16).reshape(*prev.shape[0:2])
    b = cur.reshape(-1).view(np.uint16).reshape(*cur.shape[0:2])
    c = correlate2d(a, b)
    #print(c)
    best = np.unravel_index(c.argmax(), c.shape)
    #print(best)
    print(best[0] - a.shape[0] + 1, best[1] - a.shape[1] + 1)
    print(c[best])
    return np.array(cur)

def main():
    wininfo = get_wininfo()

    #parse_dwarf_attribute(fetch_frame(wininfo))

    ffmpeg, compressor, frames = fetch_frames(wininfo)
    levels = {}
    for frame in frames():
        data = interpret_frame(frame)
        window, z = data['window'], data['z']
        if window is not None:
            if z in levels:
                levels[z] = recorrelate(levels[z], window)
            else:
                levels[z] = np.array(window)
            #for r in data['window'][:,:,0]:
            #    print(dfdecode(r))

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


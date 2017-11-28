
# HMMM


FILEPATH = "pitchtable.h"
ROWS = 16 # NOTE only goes to 50%!
COLS = 32

def write_C_array(out, _type, identifier, items):
    out.write('{} {}[] = {{\n    '.format(_type, identifier))

    for i in range(len(items)):
        bitmask = items[i]

        lastrow = i == len(items) - 1
        out.write(' {}'.format(hex(bitmask)))
        if lastrow:
            out.write('\n')
        else:
            out.write(',\n    ')
            
    out.write('};\n\n')

def getBit(idx, max_rows, l, k):
    if idx == 1:
        return int((k*l*2) + l)
    if k == 0:
        l = l / 2
        k = (max_rows / l) / 2
    k -= 1
    return getBit(idx - 1, max_rows, l, k)
        
if __name__ == "__main__":
    data = []
    for r in range(int(ROWS)):
        bitmask = 0
        debug = [0] * COLS
        for j in range(r):
            i = getBit(j+1, COLS, COLS, 0)
            bitmask |= 1 << i - 1
            debug[i-1] = 1
        data.append(bitmask)
        print(debug)
        
    out = open(FILEPATH, 'w')
    out.write('\n')
    out.write("#define PITCH_TABLE_ROWS {}\n".format(ROWS))
    out.write("#define PITCH_TABLE_COLS {}\n\n".format(COLS))
    write_C_array(out, "unsigned long", "pitchTable", data)
    
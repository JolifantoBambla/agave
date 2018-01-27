import struct

# command id will be int32 to future-proof it.
# note that the server needs to know these signatures too.
COMMANDS = {
    # tell server to identify this session?
    "SESSION": [0, "S"],
    # tell server where files might be (appends to existing)
    "ASSET_PATH": [1, "S"],
    # load a volume
    "LOAD_OME_TIF": [2, "S"],
    # set camera pos
    "EYE": [3, "F32", "F32", "F32"],
    # set camera target pt
    "TARGET": [4, "F32", "F32", "F32"],
    # set camera up direction
    "UP": [5, "F32", "F32", "F32"],
    "APERTURE": [6, "F32"],
    "FOV_Y": [7, "F32"],
    "FOCALDIST": [8, "F32"],
    "EXPOSURE": [9, "F32"],
    "MAT_DIFFUSE": [10, "I32", "F32", "F32", "F32", "F32"],
    "MAT_SPECULAR": [11, "I32", "F32", "F32", "F32", "F32"],
    "MAT_EMISSIVE": [12, "I32", "F32", "F32", "F32", "F32"],
    # set num render iterations
    "RENDER_ITERATIONS": [13, "I32"],
    # (continuous or on-demand frames)
    "STREAM_MODE": [14, "I32"],
    # request new image
    "REDRAW": [15],
    "SET_RESOLUTION": [16, "I32", "I32"],
    "DENSITY": [17, "F32"],
    # move camera to bound and look at the scene contents
    "FRAME_SCENE": [18],
    "MAT_GLOSSINESS": [19, "I32", "F32"],
    # channel index, 1/0 for enable/disable
    "ENABLE_CHANNEL": [20, "I32", "I32"],
    # channel index, window, level.  (Do I ever set these independently?)
    "SET_WINDOW_LEVEL": [21, "I32", "F32", "F32"]
}

# strategy: add elements to prebuffer, and then traverse prebuffer to convert to binary before sending?
class CommandBuffer:
    def __init__(self):
        # [command, args],...
        self.prebuffer = []
        self.buffer = None

    def compute_size(self):
        # iterate length of prebuffer to compute size.
        bytesize = 0
        for command in self.prebuffer:
            # for each command.

            # one i32 for the command id.
            bytesize += 4

            commandCode = command[0]
            signature = COMMANDS[commandCode]
            nArgsExpected = len(signature)-1
            # for each arg:
            if (len(command)-1 != nArgsExpected):
                print("BAD COMMAND: EXPECTED " + str(nArgsExpected) + " args and got " + str(len(command)-1))
                return 0

            for j in range(0, nArgsExpected):
                # get arg type
                argtype = signature[j+1]
                if argtype == "S":
                    # one int32 for string length
                    bytesize += 4
                    # followed by one byte per char.
                    bytesize += len(command[j+1])
                elif argtype == "F32":
                    bytesize+=4
                elif argtype == "I32":
                    bytesize+=4
        return bytesize
        
    def make_buffer(self):
        bytesize = self.compute_size()

        # allocate arraybuffer and then fill it.
        self.buffer = bytearray(bytesize)

        offset = 0
        LITTLE_ENDIAN = True
        for cmd in self.prebuffer:
            commandCode = cmd[0]
            signature = COMMANDS[commandCode]
            nArgsExpected = len(signature)-1

            # the numeric code for the command
            struct.pack_into('>i', self.buffer, offset, signature[0])
            offset+=4
            for j in range(0, nArgsExpected):
                # get arg type
                argtype = signature[j+1]
                if argtype == "S":
                    sstr = cmd[j+1]
                    struct.pack_into('>i', self.buffer, offset, len(sstr))
                    offset+=4
                    for k in sstr:
                        struct.pack_into('B', self.buffer, offset, ord(k))
                        offset+=1
                elif argtype == "F32":
                    struct.pack_into('f', self.buffer, offset, cmd[j+1])
                    offset+=4
                elif argtype == "I32":
                    struct.pack_into('>i', self.buffer, offset, cmd[j+1])
                    offset+=4
        # result is in this.buffer
        return self.buffer

    # commands are added by command code string name followed by appropriate signature args.
    def add_command(self, *args):
        # TODO: check against signature!!!
        self.prebuffer.append(args)

if __name__ == '__main__':
    cb = CommandBuffer()
    cb.add_command("EYE", 1.0, 1.0, 5.0);
    cb.add_command("TARGET", 3.0, 3.0, 0.0);
    cb.add_command("SESSION", "hello");
    cb.add_command("APERTURE", 7.0);
    buf = cb.make_buffer()
    print(list(buf))

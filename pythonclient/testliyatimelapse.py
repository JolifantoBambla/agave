import ws4py
from ws4py.client.threadedclient import WebSocketClient
import io
import json
import math
import numpy
from PIL import Image
from commandbuffer import CommandBuffer
from collections import deque
import matplotlib
matplotlib.use('TkAgg')
from matplotlib import pyplot as plt
# import vtk

import numpy
from aicsimage.io.tifReader import TifReader
from aicsimage.io.omeTifWriter import OmeTifWriter

INPUTS = [
    ("//allen/aics/assay-dev/MicroscopyOtherData/Liya/2017/20171107/20171107_C03_001_S2/multich/20171107_C03_001_S2_T", 47),
    ("//allen/aics/assay-dev/MicroscopyOtherData/Liya/2017/20171103/20171103_C04_001_S3/multich/20171103_C04_001_S3_T", 49),
    ("//allen/aics/assay-dev/MicroscopyOtherData/Liya/2017/20171031/20171031_C01_001_S1/multich/20171031_C01_001_S1_T", 49),
    ("//allen/aics/assay-dev/MicroscopyOtherData/Liya/2017/20171030/20171030_C01_001_S1/multich/20171030_C01_001_S1_T", 46)
]
WHICH_INPUT = 1
IN_SUFFIX = ".tif"
OUTROOT = '//allen/aics/animated-cell/Dan/LiyaTimelapse/'

def infilename(dataset, frame):
    return INPUTS[dataset][0] + str(frame).zfill(2) + IN_SUFFIX

def lerp(startframe, endframe, startval, endval):
    x = numpy.linspace(startframe, endframe, num=endframe-startframe+1, endpoint=True)
    y = startval + (endval-startval)*(x-startframe)/(endframe-startframe)
    print(y)

def rotation_matrix(axis, theta):
    """
    Return the rotation matrix associated with counterclockwise rotation about
    the given axis by theta radians.
    """
    axis = numpy.asarray(axis)
    axis = axis/math.sqrt(numpy.dot(axis, axis))
    a = math.cos(theta/2.0)
    b, c, d = -axis*math.sin(theta/2.0)
    aa, bb, cc, dd = a*a, b*b, c*c, d*d
    bc, ad, ac, ab, bd, cd = b*c, a*d, a*c, a*b, b*d, c*d
    return numpy.array([[aa+bb-cc-dd, 2*(bc+ad), 2*(bd-ac)],
                     [2*(bc-ad), aa+cc-bb-dd, 2*(cd+ab)],
                     [2*(bd+ac), 2*(cd-ab), aa+dd-bb-cc]])

def rotate_vec(v, axis, angle):
    return numpy.dot(rotation_matrix(axis,angle), v)


# assumptions: every commandbuffer send should result in one image.
# also, they arrive in the order the buffers were sent.
class DummyClient(WebSocketClient):
    def push_request(self, cb, id):
        buf = cb.make_buffer()
        self.send(buf, True)
        self.queue.append(id)

    def request_frame_info(self):
        # load image and get info back. discard any binary coming back from this request.
        if self.requested_frame + 1 == INPUTS[WHICH_INPUT][1]:
            return False

        i = self.requested_frame

        print(infilename(WHICH_INPUT, i))
        cb = CommandBuffer()
        cb.add_command("LOAD_OME_TIF", infilename(WHICH_INPUT, i))
        # this should return one image and one text response.
        self.waiting_for_info = True
        self.push_request(cb, (i, "info"))
        return True

    def request_frame(self):
        i = self.requested_frame

        cb = CommandBuffer()

        # test clipping
        # cb.add_command("SET_CLIP_REGION", 0, 1, 0, 1, 0, 0.64)

        # cb.add_command("EYE", 0.429107, 0.371954, 1.39376)
        # cb.add_command("TARGET", 0.5, 0.5, 0.0410354)
        # cb.add_command("UP", -0.00492508, 0.995561, 0.0939797)
        cb.add_command("SET_VOXEL_SCALE", 0.0826135, 0.0826135, 0.2079217)
        # cb.add_command("SET_RESOLUTION", 1024, 1024)
        # cb.add_command("CAMERA_PROJECTION", 0, 55)
        # cb.add_command("FRAME_SCENE")
        # cb.add_command("ORBIT_CAMERA", 10, 10)

        # cb.add_command("RENDER_ITERATIONS", 384)
        # cb.add_command("SET_CLIP_REGION", 0, 1, 0, 1, 0, 1)
        # cb.add_command("EXPOSURE", 0.7964)
        # cb.add_command("DENSITY", 100)
        # cb.add_command("APERTURE", 0)
        # cb.add_command("FOCALDIST", 0.75)

        # cb.add_command("ENABLE_CHANNEL", 0, 1)
        # cb.add_command("MAT_DIFFUSE", 0, 1, 0, 1, 1.0)
        # cb.add_command("MAT_SPECULAR", 0, 0, 0, 0, 0.0)
        # cb.add_command("MAT_EMISSIVE", 0, 0, 0, 0, 0.0)
        # cb.add_command("MAT_GLOSSINESS", 0, 0)
        # cb.add_command("SET_WINDOW_LEVEL", 0, 1, 0.6166)
        
        # cb.add_command("ENABLE_CHANNEL", 1, 1)
        # cb.add_command("MAT_DIFFUSE", 1, 1, 1, 1, 1.0)
        # cb.add_command("MAT_SPECULAR", 1, 0, 0, 0, 0.0)
        # cb.add_command("MAT_EMISSIVE", 1, 0, 0, 0, 0.0)
        # cb.add_command("MAT_GLOSSINESS", 1, 0)
        # cb.add_command("SET_WINDOW_LEVEL", 1, 1, 0.6067)
        
        # cb.add_command("ENABLE_CHANNEL", 2, 1)
        # cb.add_command("MAT_DIFFUSE", 2, 0, 1, 1, 1.0)
        # cb.add_command("MAT_SPECULAR", 2, 0, 0, 0, 0.0)
        # cb.add_command("MAT_EMISSIVE", 2, 0, 0, 0, 0.0)
        # cb.add_command("MAT_GLOSSINESS", 2, 0)
        # cb.add_command("SET_WINDOW_LEVEL", 2, 1, 0.6477)

        # cb.add_command("SKYLIGHT_TOP_COLOR", 0.5, 0.5, 0.5)
        # cb.add_command("SKYLIGHT_MIDDLE_COLOR", 0.5, 0.5, 0.5)
        # cb.add_command("SKYLIGHT_BOTTOM_COLOR", 0.5, 0.5, 0.5)
        # cb.add_command("LIGHT_POS", 0, 14.4781, 0, 0.7751)
        # cb.add_command("LIGHT_COLOR", 0, 414.043, 414.043, 424.394)
        # cb.add_command("LIGHT_SIZE", 0, 0.3931, 0.3931)

        cb.add_command("SET_RESOLUTION", 1024, 1024)
        cb.add_command("RENDER_ITERATIONS", 256)
        cb.add_command("SET_CLIP_REGION", 0, 1, 0, 1, 0, 1)
        # cb.add_command("EYE", 0.243166, 0.301235, 1.36233)
        # cb.add_command("TARGET", 0.5, 0.5, 0.0410354)
        # cb.add_command("UP", -0.0278741, 0.989272, 0.143399)
        cb.add_command("CAMERA_PROJECTION", 0, 55)
        cb.add_command("EXPOSURE", 0.75)
        cb.add_command("DENSITY", 4.0)
        cb.add_command("APERTURE", 0)
        cb.add_command("FOCALDIST", 0.75)
        cb.add_command("ENABLE_CHANNEL", 0, 1)
        cb.add_command("MAT_DIFFUSE", 0, 1, 0, 1, 1.0)
        cb.add_command("MAT_SPECULAR", 0, 0, 0, 0, 0.0)
        cb.add_command("MAT_EMISSIVE", 0, 0, 0, 0, 0.0)
        cb.add_command("MAT_GLOSSINESS", 0, 0)
        cb.add_command("AUTO_THRESHOLD", 0, 0)
        cb.add_command("ENABLE_CHANNEL", 1, 1)
        cb.add_command("MAT_DIFFUSE", 1, 1, 1, 1, 1.0)
        cb.add_command("MAT_SPECULAR", 1, 0.529412, 0.529412, 0.529412, 0.0)
        cb.add_command("MAT_EMISSIVE", 1, 0.109804, 0.109804, 0.109804, 0.0)
        cb.add_command("MAT_GLOSSINESS", 1, 0)
        cb.add_command("AUTO_THRESHOLD", 1, 0)
        cb.add_command("ENABLE_CHANNEL", 2, 1)
        cb.add_command("MAT_DIFFUSE", 2, 0, 1, 1, 1.0)
        cb.add_command("MAT_SPECULAR", 2, 0, 0, 0, 0.0)
        cb.add_command("MAT_EMISSIVE", 2, 0, 0, 0, 0.0)
        cb.add_command("MAT_GLOSSINESS", 2, 0)
        cb.add_command("AUTO_THRESHOLD", 2, 0)
        cb.add_command("SKYLIGHT_TOP_COLOR", 0.5, 0.5, 0.5)
        cb.add_command("SKYLIGHT_MIDDLE_COLOR", 0.5, 0.5, 0.5)
        cb.add_command("SKYLIGHT_BOTTOM_COLOR", 0.5, 0.5, 0.5)
        cb.add_command("LIGHT_POS", 0, 10, 0, 0)
        cb.add_command("LIGHT_COLOR", 0, 100, 100, 100)
        cb.add_command("LIGHT_SIZE", 0, 1, 1)

        self.push_request(cb, (i, OUTROOT + 'ZSTACK_' + str(i).zfill(4) + ".png"))
        return True

    def opened(self):
        self.requested_frame = 0
        self.queue = deque([])
        print("opened up")
        self.request_frame_info()
        # self.loop_frames(offset=0)

    def closed(self, code, reason=None):
        print("Closed down", code, reason)

    def received_message(self, m):
        req = self.queue.popleft()
        if m.is_binary:
            # print(req)
            # number = req[0]
            name = req[1]

            # don't save image from an info req
            if name == "info":
                print("got info binary: " + str(self.requested_frame))
                # put req back on queue if we are waiting for its other half
                if self.waiting_for_info:
                    self.queue.appendleft(req)
                    self.waiting_for_info = False
                else:
                    # do something special using the imgdata
                    print("Got info - requesting render " + str(self.requested_frame))
                    self.request_frame()
                return
            
            im = Image.open(io.BytesIO(m.data))
            im.save(name)
            print("Saved frame " + str(self.requested_frame) + " : "  + name)
            print("Request next frame")
            self.requested_frame = self.requested_frame + 1
            if not self.request_frame_info():
                self.close(reason='Done!')

            # imgplot.set_data(im)
        else:
            # print(m)
            if len(m) == 175:
                self.close(reason='Bye bye')
            else:
                # should be json data coming from an "info" request
                if req[1] == "info":
                    print("got info text: " + str(self.requested_frame))
                    self.imgdata = json.loads(m.data)
                    # put req back on queue if we are waiting for its other half
                    if self.waiting_for_info:
                        self.queue.appendleft(req)
                        self.waiting_for_info = False
                    else:
                        # do something special using the imgdata
                        print("Got info - requesting render " + str(self.requested_frame))
                        self.request_frame()


# imgplot = plt.imshow(numpy.zeros((1024, 768)))
if __name__ == '__main__':
    try:
        # convert_combined()
        # convertFiles()
        # ws = DummyClient('ws://dev-aics-dtp-001:1235/', protocols=['http-only', 'chat'])
        ws = DummyClient('ws://localhost:1235/', protocols=['http-only', 'chat'])
        ws.connect()
        ws.run_forever()
    except KeyboardInterrupt:
        print("keyboard")
        ws.close()


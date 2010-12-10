
from arboris.core import World, simulate, Observer


import socket
import subprocess
import time

class SocketComObs(Observer):
    def __init__(self, arborisViewer, daefile, host="127.0.0.1", port=5555):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind((host, port))
        self.app_call = app = [arborisViewer, daefile, "-socket", host, str(port)]

    def init(self,world, timline):
        self.world = world
        subprocess.Popen(self.app_call)
        self.s.listen(1)
        self.conn, self.addr = self.s.accept()
        print 'Connected by', self.addr
        time.sleep(2.)


    def update(self,dt):
        msg = ""
        for b in self.world.getbodies():
            H = b.pose
            msg += b.name + " " + " ".join([str(round(v,5)) for v in H[0:3,:].reshape(12)]) + "\n"

        try:
            self.conn.send(msg)
            pass
        except socket.error:
            print "connection lost"

    def finish(self):
        try:
            self.conn.send("close connection")
            self.s.close()
        except socket.error:
            pass


from xml.etree.ElementTree import ElementTree, SubElement
from arboris.massmatrix import principalframe, transport
from arboris.visu_collada import find_by_id, QN, SHAPES, fix_namespace

def add_inertia_in_collada(world, dae_filename):
    tree = ElementTree(file=dae_filename)
    
    bodies = world.getbodies()[1:]
    inertia = {}
    for b in bodies:
        H = principalframe(b.mass)
        M = transport(b.mass, H)
    
        e = find_by_id(tree, b.name, QN("node"))
        if e is not None:
            node = SubElement(e, QN("node"), {"id":"inertia"})
            matrix = SubElement(node, QN("matrix"), {'sid':'matrix'})
            matrix.text = str(H.reshape(-1)).strip('[]')
            scale = SubElement(node, QN('scale'))
            scale.text = "{0} {1} {2}".format(M[0,0], M[1,1], M[2,2])
            elem = SubElement(node, QN("instance_geometry"), {"url": SHAPES+"#sphere_80"})
        
    fix_namespace(tree)
    tree.write(dae_filename, "utf-8")



## WORLD
from arboris.robots import simplearm, human36
w = World()
simplearm.add_simplearm(w)
#human36.add_human36(w)

## INIT
joints = w.getjoints()
joints[0].gpos[:] = .5
joints[1].gpos[:] = .5
joints[2].gpos[:] = .5

w.update_dynamic()

## CTRL
from arboris.controllers import WeightController
w.register(WeightController())


### OBS
from arboris.observers import PerfMonitor
from arboris.visu_collada import write_collada_scene
obs = []
obs.append(PerfMonitor(True))
write_collada_scene(w, "my_scene.dae", flat=True)
add_inertia_in_collada(w, "my_scene.dae")

obs.append(SocketComObs("daenim", "my_scene.dae"))


## SIMULATE
from numpy import arange
simulate(w, arange(0,1,0.01), obs)

print "end of the simulation"


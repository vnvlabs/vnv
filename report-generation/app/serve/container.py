import threading

import docker
from docker.errors import NotFound
from flask import g

docker_client = docker.from_env()

def docker_container_ready():

    try:
        c = docker_client.containers.get("vnv-" + g.user)
        return c.ports['5001/tcp'][0]["HostPort"]

    except:
        pass

    print("Looking for docker container for ", g.user)

    return None


def launch_docker_container(uname, password):
    print("Obviously not thread safe ---- ")
    try:
        container = docker_client.containers.get("vnv-" + uname)
        container.start()


    except NotFound as e:
        #Container not found for user -- so create one.
        try:
            comm = "./launch.sh " + password

            try:
                volume = docker_client.volumes.get("vnv-" + uname)
            except Exception as e:
                volume = docker_client.volumes.create("vnv-"+uname)

            volumes = {'vnv-'+uname : {'bind': '/home/'+uname, 'mode': 'rw'} }
            container = docker_client.containers.run("vnv_serve:latest", volumes=volumes, command=comm, ports={5001:None}, name="vnv-" + uname, detach=True)
            print(container)
        except Exception as e:
            print(e)
            return None

def stop_(uname):
    c = docker_client.containers.get("vnv-" + uname)
    c.stop()

def stop_docker_container():
    try:
        c = g.user
        threading.Thread(target=stop_, args=[c]).start()
    except:
        pass
    print("Killing docker container for " ,g.user)

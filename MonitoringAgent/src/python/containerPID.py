#containerPID.py
import sys
import docker

# 获取指定容器的进程号
def getContainerPID(containerID):
    client = docker.APIClient(base_url='unix://var/run/docker.sock')
    containerData = client.inspect_container(containerID)
    pid = containerData["State"]["Pid"]
    return pid
import sys
import time
import docker
import ast
import threading

client = docker.APIClient(base_url='unix://var/run/docker.sock')
startList = []
stopList = []

# 收集在指定时间内启动的容器
def start(sinceTime, untilTime):
	events = client.events(since=sinceTime, until=untilTime, filters={'event': 'start'})
	for event in events:
		eventDict = ast.literal_eval(event.decode())
		temp = {'status':eventDict['status'], 'id':eventDict['id']}
		startList.append(temp)
	events.close()

# 收集在指定时间内关闭的容器
def stop(sinceTime, untilTime):
	events = client.events(since=sinceTime, until=untilTime, filters={'event': 'stop'})
	for event in events:
		eventDict = ast.literal_eval(event.decode())
		temp = {'status':eventDict['status'], 'id':eventDict['id']}
		hasStopped = False
		for index in startList:
			if temp['id'] == index['id']:
				startList.pop(startList.index(index))
				hasStopped = True
				break
		if not hasStopped:
			stopList.append(temp)
	events.close()

# 将收集的容器信息转换成jsoncpp能够解析的字符串格式
def toString(containerItem):
	containerId = containerItem['id']
	status = containerItem['status']
	itemStr = '{\"id\":\"'+str(containerId)+'\",\"status\":\"'+str(status)+'\"}'
	return itemStr


def getContainerInfo(sinceTime, untilTime, FOUND_CYCLE):
	startList.clear()
	stopList.clear()
	# 启动线程
	t1 = threading.Thread(target=start, args=(sinceTime, untilTime))
	t2 = threading.Thread(target=stop, args=(sinceTime, untilTime))
	t1.start()
	t2.start()
	t1.join()
	t2.join()

	listStr = ""
	if len(startList) > 0 or len(stopList) > 0:
		if len(startList) > 0:
			for i in range(len(startList)-1):
				listStr += toString(startList[i])+";"
			listStr += toString(startList[len(startList)-1])
		if len(stopList) > 0:
			for i in range(len(stopList)-1):
				listStr += toString(stopList[i])+";"
			listStr += toString(stopList[len(stopList)-1])
	else:
		listStr += '{\"id\":\"'+"null"+'\",\"status\":\"'+"null"+'\"}'
	return listStr


# 获取指定容器的进程号
def getContainerPID(containerID):
    containerData = client.inspect_container(containerID)
    pid = containerData["State"]["Pid"]
    return pid
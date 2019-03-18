import sys
import time
import docker
import ast
import _thread

containerList = []
client = docker.APIClient(base_url='unix://var/run/docker.sock')

def start(sinceTime, untilTime):
	events = client.events(since=sinceTime, until=untilTime, filters={'event': 'start'})
	for event in events:
		eventDict = ast.literal_eval(event.decode())
		temp = {'status':eventDict['status'], 'id':eventDict['id']}
		containerList.append(temp)
	events.close()

def stop(sinceTime, untilTime):
	events = client.events(since=sinceTime, until=untilTime, filters={'event': 'stop'})
	for event in events:
		eventDict = ast.literal_eval(event.decode())
		temp = {'status':eventDict['status'], 'id':eventDict['id']}
		hasStopped = False
		for index in containerList:
			if temp['id'] == index['id']:
				containerList.pop(containerList.index(index))
				hasStopped = True
				break
		if not hasStopped:
			containerList.append(temp)
	events.close()


def toString(containerItem):
	containerId = containerItem['id']
	status = containerItem['status']
	itemStr = '{\"id\":\"'+str(containerId)+'\",\"status\":\"'+str(status)+'\"}'
	return itemStr


def getContinerInfo(sinceTime, untilTime, FOUND_CYCLE):
	try:
		_thread.start_new_thread(start, (sinceTime, untilTime))
		_thread.start_new_thread(stop, (sinceTime, untilTime))
	except:
		print("can't create thread!")
	time.sleep(FOUND_CYCLE)
	listStr = ""
	for i in range(len(containerList)-1):
		listStr += toString(containerList[i] + ";")
	listStr += toString(containerList[len(containerList)-1])
	return listStr

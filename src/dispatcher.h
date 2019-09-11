/*
	dispatcher.h - header file for the Dispatcher class.
	
	Revision 0
	
	Notes:
			- 
			
	2016/11/19, Maya Posch
	(c) Nyanko.ws.
*/


#pragma once
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "abstract_request.h"
#include "worker.h"

#include <queue>
#include <mutex>
#include <thread>
#include <vector>


class Dispatcher {
	static std::queue<AbstractRequest*> requests;
	static std::queue<Worker*> workers;
	static std::mutex requestsMutex;
	static std::mutex workersMutex;
	static std::vector<Worker*> allWorkers;
	static std::vector<std::thread*> threads;
	
public:
	static bool init(int workers);
	static bool stop();
	static void addRequest(AbstractRequest* request);
	static bool addWorker(Worker* worker);
};

#endif

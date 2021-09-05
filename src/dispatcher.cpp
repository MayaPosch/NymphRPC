/*
	dispatcher.cpp - Implementation of the Dispatcher class.
	
	Revision 0
	
	Features:
			- 
			
	Notes:
			- 
			
	2016/11/19, Maya Posch
	(c) Nyanko.ws
*/


#include "dispatcher.h"

#include <iostream>
using namespace std;


// Static initialisations.
int Dispatcher::poolSize = 0;
queue<AbstractRequest*> Dispatcher::requests;
queue<Worker*> Dispatcher::workers;
mutex Dispatcher::requestsMutex;
mutex Dispatcher::workersMutex;
vector<Worker*> Dispatcher::allWorkers;
vector<thread*> Dispatcher::threads;


// --- INIT ---
// Set the maximum pool size.
bool Dispatcher::init(int workers) {
	poolSize = workers;
	
	std::cout << "Dispatcher: Setting max pool size to " << poolSize << " workers." << std::endl;
	
	return true;
}


// --- STOP ---
// Terminate the worker threads and clean up.
bool Dispatcher::stop() {
	for (int i = 0; i < allWorkers.size(); ++i) {
		allWorkers[i]->stop();
		delete allWorkers[i];
	}
	
	cout << "Stopped workers.\n";
	
	for (int j = 0; j < threads.size(); ++j) {
		threads[j]->join();
		delete threads[j];
		
		cout << "Joined threads.\n";
	}
	
	return true;
}


// --- ADD REQUEST ---
void Dispatcher::addRequest(AbstractRequest* request) {
	// Check whether there's a worker available in the workers queue, else add
	// the request to the requests queue.
	workersMutex.lock();
	if (!workers.empty()) {
		Worker* worker = workers.front();
		worker->setRequest(request);
		condition_variable* cv;
		mutex* mtx;
		worker->getCondition(cv);
		worker->getMutex(mtx);
		unique_lock<mutex> lock(*mtx);
		cv->notify_one();
		workers.pop();
		workersMutex.unlock();
	}
	else if (threads.size() < poolSize) {
		// Create new worker thread.
		std::cout << "Dispatcher: Creating new thread..." << std::endl;
		thread* t = 0;
		Worker* w = 0;
		w = new Worker;
		w->setRequest(request);
		allWorkers.push_back(w);
		t = new thread(&Worker::run, w);
		threads.push_back(t);
		workersMutex.unlock();
	}
	else {
		workersMutex.unlock();
		requestsMutex.lock();
		requests.push(request);
		requestsMutex.unlock();
	}
	
	
}


// --- ADD WORKER ---
bool Dispatcher::addWorker(Worker* worker) {
	// If a request is waiting in the requests queue, assign it to the worker.
	// Else add the worker to the workers queue.
	// Returns true if the worker was added to the queue and has to wait for
	// its condition variable.
	bool wait = true;
	requestsMutex.lock();
	if (!requests.empty()) {
		AbstractRequest* request = requests.front();
		worker->setRequest(request);
		requests.pop();
		wait = false;
		requestsMutex.unlock();
	}
	else {
		requestsMutex.unlock();
		workersMutex.lock();
		workers.push(worker);
		workersMutex.unlock();
	}
	
	return wait;
}

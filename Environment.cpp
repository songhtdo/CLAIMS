/*
 * Environment.cpp
 *
 *  Created on: Aug 10, 2013
 *      Author: wangli
 */

#include "Environment.h"
#include <libconfig.h++>
#include <iostream>
#include <sstream>
#include <string>
#include "Debug.h"
#include "Config.h"
#include "common/Logging.h"
Environment* Environment::_instance=0;
Environment::Environment(bool ismaster):ismaster_(ismaster) {
	_instance=this;
	Config::getInstance();
	logging_=new EnvironmentLogging();
	Initialize();
	portManager=PortManager::getInstance();
	catalog_=Catalog::getInstance();
	catalog_->restoreCatalog();

	if(ismaster){
		logging_->log("Initializing the Coordinator...");
		InitializeCoordinator();

	}

	logging_->log("Initializing the AdaptiveEndPoint...");
	InitializeEndPoint();
/**
 * TODO:
 * DO something in AdaptiveEndPoint such that the construction function does
	not return until the connection is completed. If so, the following sleep()
	dose not needed.

	This is done in Aug.18 by Li :)
 */

	/*Before initializing Resource Manager, the instance ip and port should be decided.*/

	InitializeResourceManager();
//		return;

	InitializeStorage();

	InitializeBufferManager();


	logging_->log("Initializing the ExecutorMaster...");
	iteratorExecutorMaster=new IteratorExecutorMaster();

	logging_->log("Initializing the ExecutorSlave...");
	iteratorExecutorSlave=new IteratorExecutorSlave();

	exchangeTracker =new ExchangeTracker();
	expander_tracker_=ExpanderTracker::getInstance();
	if(ismaster){
		InitializeClientListener();
	}
}

Environment::~Environment() {
	_instance=0;
	delete logging_;
	delete portManager;
	delete catalog_;
	delete coordinator;
	if(ismaster_){
		delete iteratorExecutorMaster;
		delete resourceManagerMaster_;
		delete blockManagerMaster_;
		destoryClientListener();
	}
	delete iteratorExecutorSlave;
	delete exchangeTracker;
	delete resourceManagerSlave_;
	delete blockManager_;
	delete bufferManager_;
	delete expander_tracker_;
	delete endpoint;
}
Environment* Environment::getInstance(bool ismaster){
	if(_instance==0){
			new Environment(ismaster);
	}
	return _instance;
}
std::string Environment::getIp(){
	return ip;
}
unsigned Environment::getPort(){
	return port;
}
void Environment::Initialize(){
	libconfig::Config cfg;
	cfg.readFile(Config::config_file.c_str());
	ip=(const char*)cfg.lookup("ip");
}
void Environment::InitializeEndPoint(){
//	libconfig::Config cfg;
//	cfg.readFile("/home/claims/config/wangli/config");
//	std::string endpoint_ip=(const char*)cfg.lookup("ip");
//	std::string endpoint_port=(const char*)cfg.lookup("port");
	std::string endpoint_ip=ip;
	int endpoint_port;
	if((endpoint_port=portManager->applyPort())==-1){
		logging_->elog("The ports in the PortManager is exhausted!");
	}
	port=endpoint_port;
	logging_->log("Initializing the AdaptiveEndPoint as EndPoint://%s:%d.",endpoint_ip.c_str(),endpoint_port);
	std::ostringstream name,port;
	port<<endpoint_port;
	name<<"EndPoint://"<<endpoint_ip<<":"<<port;

	endpoint=new AdaptiveEndPoint(name.str().c_str(),endpoint_ip,port.str());
}
void Environment::InitializeCoordinator(){
	coordinator=new Coordinator();
}
void Environment::InitializeStorage(){

	if(ismaster_){
		blockManagerMaster_=BlockManagerMaster::getInstance();
		blockManagerMaster_->initialize();
	}
		/*both master and slave node run the BlockManager.*/
//		BlockManagerId *bmid=new BlockManagerId();
//		string actorname="blockManagerWorkerActor_"+bmid->blockManagerId;
//		cout<<actorname.c_str()<<endl;
//		BlockManager::BlockManagerWorkerActor *blockManagerWorkerActor=new BlockManager::BlockManagerWorkerActor(endpoint,framework_storage,actorname.c_str());

		blockManager_=BlockManager::getInstance();
		blockManager_->initialize();

}

void Environment::InitializeResourceManager(){
	if(ismaster_){
		resourceManagerMaster_=new ResourceManagerMaster();
	}
	resourceManagerSlave_=new InstanceResourceManager();
	nodeid=resourceManagerSlave_->Register();

}
void Environment::InitializeBufferManager(){
	bufferManager_=BufferManager::getInstance();
}

void Environment::InitializeIndexManager()
{
	indexManager_ = IndexManager::getInstance();
}

AdaptiveEndPoint* Environment::getEndPoint(){
	return endpoint;
}
ExchangeTracker* Environment::getExchangeTracker(){
	return exchangeTracker;
}
ResourceManagerMaster* Environment::getResourceManagerMaster(){
	return resourceManagerMaster_;
}
InstanceResourceManager* Environment::getResourceManagerSlave(){
	return resourceManagerSlave_;
}
NodeID Environment::getNodeID()const{
	return nodeid;
}
Catalog* Environment::getCatalog()const{
	return catalog_;
}

void Environment::InitializeClientListener() {
	listener_=new ClientListener(10000);
	listener_->configure();
}

void Environment::destoryClientListener() {
	listener_->shutdown();
	delete listener_;
}
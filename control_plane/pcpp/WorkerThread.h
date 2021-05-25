#pragma once

#include "DpdkDevice.h"
#include "DpdkDeviceList.h"

struct FlowKey;
struct FlowValue;

class SenderWorkerThread : public pcpp::DpdkWorkerThread
{
 private:
	pcpp::DpdkDevice* m_TxDevice;
	bool m_Stop;
	uint32_t m_CoreId;

public:
 	// c'tor
	SenderWorkerThread(pcpp::DpdkDevice* txDevice);

	// d'tor (does nothing)
	~SenderWorkerThread() { }

	// implement abstract method

	// start running the worker thread
	bool run(uint32_t coreId);

	// ask the worker thread to stop
	void stop();

	// get worker thread core ID
	uint32_t getCoreId() const;
};

class ReceiverWorkerThread : public pcpp::DpdkWorkerThread
{
 private:
	pcpp::DpdkDevice* m_RxDevice;
	bool m_Stop;
	uint32_t m_CoreId;

public:
 	// c'tor
	ReceiverWorkerThread(pcpp::DpdkDevice* rxDevice);

	// d'tor (does nothing)
	~ReceiverWorkerThread() { }

	// implement abstract method

	// start running the worker thread
	bool run(uint32_t coreId);

	// ask the worker thread to stop
	void stop();

	// get worker thread core ID
	uint32_t getCoreId() const;
};

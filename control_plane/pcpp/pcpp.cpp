#include <vector>
#include <unistd.h>
#include <sstream>
#include "SystemUtils.h"
#include "DpdkDeviceList.h"
#include "TablePrinter.h"

#include "WorkerThread.h"


#define MBUF_POOL_SIZE 4*1024-1
#define SENDER_DEVICE_ID 1
#define RECEIVER_DEVICE_ID 0

// #define COLLECT_STATS_EVERY_SEC 2


void stop_pcpp_capture()
{
	pcpp::DpdkDeviceList::getInstance().stopDpdkWorkerThreads();
	printf("Stopped DPDK packet capture\n");
}

void start_pcpp_capture()
{
	// newlines before EAL init prints
	printf("\n\n");
	// Register the on app close event handler
	// onApplicationInterrupted function defined above will be executed when Ctrl+C is pressed while this app is running
	// pcpp::ApplicationEventHandler::getInstance().onApplicationInterrupted(onApplicationInterrupted, NULL);

	// Initialize DPDK
	pcpp::CoreMask coreMaskToUse = pcpp::getCoreMaskForAllMachineCores();
	pcpp::DpdkDeviceList::initDpdk(coreMaskToUse, MBUF_POOL_SIZE);	

	// Find DPDK devices
/* 	pcpp::DpdkDevice* device1 = pcpp::DpdkDeviceList::getInstance().getDeviceByPort(SENDER_DEVICE_ID);
	if (device1 == NULL)
	{
		printf("Cannot find device1 with port '%d'\n", SENDER_DEVICE_ID);
        return 1;
	}
 */
	pcpp::DpdkDevice* device2 = pcpp::DpdkDeviceList::getInstance().getDeviceByPort(RECEIVER_DEVICE_ID);
	if (device2 == NULL)
	{
		printf("Cannot find device2 with port '%d'\n", RECEIVER_DEVICE_ID);
        exit(1);
	}

	// Open DPDK devices
/* 	if (!device1->openMultiQueues(1, 1))
	{
		printf("Couldn't open device1 #%d, PMD '%s'\n", device1->getDeviceId(), device1->getPMDName().c_str());
        return 1;
	} */

	if (!device2->openMultiQueues(1, 1))
	{
		printf("Couldn't open device2 #%d, PMD '%s'\n", device2->getDeviceId(), device2->getPMDName().c_str());
		exit(1);
	}

	// Create worker threads
	std::vector<pcpp::DpdkWorkerThread*> workers;
	workers.push_back(new ReceiverWorkerThread(device2));
	// workers.push_back(new SenderWorkerThread(device1));
	
	// Create core mask 
	int workersCoreMask = 0;
	for (int i = 1; i <=1; i++)
	{
		workersCoreMask = workersCoreMask | (1 << (i));
	}

	// Start capture in async mode
	if (!pcpp::DpdkDeviceList::getInstance().startDpdkWorkerThreads(workersCoreMask, workers))
	{
		printf("Couldn't start worker threads");
		exit(1);
	}

	printf("\nDPDK worker threads started successfully!\n\n");
	
}

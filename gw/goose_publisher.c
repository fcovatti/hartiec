#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#include "mms_value.h"
#include "goose_publisher.h"
#include "hal.h"
#include <pthread.h>
#include "hartip.h"


#define NODE_ADDR       0xe0ff000101

static int running = 1;
char *gwname = "192.168.1.102";
static hip_u16 gwport = 20004; //emerson

static pthread_t Thread_Goose_Publisher;
static struct hip_node_data AcquiredData;
static void sigint_handler(int signalId)
{
	running = 0;
}
static void print_result(int rv)
{
    if (rv == HIP_OK) {
        printf("ok\n");
    } else {
        printf("%d\n", rv);
    }
}
//Publicador de mensagens GOOSE
static void * Goose_Publisher(void * unused)
{
	LinkedList dataSetValues = LinkedList_create();
	//TODO:Check configuration of normal start values
	MmsValue * value1 = MmsValue_newIntegerFromInt32(AcquiredData.position);
	MmsValue * value2 = MmsValue_newIntegerFromInt32(AcquiredData.torque);
	MmsValue * time_value = MmsValue_newBinaryTime(false);

	//Creating list with values
	LinkedList_add(dataSetValues, value1 );
	LinkedList_add(dataSetValues, time_value);
	LinkedList_add(dataSetValues, value2);
	
	printf("Creating Goose publisher\n");
	//Creating GOOSE Publisher
	GoosePublisher publisher = GoosePublisher_create(NULL, "eth1");
	GoosePublisher_setGoCbRef(publisher, "Test1/LLN0.gocb1");
	GoosePublisher_setConfRev(publisher, 1);
	GoosePublisher_setDataSetRef(publisher, "Test1/LLN0$dataset1");

	//Loop
	while(running) {
		//Update Values
		MmsValue_setInt32(value1, AcquiredData.position);
		MmsValue_setInt32(value2, AcquiredData.torque);

		//Publish status every second
		if (GoosePublisher_publish(publisher, dataSetValues) == -1) {
			printf("Error sending message!\n");
		}
		sleep(1);
	}

	//When stoped, destroy publisher
	GoosePublisher_destroy(publisher);

	return NULL;
}	


int main(int argc, char** argv)
{
	signal(SIGINT, sigint_handler);

	int rv, ent=0;
	struct hip_sess *sess;

	printf("Conectando com %s...\n", gwname);
	rv = hip_connect(&sess, gwname, gwport);
    if (rv != HIP_OK) {
        printf("Falha na conexão.\n");
        return 1;
    }
	int unsigned tmps=140;
	while (ent<5){
		printf("> ");
		usleep(tmps*1000);		
		ent++;
	}
	printf("conectado!!\n");
	
	pthread_create(&Thread_Goose_Publisher, NULL, Goose_Publisher, NULL);
	while(running) {
		int rv;
		struct hip_node_data data;

		printf("Reading node...   \n");
		fflush(stdout);
		rv = hip_read_node(sess, NODE_ADDR, &data);
		print_result(rv);
		if (rv != HIP_OK) {
			return -1;
		}

		printf("status: %#x\n", data.status);
		printf("alarms: %#x\n", data.alarms);
		printf("position: %d %%\n", data.position);
		printf("torque: %u Nm\n", data.torque);
		AcquiredData.status = data.status;
		AcquiredData.alarms = data.alarms;
		AcquiredData.position = data.position;
		AcquiredData.torque = data.torque;
		sleep(1);//each second asks again

	}
	return 0;
}

// CDDL HEADER START
//*************************************************************************************************
//                                     Copyright © 2006-2009
//                                           Itron, Inc.
//                                      All Rights Reserved.
//
//
// The contents of this file, and the files included with this file, are subject to the current 
// version of Common Development and Distribution License, Version 1.0 only (the “License”), which 
// can be obtained at http://www.sun.com/cddl/cddl.html. You may not use this file except in 
// compliance with the License. 
//
// The original code, and all software distributed under the License, are distributed and made 
// available on an “AS IS” basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED. ITRON, 
// INC. HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR PARTICULAR PURPOSE, OR NON INFRINGEMENT. Please see the License 
// for the specific language governing rights and limitations under the License.
//
// When distributing Covered Software, include this CDDL Header in each file and include the 
// License file at http://www.sun.com/cddl/cddl.html.  If applicable, add the following below this 
// CDDL HEADER, with the fields enclosed by brackets “[   ]” replaced with your own identifying 
// information: 
//		Portions Copyright [yyyy]  [name of copyright owner]
//
//*************************************************************************************************
// CDDL HEADER END

#define C1222_WANT_EVENTID_TEXT

#include "c1222.h"
#include "c1222stack.h"
#include "c1222misc.h"
#include "c1222_client.h"
#include "c1222al_receivehistory.h"

//#include "windows.h"
#include <winsock2.h>
#include <process.h>




void InitializeLogCriticalSection(void);

extern void RegisterSetLogs(C1222Stack* pStack);
extern void CommModuleSetLogs(C1222Stack* pStack);

extern void RegisterOpenLog(void);
extern void CommModuleOpenLog(void);

extern int SendToClient(Unsigned8* buffer, Unsigned16 length);
extern Boolean HandleMessageFromClient(C1222Stack* pStack);
extern int StartSendToClient(Unsigned8* buffer, Unsigned16 length);
extern void DoSleep(Unsigned32 delay);

extern Boolean simulateHost(ACSE_Message* pMsg, C1222Stack* pStack, Boolean * pMessageSent);
extern void initTables(void);


//Modify these accordingly
static Unsigned8 CalledAp_title[] = {0x80, 0x03, 0x44, 0x21, 0x03 };
static Unsigned8 CallingAp_title[] = {0x80, 0x03, 0x44, 0x21, 0x10 };
static Unsigned8 Called_Aptitle_length = 5;
static Unsigned8 Calling_Aptitle_length = 5;
//Unsigned8 Destination_Native_Address[] = {0x12, 0x34, 0x56, 0x78, 0, 0};
static Unsigned8 Destination_Native_Address[] = {0, 0, 0, 0, 0, 0};
static Unsigned16 Destination_Native_Address_length = 4; //Relay address
static Unsigned8 myMRApTitle[] = { 0x80, 0x02, 0x44, 0x21 };
static Unsigned8 myMRAptile_length = 4;

static C1222Stack registerStack;
static C1222Stack commModuleStack;



#ifdef ENABLE_LOCAL_PORT_ROUTER
C1222LocalPortRouter registerLocalPortRouter;
C1222LocalPortRouter commModuleLocalPortRouter;
#endif // ENABLE_LOCAL_PORT_ROUTER

static C1222ReceiveHistory registerReceiveHistory;
static C1222ReceiveHistory commModuleReceiveHistory;


//Have to change this.
static C1222TransportConfig config = 
             {
    {TRUE}, //Boolean passthroughMode;
    {0x01}, //Unsigned8 linkControlByte;
    {0x20}, //Unsigned8 nodeType;
    {0x01, 0x02, 0x03, 0x04}, //Unsigned8 deviceClass[C1222_DEVICE_CLASS_LENGTH];
    {0x06, 18, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12}, //Unsigned8 esn[C1222_ESN_LENGTH]; // I changed this.
    {4}, //Unsigned8 nativeAddressLength;
    {0x12, 0x34, 0x56, 0x80, 0, 0}, //Unsigned8 nativeAddress[C1222_NATIVE_ADDRESS_LENGTH]; // I changed this ..
    {0}, //Unsigned8 broadcastAddressLength;
    {0, 0, 0, 0, 0, 0}, //Unsigned8 broadcastAddress[C1222_NATIVE_ADDRESS_LENGTH];
    {4}, //Unsigned8 relayAddressLength;
    {0, 0, 0, 0, 0, 0}, //Unsigned8 relayNativeAddress[C1222_NATIVE_ADDRESS_LENGTH];
    {0x0d, 0x00, 0x00, 0x00,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Unsigned8 nodeApTitle[C1222_APTITLE_LENGTH];
    {0x0d, 0x00, 0x00, 0x00,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},//Unsigned8 masterRelayApTitle[C1222_APTITLE_LENGTH];
    {FALSE}, //Boolean wantClientResponseControl;
    {0}, //Unsigned8 numberOfRetries;
    {0}, //Unsigned16 responseTimeout;                
             };




// register thread



static Unsigned32 registerCounter = 0;
static Unsigned32 commModuleCounter = 0;

static C1222Client registerClient;
static Unsigned32 messageSentCount = 0;
static Unsigned32 messageNotSentCount = 0;

static Unsigned8 c1222Register_Send_MessageBuffer[MAX_ACSE_MESSAGE_LENGTH];
static Unsigned8 c1222Register_Receive_MessageBuffer[MAX_ACSE_MESSAGE_LENGTH];

void Set_Request_parameters(C1222Client * pClient, Unsigned16 CallingAp_InvocationId)
{
	setmyApTitle(pClient,CallingAp_title,Calling_Aptitle_length);
	setCalled_Aptitle(pClient,CalledAp_title,Called_Aptitle_length);
	//setDestination_Native_Address( pClient, Destination_Native_Address, Destination_Native_Address_length);-
	setDestination_Native_Address( pClient, pClient->pStack->Xapplication.transportConfig.relayNativeAddress, pClient->pStack->Xapplication.transportConfig.relayAddressLength);
	setmyMRApTitle(pClient,myMRApTitle,myMRAptile_length);
	setCallingApInvocationId(pClient, CallingAp_InvocationId);
	printf("Reg : Parameters set successfully\n");

}

void RegisterThread(void* param)
{
    ACSE_Message* pMsg;
    char myBuffer[100];
    Boolean messageSent = FALSE;
	Unsigned8 Expected_type_of_response = 0;
	Unsigned16 CallingAp_InvocationId =1 ;
	int sent_request = 0;
	Unsigned8 Read_request_table_id = 52;
    
	RegisterOpenLog();

    (void)param;
 
    initTables();
	printf("Started register thread\n");
    #ifdef ENABLE_LOCAL_PORT_ROUTER
	C1222Stack_Init(&registerStack, &registerLocalPortRouter, 1, FALSE, 0, FALSE, FALSE);
    #else
    C1222Stack_Init(&registerStack, 0, 1, FALSE, 0, FALSE, FALSE); // the port is set to 1 here. Different from simple_serial_device.c. Interface number is set to 0 *
	#endif // ENABLE_LOCAL_PORT_ROUTER
	
	C1222Stack_EnableLog(&registerStack, TRUE);
	
	RegisterSetLogs(&registerStack);

    C1222Stack_SetConfiguration(&registerStack, &config);
    
    C1222Stack_SetRegistrationMaxRetryFactor(&registerStack, 0xFF);

    C1222Stack_SetRegistrationRetryLimitsFormat1(&registerStack, 0, 1);
    
    C1222Client_Init(&registerClient, &registerStack, c1222Register_Send_MessageBuffer, c1222Register_Receive_MessageBuffer);
    
    C1222ALReceiveHistory_Init(&registerReceiveHistory, &registerStack.Xapplication);

    DoSleep(2000);
	
    while(1)
    {
    	registerCounter++;
    	if ( (registerCounter%19) == 0 )
        {
    	    /*printf("Reg: %ld, R=%d, TS=%d, AS=%d, RP=%ld, TP=%ld\n", registerCounter, registerStack.Xtransport.counters.numberOfResets,
    	                               registerStack.Xtransport.status.state,
    	                               registerStack.Xapplication.status.state,
                                       registerStack.Xdatalink.counters.numberPacketsReceived,
                                       registerStack.Xdatalink.counters.numberPacketsSent);
			*/
            //printf("%s", myBuffer);
        }
        
        
    	C1222Stack_Run(&registerStack);
    	
    	if ( messageSent && C1222Stack_IsSendMessageComplete(&registerStack) )
    	{
    	    messageSent = FALSE;
    	}
    	

        if ( !messageSent && C1222Stack_IsMessageAvailable(&registerStack) )
        {
            pMsg = C1222Stack_GetReceivedMessage(&registerStack);
            
            
            
            if ( C1222ALReceiveHistory_IsMessageAlreadyReceived(&registerReceiveHistory, pMsg) )
            {
                C1222Stack_NoteReceivedMessageHandlingIsComplete(&registerStack);
            }
            else
            {
                messageSent = C1222Client_Process_Received_Message(&registerClient, pMsg, CSV_2008, Expected_type_of_response); // Check this
            
                if ( messageSent )
            	    messageSentCount++;
        	    else
            	    messageNotSentCount++;            
            }
        }
		
		if(registerCounter > 100 && sent_request == 0){
		sent_request = 1;
		// Send some message here. It should go to the other process.
		Set_Request_parameters(&registerClient,CallingAp_InvocationId);
		/*if(!C1222Client_Send_Ident_Request(&registerClient))
			printf("Reg : Encountered error while sending request\n");
		else
		{
			printf("Reg : Ident request sent successfully\n");
			
		}*/
		if(!C1222Client_Send_Read_Request(&registerClient,&Read_request_table_id,TRUE,NULL,NULL))
			printf("Reg : Encountered error while sending request\n");
		else
		{
			Expected_type_of_response = 2;
			printf("Reg : Read request sent successfully\n");
			
		}
		CallingAp_InvocationId ++;
		}
    	DoSleep(100);
    }
}




// comm module thread

void CommModuleThread(void* param)
{
    ACSE_Message* pMsg;

    char myBuffer[100];
#ifdef WANT_TCPIP
	Unsigned8 localBuffer[200];
    Unsigned16 length;
    int bytesSent = 0,temp, i = 0;
#endif
    Boolean messageSent = FALSE;
	

	(void)param;

    CommModuleOpenLog();


    #ifdef ENABLE_LOCAL_PORT_ROUTER
	C1222Stack_Init(&commModuleStack, &commModuleLocalPortRouter, 2, TRUE, 0, FALSE, FALSE);
	#else
	C1222Stack_Init(&commModuleStack, 0, 2, TRUE, 0, FALSE, FALSE); // The port is set to 2 here . Interface number is set to 0. *****
	#endif // ENABLE_LOCAL_PORT_ROUTER
	
	CommModuleSetLogs(&commModuleStack);
	
	C1222Stack_EnableLog(&commModuleStack, TRUE);
	
	C1222Stack_SetRegistrationRetryLimitsFormat1(&commModuleStack, 0, 1);

	
	C1222AL_SetReassemblyOption(&commModuleStack.Xapplication, TRUE);
	
	C1222ALReceiveHistory_Init(&commModuleReceiveHistory, &commModuleStack.Xapplication);
	
	C1222AL_SetInterfaceName(&commModuleStack.Xapplication,"TestInterface");
	
	C1222AL_SetRFLANisSynchronizedState(&commModuleStack.Xapplication, TRUE);

    DoSleep(4000);
	
    while(1)
    {
        commModuleCounter++;
        if ( (commModuleCounter % 77) == 0 )
        {
        	/*printf("CM:  %ld, R=%d, TS=%d, AS=%d, RP=%ld, TP=%ld\n", commModuleCounter, commModuleStack.Xtransport.counters.numberOfResets,
        	                   commModuleStack.Xtransport.status.state,
        	                   commModuleStack.Xapplication.status.state,
                                       commModuleStack.Xdatalink.counters.numberPacketsReceived,
                                       commModuleStack.Xdatalink.counters.numberPacketsSent);
           */
            //printf("%s", myBuffer);
        }
        
        
        C1222Stack_Run(&commModuleStack); // Not sure
		
    	if ( messageSent && C1222Stack_IsSendMessageComplete(&commModuleStack) )
    	{
    	    messageSent = FALSE;
    	}
        

        // check for a registration request and if so, send a registration response

        if ( !messageSent && C1222Stack_IsMessageAvailable(&commModuleStack) )
        {
            pMsg = C1222Stack_GetReceivedMessage(&commModuleStack);
			printf("CM : module received a message. Relaying it appropriately\n");
            
            if ( C1222ALReceiveHistory_IsMessageAlreadyReceived(&commModuleReceiveHistory, pMsg) )
            {
                printf("Duplicate message ignored\n");
                C1222Stack_NoteReceivedMessageHandlingIsComplete(&commModuleStack);
            }

            // if this is a message to the host (registration for example) the 
            // simulateHost routine will consume the message, otherwise the
            // message should be sent to the network . It will get registration message from the other register thread. It 
			// should actually send it to the Master Relay which inturn would send it to a host. Since the Relay and host are
			// not available, it simulates a response as if it is coming from the Master Relay.
            
            else if ( !simulateHost_for_Client(pMsg, &commModuleStack, &messageSent) )
            {
                // send the message to the client ( which in this case is the server). These messages should ideally come from the register thread.

#ifdef WANT_TCPIP
                localBuffer[0] = 0x60;
                length = C1222Misc_EncodeLength(pMsg->length, &localBuffer[1]);
				for(i = 0; i< pMsg->length; i++ )
				{
					localBuffer[i + 2] = pMsg->buffer[i];
				}
				printf("CM : Message is  to be sent to the Server. Called SendToServer \n");
				
				temp = SendToServer(localBuffer, (Unsigned16)(pMsg->length+2)); 
				//temp = 1;
				if(temp == -1)
				{
					bytesSent = 0;
					printf("CM : Couldn't send data. Encountered error in TcpIpSend_to_Server\n");
				}
				else
					bytesSent = temp;
				
                printf("CM : Sent %d bytes\n", bytesSent);
#endif
                C1222Stack_NoteReceivedMessageHandlingIsComplete(&commModuleStack);
            }  
        }
		else{
			if(C1222Stack_IsMessageAvailable(&commModuleStack))
				printf("CM: There is a message available but message sent flag is wrong\n");
		
		}
        // check for and process messages from the client. Client in this case will be the server.
#ifdef WANT_TCPIP
        messageSent = HandleMessageFromServer(&commModuleStack); //Sends messages after validation, to their intended native address which is obtained from the stored list
																// of called Aptitles (if a match is found). These messages should ideally be addressed to the register thread.
#endif

        // wait a bit

        DoSleep(10);
    }
}


extern void TcpIpServerThread(void*);
extern void TcpIpReceiverThread(void*);





unsigned long mainCount = 0;

extern unsigned long g_server_port;

Boolean g_wantLogNetworkComm = FALSE;


// normally this is run with no params, and the tcpip port is 27015
// but sometimes that seems to be taken, so allow overriding the port

void main(int argc, char** argv)
{
	Unsigned32 start, next;   

    if ( argc > 1 )
    {
        g_server_port = atoi(argv[1]);
    }
    
    printf("Using TcpIp port = %d\n", g_server_port);

    srand(GetTickCount()&0xFFFF);
    
    InitializeLogCriticalSection();

    #ifdef ENABLE_LOCAL_PORT_ROUTER
    C1222LocalPortRouter_Init(&registerLocalPortRouter);
    C1222LocalPortRouter_Init(&commModuleLocalPortRouter);
    #endif // ENABLE_LOCAL_PORT_ROUTER



    _beginthread(RegisterThread, 0, NULL);
    
    DoSleep(1000);
    _beginthread(CommModuleThread, 0, NULL);

#ifdef WANT_TCPIP
	DoSleep(500);
	_beginthread(TcpIpReceiverThread, 0, NULL);
#endif
    

    start = GetTickCount();
    while(1)
    {
        mainCount++;

    	DoSleep(200);
        next = GetTickCount();

    	C1222Misc_IsrIncrementFreeRunningTime((Unsigned16)(next-start));
        start = next;
    }
}




 
// some needed routines

// should not be called
Boolean C1222Stack_SendMessageToNonC1222Stack(ACSE_Message* pMsg, Unsigned8* destinationNativeAddress, Unsigned16 addressLength)
{
    (void)pMsg;
    (void)destinationNativeAddress;
    (void)addressLength;
    
    return FALSE;
}

// I don't think anything is needed in the test routines
// maybe later - these protect changes to some things that are in different tasks in the register
void C1222EnterCriticalRegion(void)
{
}

void C1222LeaveCriticalRegion(void)
{
}

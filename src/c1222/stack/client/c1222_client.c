// This is the client program.
// Author - Vignesh

//#include "c1222al.h"
//#include "c1222server.h"
///#include "c1222response.h"
//#include "c1222misc.h"
//#include "c1222_client.h"
#include "policyengine.h"


void setmyApTitle(C1222Client * pClient, Unsigned8* Aptitle, Unsigned8 length)
{
pClient->myAptitle_length = length;
pClient->myAptitle = Aptitle;
}

void setmyMRApTitle(C1222Client * pClient, Unsigned8* Aptitle, Unsigned8 length)
{
pClient->myMRAptitle_length = length;
pClient->myMRAptitle = Aptitle;
}

void setCalled_Aptitle(C1222Client * pClient, Unsigned8 * Called_Aptitle, Unsigned8 Called_Aptitle_length)
{
pClient->Called_Aptitle = Called_Aptitle;
pClient->Called_Aptitle_length = Called_Aptitle_length;
}

void setDestination_Native_Address(C1222Client * pClient, Unsigned8 * Destination_Native_Address, Unsigned16 Destination_Native_Address_length)
{
pClient->Destination_Native_Address = Destination_Native_Address;
pClient->Destination_Native_Address_length = Destination_Native_Address_length;
}

void setCallingApInvocationId(C1222Client * pClient, Unsigned16 CallingAp_InvocationId)
{
pClient->CallingAp_InvocationId = CallingAp_InvocationId;
}
// Request for Identity Information
Unsigned16 ComposeIdentRequest_Epsem(Unsigned8* request)
{
    // I guess we will respond with 2008 format until told otherwise - I think only the collection engine will ask and we should
    // respond in 2008 format for it.
    
    Unsigned16 request_length = 0;
    //Unsigned8 * init_pos = request;
	////request++;
    
	
	//request[index++] = 0x20;      // Code for Ident Request
	request_length++;
    
	//*init_pos = (request_length &0xFF);
	request[0] = 0x01;
	request[1] = 0x20;
    
    return request_length + 1;
} 

//Read from Table request
Unsigned16 ComposeReadRequest_Epsem(Unsigned8* request, Unsigned8* tableId, Boolean isFullRead, Unsigned8* offset, Unsigned8* octet_count)
{
    // I guess we will respond with 2008 format until told otherwise - I think only the collection engine will ask and we should
    // respond in 2008 format for it.
    
    Unsigned16 request_length = 0;
	Unsigned8 index = 0;
	//request++;
	
    if(isFullRead){
		request[0] = 0x03;
		request[1] = 0x30;      //Code for Read request
		request_length++;
		
		request[2] = (*tableId) >> 8;
		request[3] = (*tableId) & 0xFF;
		
		//memcpy(&request[index], tableId, 2); // Copy table ID. Its length is 2
		//request += 2;
		request_length += 2;
	}
	else{
		request[0] = 0x08;
		request[1] = 0x3F; //Code for offset based read. Index based read not supported
		request_length++;
		//memcpy(&request[index], tableId, 2); // Copy table ID
		//index += 2;
		request[2] = (*tableId) >> 8;
		request[3] = (*tableId) & 0xFF;
		request_length += 2;
		request[4] = offset[0];
		request[5] = offset[1];
		request[6] = offset[2]; //Offset is a 24 bit word
		request_length += 3;
		request[7] = octet_count[0];
		request[8] = octet_count[1]; // octet count is a 16bit word
		request_length +=2;
	}
	
   
    return request_length + 1;
} 

//This function computes the 8 bit checksum.
Unsigned8 ComputeCRC(Unsigned8 * data, Unsigned16 size)
{
Unsigned8 checksum = 0;
Unsigned16 i = 0;
for(i = 0; i<size;i++){
checksum += data[i];

}
checksum = (~checksum)  + 1;

return checksum;

}

// Write to Table Request
Unsigned16 ComposeWriteRequest_Epsem(Unsigned8* request, Unsigned8* tableId, Boolean isFullWrite, Unsigned8* offset, Unsigned8* tabledata, Unsigned16 tabledata_length)
{
    // I guess we will respond with 2008 format until told otherwise - I think only the collection engine will ask and we should
    // respond in 2008 format for it.
    
    Unsigned16 request_length = 0;
	Unsigned8 checksum;
	Unsigned16 i = 0;	
	Unsigned8 index = 1;
	//request++;
	
	checksum = ComputeCRC(tabledata, tabledata_length);
	
    if(isFullWrite){
		request[index++] = 0x40;      //Code for full write request
		request_length++;
		//memcpy(&request[index], tableId, 2); // Copy table ID
		//request += 2;
		//index += 2;
		request[index++] = (*tableId) >> 8;
		request[index++] = (*tableId) & 0xFF;
		request_length += 2;
		
		// Start to copy the table data <count><data><checksum>
		request[index++] = ( tabledata_length >> 8); // Copy the count
		request[index++] = ( tabledata_length & 0x00FF);
		request_length += 2;
		for( i =0; i< tabledata_length; i++){ // Copy the table data
			request[index++] = tabledata[i];
			request_length ++;
		}
		request[index++] = checksum;
		request_length ++;
		request[0] = request_length;
	}
	else{
		request[index++] = 0x4F; //Code for offset based Write. Index based Write not supported
		request_length++;
		//memcpy(&request[index], tableId, 2); // Copy table ID
		//request += 2;
		//index += 2;
		request[index++] = (*tableId) >> 8;
		request[index++] = (*tableId) & 0xFF;
		request_length += 2;
		request[index++] = offset[0];
		request[index++] = offset[1];
		request[index++] = offset[2]; //Offset is a 24 bit word
		request_length += 3;
		// Start to copy the table data <count><data><checksum>
		request[index++] = ( tabledata_length >> 8); // Copy the count
		request[index++] = ( tabledata_length & 0x00FF);
		request_length += 2;
		for( i =0; i< tabledata_length; i++){ // Copy the table data
			request[index++] = tabledata[i];
			request_length ++;
		}
		request[index++] = checksum;
		request_length ++;
		request[0] = request_length;
	}
	
    
    return request_length + 1;
} 


//Log on service request
Unsigned16 ComposeLogonRequest_Epsem(Unsigned8* request, Unsigned8 * User_Info, Unsigned8 * User_id, Unsigned16 session_idle_timeout)
{
Unsigned16 request_length =0, i;
//request++;
Unsigned8 index = 1;

request[index++] = 0x50; // Code for logon request
request[index++] = User_id[0];
request[index++] = User_id[1];
request_length += 3;

//Copy 10 bytes containing user information
for(i=0;i<10;i++)
{
	request[index++] = User_Info[i];
	request_length ++;
}

request[index++] = (session_idle_timeout >> 8); //Copy the idle timeout for the session
request[index++] = (session_idle_timeout & 0xFF);
request_length +=2;
request[0] = request_length;


return request_length + 1;
}

// Log off Service request
Unsigned16 ComposeLogoffRequest_Epsem(Unsigned8* request)
{

Unsigned16 request_length = 0;
//request++;
Unsigned8 index = 1;

request[index++] = 0x52;
request_length ++;
request[0] = request_length;

return request_length + 1;

}

//Terminate Service Request
Unsigned16 ComposeTerminateRequest_Epsem(Unsigned8* request)
{

Unsigned16 request_length = 0;
Unsigned8 index = 1;
//request++;

request[index++] = 0x21;
request_length ++;
request[0] = request_length;

return request_length + 1;

}

// Disconnect Service Request
Unsigned16 ComposeDisconnectRequest_Epsem(Unsigned8* request)
{

Unsigned16 request_length = 0;
Unsigned8 index = 1;
//request++;

request[index++] = 0x22;
request_length ++;
request[0] = request_length;


return request_length + 1;

}


// Wait Service Request
Unsigned16 ComposeWaitRequest_Epsem(Unsigned8* request, Unsigned8 duration)
{

Unsigned16 request_length = 0;

Unsigned8 index = 1;
//request++;

request[index++] = 0x70;
request_length ++;
request[index++] = duration;
request_length++;
request[0] = request_length;

return request_length + 1;

}

// Resolve service Request
Unsigned16 ComposeResolveRequest_Epsem(Unsigned8* request, Unsigned8* targetNodeAptitle, Unsigned8 Aptitle_length)
{

Unsigned16 request_length = 0;
Unsigned8 index = 1;
//request++;

request[index++] = 0x25;
request_length ++;
if( targetNodeAptitle[0] == 0x80){
	request[index++] = 0x06;
	request_length ++;
	request[index++] = Aptitle_length;
	request_length ++;
	memcpy(&request[index], targetNodeAptitle, Aptitle_length);
	request_length += Aptitle_length;
	
}
else{
	request[index++] = 0x80;
	request_length ++;
	request[index++] = Aptitle_length;
	request_length ++;
	memcpy(&request[index], targetNodeAptitle, Aptitle_length);
	request_length += Aptitle_length;

}
request[0] = request_length;

return request_length + 1;

}

// Trace service Request
Unsigned16 ComposeTraceRequest_Epsem(Unsigned8* request, Unsigned8* targetNodeAptitle, Unsigned8 Aptitle_length)
{



Unsigned16 request_length = 0;
Unsigned8 * init_pos = request;
Unsigned8 index = 1;
//request++;

request[index++] = 0x26;
request_length ++;
if( targetNodeAptitle[0] == 0x80){
	request[index++] = 0x06;
	request_length ++;
	request[index++] = Aptitle_length;
	request_length ++;
	memcpy(&request[index], targetNodeAptitle, Aptitle_length);
	request_length += Aptitle_length;
	
}
else{
	request[index++] = 0x80;
	request_length ++;
	request[index++] = Aptitle_length;
	request_length ++;
	memcpy(&request[index], targetNodeAptitle, Aptitle_length);
	request_length += Aptitle_length;

}
request[0] = request_length;
return request_length + 1;

}

//TODO : Register service request.



//Create a common header for all types of requests. Fields obtained from simulatehost.c
//Calling Ap invocation id is an argument should be less than 128.
Unsigned16 Create_ACSE_Header(ACSE_Message* pRequestMsg, Unsigned8* Called_Aptitle, Unsigned8 length_of_called_Aptitle, Unsigned8* Calling_Aptitle, Unsigned8 length_of_callng_Aptitle, Unsigned16 CallingAp_InvocationId, C1222StandardVersion standardVersion) 
{
//C1222ApTitle apTitle;
    static Unsigned8 localBuffer[MAX_ACSE_HDR_MSG_SIZE];   
    
    if (standardVersion == CSV_MARCH2006 )
    {
        // Don't know how to construct a response. I think it can be ignored because it is an old standard anyway.
    }
    else
    {
        localBuffer[0] = 0x60; //ACSE identifier
        localBuffer[1] = 16 + length_of_called_Aptitle + length_of_callng_Aptitle; // length of whole ACSE message        
        localBuffer[2] = 0xA2; // Called Aptitle element or destination*
        localBuffer[3] = length_of_called_Aptitle; // Called Aptitle length
        memcpy(&localBuffer[4], Called_Aptitle, length_of_called_Aptitle);
        if ( Called_Aptitle[0] == 0x0d )
            localBuffer[4] = 0x80;
            
        localBuffer[4 + length_of_called_Aptitle] = 0xA4;//called-AP-invocation-id-element
        localBuffer[5 + length_of_called_Aptitle] = 4;//called-AP-invocation-id-integerlength. 
        localBuffer[6 + length_of_called_Aptitle] = 0x02;//called-AP-invocation-id-integer
        localBuffer[7 + length_of_called_Aptitle] = 2;//called-AP-invocation-id-length
        localBuffer[8 + length_of_called_Aptitle] = (Unsigned16)(CallingAp_InvocationId >> 8);//called-AP-invocation-id
        localBuffer[9 + length_of_called_Aptitle] = (Unsigned16)(CallingAp_InvocationId & 0xFF); // need a called ap invocation id equal to calling from msg //called-AP-invocation-id

        localBuffer[10 + length_of_called_Aptitle] = 0xA6; // Calling Aptitle element or initiator*
        localBuffer[11 + length_of_called_Aptitle] = length_of_callng_Aptitle; // Calling Aptitle length
        memcpy(&localBuffer[12 + length_of_called_Aptitle], Calling_Aptitle, length_of_callng_Aptitle);
        if ( Calling_Aptitle[0] == 0x0d )
            localBuffer[12 + length_of_called_Aptitle] = 0x80;
            
        localBuffer[12 + length_of_called_Aptitle + length_of_callng_Aptitle] = 0xA8;//calling-AP-invocation-id-element
        localBuffer[13 + length_of_called_Aptitle + length_of_callng_Aptitle] = 4; // Its length
        localBuffer[14 + length_of_called_Aptitle + length_of_callng_Aptitle] = 0x02; //calling-AP-invocation-id-integer
        localBuffer[15 + length_of_called_Aptitle + length_of_callng_Aptitle] = 2; //calling-AP-invocation-id-length
        localBuffer[16 + length_of_called_Aptitle + length_of_callng_Aptitle] = (CallingAp_InvocationId >> 8); //calling-AP-invocation-id
        localBuffer[17 + length_of_called_Aptitle + length_of_callng_Aptitle] = (CallingAp_InvocationId & 0xFF); //calling-AP-invocation-id
        
        
        pRequestMsg->buffer = &localBuffer[2];
        pRequestMsg->length = 16 + length_of_called_Aptitle + length_of_callng_Aptitle;
        pRequestMsg->maxLength = MAX_ACSE_MSG_SIZE; // Dont know. have to check it later
    }

    
	printf("Exited Create ACSSE Header\n");


}

Boolean CreateRequestMessage_Ident(ACSE_Message * pRequestMsg)
{
//printf("Entered CreateRequestMessage_Ident\n");
Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeIdentRequest_Epsem(Msg_body);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }

}

Boolean CreateRequestMessage_Read(ACSE_Message * pRequestMsg, Unsigned8* tableId, Boolean isFullRead, Unsigned8* offset, Unsigned8* octet_count)
{

Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeReadRequest_Epsem(Msg_body, tableId, isFullRead, offset, octet_count);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }



}

Boolean CreateRequestMessage_Write(ACSE_Message * pRequestMsg, Unsigned8* tableId, Boolean isFullWrite, Unsigned8* offset, Unsigned8* tabledata, Unsigned16 tabledata_length)
{

Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeWriteRequest_Epsem(Msg_body, tableId, isFullWrite, offset, tabledata, tabledata_length);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }


}


Boolean CreateRequestMessage_Logon(ACSE_Message * pRequestMsg, Unsigned8 * User_Info, Unsigned8 * User_id, Unsigned16 session_idle_timeout)
{


Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeLogonRequest_Epsem(Msg_body, User_Info, User_id, session_idle_timeout);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }


}


Boolean CreateRequestMessage_Logoff(ACSE_Message * pRequestMsg)
{
Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeLogoffRequest_Epsem(Msg_body);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }


}


Boolean CreateRequestMessage_Terminate(ACSE_Message * pRequestMsg)
{

Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeTerminateRequest_Epsem(Msg_body);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }


}


Boolean CreateRequestMessage_Disconnect(ACSE_Message * pRequestMsg)
{
Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeDisconnectRequest_Epsem(Msg_body);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }



}


Boolean CreateRequestMessage_Wait(ACSE_Message * pRequestMsg,Unsigned8 duration)
{

Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeWaitRequest_Epsem(Msg_body, duration);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }


}


Boolean CreateRequestMessage_Resolve(ACSE_Message * pRequestMsg, Unsigned8* targetNodeAptitle, Unsigned8 Aptitle_length)
{

Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeResolveRequest_Epsem(Msg_body, targetNodeAptitle, Aptitle_length);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }


}

Boolean CreateRequestMessage_Trace(ACSE_Message * pRequestMsg, Unsigned8* targetNodeAptitle, Unsigned8 Aptitle_length)
{

Epsem EpsemMsg;
Unsigned8 Msg_body[MAX_EPSEM_BODY_SIZE];
Unsigned16 Msg_length;
int i = 0;
EpsemMsg.epsemControl = 0x80; // The default control pattern.
Msg_length = ComposeTraceRequest_Epsem(Msg_body, targetNodeAptitle, Aptitle_length);
EpsemMsg.length = Msg_length;
EpsemMsg.maxLength = MAX_EPSEM_BODY_SIZE;
EpsemMsg.buffer = Msg_body;
for(i = 0; i< Msg_length; i ++){
	EpsemMsg.buffer[i] = Msg_body[i];
	printf("Reg : Epsem buffer[%d] = %d\n",i,EpsemMsg.buffer[i]);
}
memset(EpsemMsg.deviceClass, 0, 4);
 if ( Epsem_Validate(&EpsemMsg) )
        {
            ACSE_Message_SetUserInformation(pRequestMsg, &EpsemMsg, 0);
			//printf("Exited CreateRequestMessage_Ident\n");
			return TRUE;
            
		} // end if the epsem validated
		else
		{
			printf("Failure in Epsem Validation\n");
			return FALSE;
            
        }



}



C1222Client_Init(C1222Client * pClient, C1222Stack * pStack, Unsigned8 * requestMsgbuffer, Unsigned8* responseMsgbuffer)
{
pClient->pStack = pStack;
pClient->requestMsgbuffer = requestMsgbuffer;
pClient->responseMsgbuffer = responseMsgbuffer;

}

Boolean C1222Client_Send_Message(C1222Stack * pStack, ACSE_Message* Msg, Unsigned8* Destination_Native_Address, Unsigned16 Destination_Native_Address_length)
{
	return C1222AL_SendMessage(&pStack->Xapplication, Msg, Destination_Native_Address, Destination_Native_Address_length);
}


Boolean C1222Client_Send_Ident_Request(C1222Client * pClient)
{
ACSE_Message pRequestMsg;
//printf("Entered Send Ident request successfully\n");
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008) ;
if(!CreateRequestMessage_Ident(&pRequestMsg))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Ident Request Message failed\n");
			return FALSE;
		}
}

pClient->requestMsgbuffer = pRequestMsg.buffer;
//printf("Exited Send Ident request successfully\n");
return TRUE;
}


Boolean C1222Client_Send_Read_Request(C1222Client * pClient,Unsigned8* tableId, Boolean isFullRead, Unsigned8* offset, Unsigned8* octet_count)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008); 
if(!CreateRequestMessage_Read(&pRequestMsg, tableId, isFullRead, offset, octet_count))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Read Request Message failed\n");
			return FALSE;
		}
}

pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Write_Request(C1222Client * pClient,Unsigned8* tableId, Boolean isFullWrite, Unsigned8* offset, Unsigned8* tabledata, Unsigned16 tabledata_length)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008) ;
if(!CreateRequestMessage_Write(&pRequestMsg, tableId, isFullWrite, offset, tabledata, tabledata_length))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
		
			printf("Reg : Send Write Message failed\n");
			return FALSE;
		}
}

pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Logon_Request(C1222Client * pClient, Unsigned8 * User_Info, Unsigned8 * User_id, Unsigned16 session_idle_timeout)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008) ;
if(!CreateRequestMessage_Logon(&pRequestMsg, User_Info, User_id, session_idle_timeout))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Log on Request Message failed\n");
			return FALSE;
		}
}

pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Logoff_Request(C1222Client * pClient)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008) ;
if(!CreateRequestMessage_Logoff(&pRequestMsg))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Logoff request Message failed\n");
			return FALSE;
			
		}
}
pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Terminate_Request(C1222Client * pClient)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008) ;
if(!CreateRequestMessage_Terminate(&pRequestMsg))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Terminate Request Message failed\n");
			return FALSE;
		}
}
pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Disconnect_Request(C1222Client * pClient)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008) ;
if(!CreateRequestMessage_Disconnect(&pRequestMsg))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Diconnect request Message failed\n");
			return FALSE;
		}
}
pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Wait_Request(C1222Client * pClient, Unsigned8 duration)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008) ;
if(!CreateRequestMessage_Wait(&pRequestMsg, duration))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Wait request Message failed\n");
			return FALSE;
		}
}
pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Resolve_Request(C1222Client * pClient, Unsigned8* targetNodeAptitle, Unsigned8 Aptitle_length)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008); 
if(!CreateRequestMessage_Resolve(&pRequestMsg, targetNodeAptitle, Aptitle_length))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			
			printf("Reg : Send Resolve request Message failed\n");
			return FALSE;
		}
}
pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}


Boolean C1222Client_Send_Trace_Request(C1222Client * pClient, Unsigned8* targetNodeAptitle, Unsigned8 Aptitle_length)
{
ACSE_Message pRequestMsg;
Create_ACSE_Header( &pRequestMsg, pClient->Called_Aptitle, pClient->Called_Aptitle_length, pClient->myAptitle, pClient->myAptitle_length, pClient->CallingAp_InvocationId, CSV_2008); 
if(!CreateRequestMessage_Trace(&pRequestMsg, targetNodeAptitle, Aptitle_length))
	return FALSE;
else
{
		if(C1222Client_Send_Message(pClient->pStack, &pRequestMsg, pClient->Destination_Native_Address, pClient->Destination_Native_Address_length) == FALSE){
			printf("Reg : Send Trace request Message failed\n");
			return FALSE;
		}
}
pClient->requestMsgbuffer = pRequestMsg.buffer;
return TRUE;
}

Boolean C1222Client_Process_Received_Message(C1222Client * pClient, ACSE_Message* pReceived_Msg, C1222StandardVersion Version, Unsigned8 Expected_type_of_response)
{
	int i = 0;

	if(!C1222PolicyEngine_Process_Received_Message(pClient,pReceived_Msg,Version,Expected_type_of_response)) // Process message through the policy engine to detect malice.
		printf("Reg : Policy engine detected a problem with the received message. It should be dropped\n");
	else{
		printf("Reg : Received message passed all policy checks. Should be safe. The received Response is \n");
		for(i = 0 ; i < pReceived_Msg->length; i ++)
			printf("%02X ",pReceived_Msg->buffer[i]);
		printf("\n\n");
		
	}
}



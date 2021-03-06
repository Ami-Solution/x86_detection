#include "c1222al.h"
#include "c1222server.h"
#include "c1222response.h"
#include "c1222misc.h"
#include "c1222_client.h"
//#include <vector.h>

Boolean epsemControl_Checks(Unsigned8 epsemControl, Boolean * isEd_Class_included);
Boolean Validate_Ident_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Read_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Write_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Logon_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Logoff_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Terminate_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Disconnect_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Wait_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Resolve_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Validate_Trace_Response(Unsigned8* buffer, Unsigned16 length, Boolean isEd_Class_included);
Boolean Check_Received_Message_Semantics(ACSE_Message* pReceived_Msg, C1222StandardVersion standardVersion, Unsigned8 Type_of_sent_message);
void EntropyAnalysis(Unsigned8 * buff, Unsigned16 length);
//void OneGramMeterbits(Unsigned8 * buffer, Unsigned16 length, int offset, int &no_of_hits, int &no_of_misses);
void bitsAnalysis(Unsigned8 * buffer, Unsigned16 length);

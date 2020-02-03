
#pragma pack(1)


typedef struct tagCONX_REQ         // Client Connection Request
{
    T_TXT HubId[11];               //
    T_NUM ProVer[4];               //
    T_NUM SessConf[16];            //
    T_TXT AuthData[8];             //
    T_TXT ETX;                     //

} CONX_REQ, *PCONX_REQ;

#define SIZEOF_CONX_REQ_MIN (40)
#define SIZEOF_CONX_REQ_MAX (40)
#define CONX_REQ_PRINTF_FORMAT "%s""%sHubId[%.11s];%sProVer[%.4s];%sSessConf[%.16s];%sAuthData[%.8s];""%s"
#define CONX_REQ_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->HubId, seperator, obj->ProVer, seperator, obj->SessConf, seperator, obj->AuthData, suffix

typedef struct tagCONX_ACK         // Connection Acceptance by the HUB
{
    T_NUM SessConf[16];            //
    T_TXT ETX;                     //

} CONX_ACK, *PCONX_ACK;

#define SIZEOF_CONX_ACK_MIN (17)
#define SIZEOF_CONX_ACK_MAX (17)
#define CONX_ACK_PRINTF_FORMAT "%s""%sSessConf[%.16s];""%s"
#define CONX_ACK_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->SessConf, suffix

typedef struct tagCONX_NCK         // Connection Refusal by the HUB
{
    T_NUM Reason[2];               //
    T_TXT ETX;                     //

} CONX_NCK, *PCONX_NCK;

#define SIZEOF_CONX_NCK_MIN (3)
#define SIZEOF_CONX_NCK_MAX (3)
#define CONX_NCK_PRINTF_FORMAT "%s""%sReason[%.2s];""%s"
#define CONX_NCK_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->Reason, suffix

typedef struct tagDCNX_REQ         // Disconnection Request
{
    T_NUM Reason[2];               //
    T_NUM LastSeqNo[8];            //
    T_TXT ETX;                     //

} DCNX_REQ, *PDCNX_REQ;

#define SIZEOF_DCNX_REQ_MIN (11)
#define SIZEOF_DCNX_REQ_MAX (11)
#define DCNX_REQ_PRINTF_FORMAT "%s""%sReason[%.2s];%sLastSeqNo[%.8s];""%s"
#define DCNX_REQ_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->Reason, seperator, obj->LastSeqNo, suffix

typedef struct tagDCNX_ACK         // Disconnection Request Acknowledgement
{
    T_NUM LastSeqNo[8];            //
    T_TXT ETX;                     //

} DCNX_ACK, *PDCNX_ACK;

#define SIZEOF_DCNX_ACK_MIN (9)
#define SIZEOF_DCNX_ACK_MAX (9)
#define DCNX_ACK_PRINTF_FORMAT "%s""%sLastSeqNo[%.8s];""%s"
#define DCNX_ACK_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LastSeqNo, suffix

typedef struct tagDATA_MSG         // Data Transmission
{
    T_NUM SeqNo[8];                //
    T_NUM AdminLen[4];             //
    T_NUM BusinessLen[4];          //

} DATA_MSG, *PDATA_MSG;

#define SIZEOF_DATA_MSG_MIN (16)
#define SIZEOF_DATA_MSG_MAX (16)
#define DATA_MSG_PRINTF_FORMAT "%s""%sSeqNo[%.8s];%sAdminLen[%.4s];%sBusinessLen[%.4s];""%s"
#define DATA_MSG_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->SeqNo, seperator, obj->AdminLen, seperator, obj->BusinessLen, suffix

typedef struct tagSTRT_REQ         // Transmission/Retransmission Request
{
    T_TXT MessID[24];              //
    T_TXT ETX;                     //

} STRT_REQ, *PSTRT_REQ;

#define SIZEOF_STRT_REQ_MIN (25)
#define SIZEOF_STRT_REQ_MAX (25)
#define STRT_REQ_PRINTF_FORMAT "%s""%sMessID[%.24s];""%s"
#define STRT_REQ_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MessID, suffix

typedef struct tagSTRT_ACK         // Transmission/Retransmission Request Acknowledgement
{
    T_NUM NextSeqNo[8];            //
    T_TXT MessID[24];              //
    T_TXT ETX;                     //

} STRT_ACK, *PSTRT_ACK;

#define SIZEOF_STRT_ACK_MIN (33)
#define SIZEOF_STRT_ACK_MAX (33)
#define STRT_ACK_PRINTF_FORMAT "%s""%sNextSeqNo[%.8s];%sMessID[%.24s];""%s"
#define STRT_ACK_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->NextSeqNo, seperator, obj->MessID, suffix

typedef struct tagSTRT_NCK         // Transmission/Retransmission Request Refusal
{
    T_NUM Reason[2];               //
    T_TXT MessID[24];              //
    T_TXT ETX;                     //

} STRT_NCK, *PSTRT_NCK;

#define SIZEOF_STRT_NCK_MIN (27)
#define SIZEOF_STRT_NCK_MAX (27)
#define STRT_NCK_PRINTF_FORMAT "%s""%sReason[%.2s];%sMessID[%.24s];""%s"
#define STRT_NCK_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->Reason, seperator, obj->MessID, suffix

typedef struct tagSYNC_REQ         // Acknowledgement Request by Data Source
{
    T_TXT ETX;                     //

} SYNC_REQ, *PSYNC_REQ;

#define SIZEOF_SYNC_REQ_MIN (1)
#define SIZEOF_SYNC_REQ_MAX (1)
#define SYNC_REQ_PRINTF_FORMAT "%s""""%s"
#define SYNC_REQ_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, suffix

typedef struct tagSYNC_ACK         // Acknowledgement by Data Receiver
{
    T_NUM LastSeqNo[8];            //
    T_TXT MessID[24];              //
    T_TXT ETX;                     //

} SYNC_ACK, *PSYNC_ACK;

#define SIZEOF_SYNC_ACK_MIN (33)
#define SIZEOF_SYNC_ACK_MAX (33)
#define SYNC_ACK_PRINTF_FORMAT "%s""%sLastSeqNo[%.8s];%sMessID[%.24s];""%s"
#define SYNC_ACK_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LastSeqNo, seperator, obj->MessID, suffix

typedef struct tagERR_INDI         // Error Indication by Data Receiver
{
    T_NUM ErrCode[2];              //
    T_NUM ErrDetail[2];            //
    T_NUM LastSeqNo[8];            //
    T_NUM MsgLen[4];               //

} ERR_INDI, *PERR_INDI;

#define SIZEOF_ERR_INDI_MIN (16)
#define SIZEOF_ERR_INDI_MAX (16)
#define ERR_INDI_PRINTF_FORMAT "%s""%sErrCode[%.2s];%sErrDetail[%.2s];%sLastSeqNo[%.8s];%sMsgLen[%.4s];""%s"
#define ERR_INDI_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->ErrCode, seperator, obj->ErrDetail, seperator, obj->LastSeqNo, seperator, obj->MsgLen, suffix

typedef struct tagSRVC_MSG         // Service Message
{
    T_TXT Type[4];                 //
    T_NUM ServiceLen[4];           //

} SRVC_MSG, *PSRVC_MSG;

#define SIZEOF_SRVC_MSG_MIN (8)
#define SIZEOF_SRVC_MSG_MAX (8)
#define SRVC_MSG_PRINTF_FORMAT "%s""%sType[%.4s];%sServiceLen[%.4s];""%s"
#define SRVC_MSG_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->Type, seperator, obj->ServiceLen, suffix

typedef struct tagPRSC_MSG         // Heartbeat Message
{
    T_TXT ETX;                     //

} PRSC_MSG, *PPRSC_MSG;

#define SIZEOF_PRSC_MSG_MIN (1)
#define SIZEOF_PRSC_MSG_MAX (1)
#define PRSC_MSG_PRINTF_FORMAT "%s""""%s"
#define PRSC_MSG_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, suffix

typedef struct tagADMIN_DATA_E0    // E0 Admin Data Type
{
    T_TXT Type[2];                 // Indicates the admin data type.
    T_TXT MsgId[24];               // Message identifier assigned by the sender.
    T_NUM SendTime[12];            // Send timestamp. Format: MMDDHHmmsscc. Date and time of message transmission.
    T_NUM ReceiptTime[12];         // Receive timestamp. Format: MMDDHHmmsscc. Date and time of message reception.
    T_NUM DeliveryTimeout[6];      // Format: HHmmss. This field is a timestamp, NOT a delay. For future use.
    T_TXT RouteData[11];           // Routable field. May be used by the HUB for routing data: ROUTE_DATA.
    T_TXT Filler[5];               // Characters to fill E0 administrative data to its defined size.

} ADMIN_DATA_E0, *PADMIN_DATA_E0;

#define SIZEOF_ADMIN_DATA_E0_MIN (72)
#define SIZEOF_ADMIN_DATA_E0_MAX (72)
#define ADMIN_DATA_E0_PRINTF_FORMAT "%s""%sType[%.2s];%sMsgId[%.24s];%sSendTime[%.12s];%sReceiptTime[%.12s];%sDeliveryTimeout[%.6s];%sRouteData[%.11s];%sFiller[%.5s];""%s"
#define ADMIN_DATA_E0_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->Type, seperator, obj->MsgId, seperator, obj->SendTime, seperator, obj->ReceiptTime, seperator, obj->DeliveryTimeout, seperator, obj->RouteData, seperator, obj->Filler, suffix

typedef struct tagADMIN_DATA_E1    // E1 Admin Data Type
{
    T_TXT Type[2];                 // Indicates the admin data type.
    T_TXT MsgId[24];               // Message identifier assigned by the sender.
    T_NUM SendTime[12];            // Send timestamp. Format: MMDDHHmmsscc. Date and time of message transmission.
    T_NUM ReceiptTime[12];         // Receive timestamp. Format: MMDDHHmmsscc. Date and time of message reception.
    T_NUM DeliveryTimeout[6];      // Format: HHmmss. This field is a timestamp, NOT a delay. For future use.
    T_TXT Filler[8];               // Characters to fill E1 administrative data to its defined size.

} ADMIN_DATA_E1, *PADMIN_DATA_E1;

#define SIZEOF_ADMIN_DATA_E1_MIN (64)
#define SIZEOF_ADMIN_DATA_E1_MAX (64)
#define ADMIN_DATA_E1_PRINTF_FORMAT "%s""%sType[%.2s];%sMsgId[%.24s];%sSendTime[%.12s];%sReceiptTime[%.12s];%sDeliveryTimeout[%.6s];%sFiller[%.8s];""%s"
#define ADMIN_DATA_E1_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->Type, seperator, obj->MsgId, seperator, obj->SendTime, seperator, obj->ReceiptTime, seperator, obj->DeliveryTimeout, seperator, obj->Filler, suffix

typedef struct tagADMIN_DATA_M0    // M0 Admin Data Type
{
    T_TXT Type[2];                 // Indicates the admin data type.
    T_TXT MsgId[24];               // Message identifier assigned by the sender.
    T_NUM SendTime[12];            // Send timestamp. Format: MMDDHHmmsscc. Date and time of message transmission.
    T_NUM ReceiptTime[12];         // Receive timestamp. Format: MMDDHHmmsscc. Date and time of message reception.
    T_NUM DeliveryTimeout[6];      // Format: HHmmss. This field is a timestamp, NOT a delay. For future use.
    T_TXT RouteData[11];           // Routable field. May be used by the HUB for routing data: ROUTE_DATA.
    T_TXT OnMember[8];             // Member ID.
    T_TXT Domain;                  //
    T_TXT Dest[11];                //
    T_TXT Filler[41];              // Characters to fill M0 administrative data to its defined size.

} ADMIN_DATA_M0, *PADMIN_DATA_M0;

#define SIZEOF_ADMIN_DATA_M0_MIN (128)
#define SIZEOF_ADMIN_DATA_M0_MAX (128)
#define ADMIN_DATA_M0_PRINTF_FORMAT "%s""%sType[%.2s];%sMsgId[%.24s];%sSendTime[%.12s];%sReceiptTime[%.12s];%sDeliveryTimeout[%.6s];%sRouteData[%.11s];%sOnMember[%.8s];%sDomain[%.1s];%sDest[%.11s];%sFiller[%.41s];""%s"
#define ADMIN_DATA_M0_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->Type, seperator, obj->MsgId, seperator, obj->SendTime, seperator, obj->ReceiptTime, seperator, obj->DeliveryTimeout, seperator, obj->RouteData, seperator, obj->OnMember, seperator, &obj->Domain.data, seperator, obj->Dest, seperator, obj->Filler, suffix


#pragma pack()



#pragma pack(1)


typedef struct tagSLE0401K        // Declaration
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT PrincCode[8];           // Principal code
    T_TXT Sign;                   // Sign of the declaring party’s trade
    T_NUM Qty[9];                 // Quantity
    T_TXT SecCode[12];            // Security code
    struct tagPrice               // Price
    {
        T_TXT Ift;                // Format indicator
        T_NUM Qmt[9];             // Data

    } Price;
    T_TXT ClearCode[8];           // Clearer code
    T_TXT OrderType;              // Order type
    T_TXT Filler1;                // Filler1
    T_TXT Filler2[3];             // Filler2
    T_TXT Memo[10];               // Memo
    T_TXT CtprtCode[8];           // Counterpart member code
    T_TXT CtprtPrincCode[8];      // Counterpart principal code
    T_TXT OpeInd;                 // Operation type indicator
    T_NUM StartVWAP[6];           // Start time for VWAP computation period
    T_NUM EndVWAP[6];             // End time for VWAP computation period
    T_TXT ClientAcct[16];         // Client account
    T_TXT TraderID[8];            // Trader ID
    T_TXT CtprtTraderID[8];       // Counterpart Trader ID
    struct tagCBIC                // BIC Code of the Member
    {
        T_TXT CBank[4];           // Bank Code
        T_TXT Country[2];         // Country Code
        T_TXT CTown[2];           // Town Code
        T_TXT CBranch[3];         // Branch Code

    } CBIC;
    T_TXT ClearCodeCross[8];      // Clearer code for seller in cross situation
    T_TXT OrderTYpeCross;         // Order type for seller in cross situation
    T_TXT ClientAcctCross[16];    // Client account number for seller in cross situation
    T_TXT MemoCross[10];          // Memo for seller in cross situation
    T_TXT Filler3[30];            // Filler3

} SLE0401K, *PSLE0401K;

#define SIZEOF_SLE0401K_MIN (220)
#define SIZEOF_SLE0401K_MAX (220)
#define SLE0401K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sPrincCode[%.8s];%sSign[%.1s];%sQty[%.9s];%sSecCode[%.12s];%sPrice.Ift[%.1s];%sPrice.Qmt[%.9s];%sClearCode[%.8s];%sOrderType[%.1s];%sFiller1[%.1s];%sFiller2[%.3s];%sMemo[%.10s];%sCtprtCode[%.8s];%sCtprtPrincCode[%.8s];%sOpeInd[%.1s];%sStartVWAP[%.6s];%sEndVWAP[%.6s];%sClientAcct[%.16s];%sTraderID[%.8s];%sCtprtTraderID[%.8s];%sCBIC.CBank[%.4s];%sCBIC.Country[%.2s];%sCBIC.CTown[%.2s];%sCBIC.CBranch[%.3s];%sClearCodeCross[%.8s];%sOrderTYpeCross[%.1s];%sClientAcctCross[%.16s];%sMemoCross[%.10s];%sFiller3[%.30s];""%s"
#define SLE0401K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->PrincCode, seperator, &obj->Sign.data, seperator, obj->Qty, seperator, obj->SecCode, seperator, &obj->Price.Ift.data, seperator, obj->Price.Qmt, seperator, obj->ClearCode, seperator, &obj->OrderType.data, seperator, &obj->Filler1.data, seperator, obj->Filler2, seperator, obj->Memo, seperator, obj->CtprtCode, seperator, obj->CtprtPrincCode, seperator, &obj->OpeInd.data, seperator, obj->StartVWAP, seperator, obj->EndVWAP, seperator, obj->ClientAcct, seperator, obj->TraderID, seperator, obj->CtprtTraderID, seperator, obj->CBIC.CBank, seperator, obj->CBIC.Country, seperator, obj->CBIC.CTown, seperator, obj->CBIC.CBranch, seperator, obj->ClearCodeCross, seperator, &obj->OrderTYpeCross.data, seperator, obj->ClientAcctCross, seperator, obj->MemoCross, seperator, obj->Filler3, suffix

typedef struct tagSLE0411K        // Declaration reception notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference
    T_TXT PrevInd;                // Previous day trade indicator
    T_TXT MembCode[8];            // Member code
    T_TXT PrincCode[8];           // Principal code (if declaration mandated)
    T_TXT Sign;                   // Sign of declaring party's trade
    T_NUM Qty[9];                 // Quantity
    T_TXT SecCode[12];            // Security code
    struct tagPrice               // Price
    {
        T_TXT Ift;                // Format indicator
        T_NUM Qmt[9];             // Data

    } Price;
    T_TXT ClearCode[8];           // Clearer code
    T_TXT OrderType;              // Order type
    T_TXT Filler1;                // Filler1
    T_TXT Filler2[3];             // Filler2
    T_TXT Memo[10];               // Memo
    T_TXT CtprtCode[8];           // Counterpart member code
    T_TXT CtprtPrincCode[8];      // Counterpart principal code (if counterpart declaration mandated)
    T_NUM Time[6];                // Time declaration received
    T_TXT OpeInd;                 // Operation type indicator
    T_NUM StartVWAP[6];           // Start time for VWAP computation period
    T_NUM EndVWAP[6];             // End time for VWAP computation period
    T_TXT ClientAcct[16];         // Client account number
    T_TXT TraderID[8];            // Trader ID
    T_TXT CtprtTraderID[8];       // Counterpart Trader ID
    struct tagCBIC                // BIC Code of the Member
    {
        T_TXT CBank[4];           // Bank Code
        T_TXT Country[2];         // Country Code
        T_TXT CTown[2];           // Town Code
        T_TXT CBranch[3];         // Branch Code

    } CBIC;
    T_TXT Filler3[30];            // Filler3

} SLE0411K, *PSLE0411K;

#define SIZEOF_SLE0411K_MIN (216)
#define SIZEOF_SLE0411K_MAX (216)
#define SLE0411K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sPrevInd[%.1s];%sMembCode[%.8s];%sPrincCode[%.8s];%sSign[%.1s];%sQty[%.9s];%sSecCode[%.12s];%sPrice.Ift[%.1s];%sPrice.Qmt[%.9s];%sClearCode[%.8s];%sOrderType[%.1s];%sFiller1[%.1s];%sFiller2[%.3s];%sMemo[%.10s];%sCtprtCode[%.8s];%sCtprtPrincCode[%.8s];%sTime[%.6s];%sOpeInd[%.1s];%sStartVWAP[%.6s];%sEndVWAP[%.6s];%sClientAcct[%.16s];%sTraderID[%.8s];%sCtprtTraderID[%.8s];%sCBIC.CBank[%.4s];%sCBIC.Country[%.2s];%sCBIC.CTown[%.2s];%sCBIC.CBranch[%.3s];%sFiller3[%.30s];""%s"
#define SLE0411K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, &obj->PrevInd.data, seperator, obj->MembCode, seperator, obj->PrincCode, seperator, &obj->Sign.data, seperator, obj->Qty, seperator, obj->SecCode, seperator, &obj->Price.Ift.data, seperator, obj->Price.Qmt, seperator, obj->ClearCode, seperator, &obj->OrderType.data, seperator, &obj->Filler1.data, seperator, obj->Filler2, seperator, obj->Memo, seperator, obj->CtprtCode, seperator, obj->CtprtPrincCode, seperator, obj->Time, seperator, &obj->OpeInd.data, seperator, obj->StartVWAP, seperator, obj->EndVWAP, seperator, obj->ClientAcct, seperator, obj->TraderID, seperator, obj->CtprtTraderID, seperator, obj->CBIC.CBank, seperator, obj->CBIC.Country, seperator, obj->CBIC.CTown, seperator, obj->CBIC.CBranch, seperator, obj->Filler3, suffix

typedef struct tagSLE0412K        // Notification of a declaration issued by the counterpart
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference
    T_TXT PrevInd;                // Previous day trade indicator
    T_TXT MembCode[8];            // Member code
    T_TXT PrincCode[8];           // Principal code (if declaration mandated)
    T_TXT Sign;                   // Sign of declaring party's trade
    T_NUM Qty[9];                 // Quantity
    T_TXT SecCode[12];            // Security code
    struct tagPrice               // Price
    {
        T_TXT Ift;                // Format indicator
        T_NUM Qmt[9];             // Data

    } Price;
    T_TXT Filler1[8];             // Filler1
    T_TXT Filler2;                // Filler2
    T_TXT Filler3;                // Filler3
    T_TXT Filler4[3];             // Filler4
    T_TXT Filler5[10];            // Filler5
    T_TXT CtprtCode[8];           // Counterpart member code
    T_TXT CtprtPrincCode[8];      // Counterpart principal code (if counterpart declaration mandated)
    T_NUM Time[6];                // Time declaration received
    T_TXT OpeInd;                 // Operation type indicator
    T_NUM StartVWAP[6];           // Start time for VWAP computation period
    T_NUM EndVWAP[6];             // End time for VWAP computation period
    T_TXT TraderID[8];            // Trader ID
    T_TXT CtprtTraderID[8];       // Counterpart Trader ID
    T_TXT Filler6[30];            // Filler6

} SLE0412K, *PSLE0412K;

#define SIZEOF_SLE0412K_MIN (189)
#define SIZEOF_SLE0412K_MAX (189)
#define SLE0412K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sPrevInd[%.1s];%sMembCode[%.8s];%sPrincCode[%.8s];%sSign[%.1s];%sQty[%.9s];%sSecCode[%.12s];%sPrice.Ift[%.1s];%sPrice.Qmt[%.9s];%sFiller1[%.8s];%sFiller2[%.1s];%sFiller3[%.1s];%sFiller4[%.3s];%sFiller5[%.10s];%sCtprtCode[%.8s];%sCtprtPrincCode[%.8s];%sTime[%.6s];%sOpeInd[%.1s];%sStartVWAP[%.6s];%sEndVWAP[%.6s];%sTraderID[%.8s];%sCtprtTraderID[%.8s];%sFiller6[%.30s];""%s"
#define SLE0412K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, &obj->PrevInd.data, seperator, obj->MembCode, seperator, obj->PrincCode, seperator, &obj->Sign.data, seperator, obj->Qty, seperator, obj->SecCode, seperator, &obj->Price.Ift.data, seperator, obj->Price.Qmt, seperator, obj->Filler1, seperator, &obj->Filler2.data, seperator, &obj->Filler3.data, seperator, obj->Filler4, seperator, obj->Filler5, seperator, obj->CtprtCode, seperator, obj->CtprtPrincCode, seperator, obj->Time, seperator, &obj->OpeInd.data, seperator, obj->StartVWAP, seperator, obj->EndVWAP, seperator, obj->TraderID, seperator, obj->CtprtTraderID, seperator, obj->Filler6, suffix

typedef struct tagSLE0402K        // Declaration cancellation request
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference (attributed to the declaration to be cancelled)
    T_TXT Filler1[30];            // Filler1

} SLE0402K, *PSLE0402K;

#define SIZEOF_SLE0402K_MIN (66)
#define SIZEOF_SLE0402K_MAX (66)
#define SLE0402K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0402K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0413K        // Declaration cancellation notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference
    T_TXT Filler1[30];            // Filler1

} SLE0413K, *PSLE0413K;

#define SIZEOF_SLE0413K_MIN (66)
#define SIZEOF_SLE0413K_MAX (66)
#define SLE0413K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0413K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0403K        // Declaration refusal request issued by the counterpart
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference (attributed to the declaration issued by the counterpart)
    T_TXT Filler1[30];            // Filler1

} SLE0403K, *PSLE0403K;

#define SIZEOF_SLE0403K_MIN (66)
#define SIZEOF_SLE0403K_MAX (66)
#define SLE0403K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0403K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0414K        // Declaration refusal notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference
    T_TXT Filler1[30];            // Filler1

} SLE0414K, *PSLE0414K;

#define SIZEOF_SLE0414K_MIN (66)
#define SIZEOF_SLE0414K_MAX (66)
#define SLE0414K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0414K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0415K        // Matching notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference
    T_TXT TCSCtprtRef[16];        // TCS counterparty reference
    T_TXT Filler1[30];            // Filler1

} SLE0415K, *PSLE0415K;

#define SIZEOF_SLE0415K_MIN (82)
#define SIZEOF_SLE0415K_MAX (82)
#define SLE0415K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sTCSCtprtRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0415K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, obj->TCSCtprtRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0416K        // Elimination notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference
    T_TXT Filler1[30];            // Filler1

} SLE0416K, *PSLE0416K;

#define SIZEOF_SLE0416K_MIN (66)
#define SIZEOF_SLE0416K_MAX (66)
#define SLE0416K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0416K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0404K        // TCS trade cancellation request
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT TCSRef[16];             // TCS reference
    T_TXT Filler1[30];            // Filler1

} SLE0404K, *PSLE0404K;

#define SIZEOF_SLE0404K_MIN (66)
#define SIZEOF_SLE0404K_MAX (66)
#define SLE0404K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sTCSRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0404K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->TCSRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0417K        // TCS trade cancellation notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT CancelInd;              // Cancellation indicator
    T_TXT TCSRef[16];             // TCS reference
    T_TXT TCSCtprtRef[16];        // TCS counterparty reference
    T_TXT Filler1[30];            // Filler1

} SLE0417K, *PSLE0417K;

#define SIZEOF_SLE0417K_MIN (83)
#define SIZEOF_SLE0417K_MAX (83)
#define SLE0417K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sCancelInd[%.1s];%sTCSRef[%.16s];%sTCSCtprtRef[%.16s];%sFiller1[%.30s];""%s"
#define SLE0417K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, &obj->CancelInd.data, seperator, obj->TCSRef, seperator, obj->TCSCtprtRef, seperator, obj->Filler1, suffix

typedef struct tagSLE0418K        // Rejection notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_NUM RejCode[6];             // Rejection code
    T_TXT RejName[71];            // Rejection name
    T_TXT MssgRej[200];           // Message rejected
    T_TXT Filler1[30];            // Filler1

} SLE0418K, *PSLE0418K;

#define SIZEOF_SLE0418K_MIN (327)
#define SIZEOF_SLE0418K_MAX (327)
#define SLE0418K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sRejCode[%.6s];%sRejName[%.71s];%sMssgRej[%.200s];%sFiller1[%.30s];""%s"
#define SLE0418K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->RejCode, seperator, obj->RejName, seperator, obj->MssgRej, seperator, obj->Filler1, suffix

typedef struct tagSLE0419K        // Start of TCS trading session notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT Filler1[30];            // Filler1

} SLE0419K, *PSLE0419K;

#define SIZEOF_SLE0419K_MIN (50)
#define SIZEOF_SLE0419K_MAX (50)
#define SLE0419K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sFiller1[%.30s];""%s"
#define SLE0419K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->Filler1, suffix

typedef struct tagSLE0420K        // End of TCS trading session notice
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT Filler1[30];            // Filler1

} SLE0420K, *PSLE0420K;

#define SIZEOF_SLE0420K_MIN (50)
#define SIZEOF_SLE0420K_MAX (50)
#define SLE0420K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sFiller1[%.30s];""%s"
#define SLE0420K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->Filler1, suffix

typedef struct tagSLE0502K        // Entry of a client account
{
    T_TXT LRfIntAdfMsg[16];       // Internal Subscriber Reference
    T_NUM CFon[4];                // Function Code
    T_TXT DSeaBs[8];              // Trading Date
    T_TXT HSeaBs[6];              // Trading Time
    T_TXT NCptePosIptOm[16];      // Clearing Client Account Number
    T_TXT ICptePosIptOm;          // Clearing Client Account Status
    struct tagCBIC                // BIC Code of the Member
    {
        T_TXT CBank[4];           // Bank Identifier Code
        T_TXT Country[2];         // Country Code
        T_TXT CTown[2];           // Town Code
        T_TXT CBranch[3];         // Branch Code

    } CBIC;

} SLE0502K, *PSLE0502K;

#define SIZEOF_SLE0502K_MIN (62)
#define SIZEOF_SLE0502K_MAX (62)
#define SLE0502K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBs[%.8s];%sHSeaBs[%.6s];%sNCptePosIptOm[%.16s];%sICptePosIptOm[%.1s];%sCBIC.CBank[%.4s];%sCBIC.Country[%.2s];%sCBIC.CTown[%.2s];%sCBIC.CBranch[%.3s];""%s"
#define SLE0502K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBs, seperator, obj->HSeaBs, seperator, obj->NCptePosIptOm, seperator, &obj->ICptePosIptOm.data, seperator, obj->CBIC.CBank, seperator, obj->CBIC.Country, seperator, obj->CBIC.CTown, seperator, obj->CBIC.CBranch, suffix

typedef struct tagSLE0512K        // Acknowledgement of a client account
{
    T_TXT MembRef[16];            // Internal Member Reference
    T_NUM FuncCode[4];            // Function code
    T_TXT Filler1[344];           // Filler1

} SLE0512K, *PSLE0512K;

#define SIZEOF_SLE0512K_MIN (364)
#define SIZEOF_SLE0512K_MAX (364)
#define SLE0512K_PRINTF_FORMAT "%s""%sMembRef[%.16s];%sFuncCode[%.4s];%sFiller1[%.344s];""%s"
#define SLE0512K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->MembRef, seperator, obj->FuncCode, seperator, obj->Filler1, suffix


#pragma pack()


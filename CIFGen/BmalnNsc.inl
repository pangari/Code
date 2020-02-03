
#pragma pack(1)


typedef struct tagADonCompOmAplKL        // KLSE Clearing data aggregate
{
    T_TXT CIdNgSaiOm[8];                 // ID of the trader that entered the order
    T_TXT YCpteOm;                       // Type of Clearing Account for Member that owns the order
    T_TXT NCptePosIptOm[16];             // Client Account Number
    T_TXT CIdOmNg[8];                    // Trader Order Number (TON)
    T_TXT DHSaiOmAdf[14];                // Order Entry Date and Time
    struct tagCBIC                       // BIC Code of the Member
    {
        T_TXT CBq[4];                    // Bank Code
        T_TXT CPyMbr[2];                 // Country Code
        T_TXT CVilMbr[2];                // Town Code
        T_TXT CSec[3];                   // Branch Code

    } CBIC;
    T_TXT LSaiOm[18];                    // Free text entered with order
    T_TXT CIdMbrDestGupOm[8];            // ID of Clearing System Member that is the beneficiary of a give-up

} ADonCompOmAplKL, *PADonCompOmAplKL;

#define SIZEOF_ADonCompOmAplKL_MIN (84)
#define SIZEOF_ADonCompOmAplKL_MAX (84)
#define ADonCompOmAplKL_PRINTF_FORMAT "%s""%sCIdNgSaiOm[%.8s];%sYCpteOm[%.1s];%sNCptePosIptOm[%.16s];%sCIdOmNg[%.8s];%sDHSaiOmAdf[%.14s];%sCBIC.CBq[%.4s];%sCBIC.CPyMbr[%.2s];%sCBIC.CVilMbr[%.2s];%sCBIC.CSec[%.3s];%sLSaiOm[%.18s];%sCIdMbrDestGupOm[%.8s];""%s"
#define ADonCompOmAplKL_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->CIdNgSaiOm, seperator, &obj->YCpteOm.data, seperator, obj->NCptePosIptOm, seperator, obj->CIdOmNg, seperator, obj->DHSaiOmAdf, seperator, obj->CBIC.CBq, seperator, obj->CBIC.CPyMbr, seperator, obj->CBIC.CVilMbr, seperator, obj->CBIC.CSec, seperator, obj->LSaiOm, seperator, obj->CIdMbrDestGupOm, suffix

typedef struct tagADonProdCpsTran        // Underlying Instrument Aggregate
{
    T_TXT CIsinProdCps[12];              // Underlying Identification Code
    struct tagARaoCps                    // Leg ratio description aggregate
    {
        T_TXT CSignKMuProdCps;           // Component product multiplication coefficient sign
        T_NUM KRaoCpsStg[4];             // Leg number Ratio

    } ARaoCps;
    struct tagPTranProdCps               // Underlying Instruments Trade Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PTranProdCps;
    T_NUM NTranKLProdCps[8];             // Underlying Instruments TRS Trade Number Bis

} ADonProdCpsTran, *PADonProdCpsTran;

#define SIZEOF_ADonProdCpsTran_MIN (35)
#define SIZEOF_ADonProdCpsTran_MAX (35)
#define ADonProdCpsTran_PRINTF_FORMAT "%s""%sCIsinProdCps[%.12s];%sARaoCps.CSignKMuProdCps[%.1s];%sARaoCps.KRaoCpsStg[%.4s];%sPTranProdCps.IFt[%.1s];%sPTranProdCps.QMt[%.9s];%sNTranKLProdCps[%.8s];""%s"
#define ADonProdCpsTran_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->CIsinProdCps, seperator, &obj->ARaoCps.CSignKMuProdCps.data, seperator, obj->ARaoCps.KRaoCpsStg, seperator, &obj->PTranProdCps.IFt.data, seperator, obj->PTranProdCps.QMt, seperator, obj->NTranKLProdCps, suffix

typedef struct tagACpsCreStg             // Leg description aggregate
{
    T_TXT CIsinProdCps[12];              // Underlying Identification Code
    struct tagARaoCps                    // Leg ratio description aggregate
    {
        T_TXT CSignKMuProdCps;           // Component product multiplication coefficient sign
        T_NUM KRaoCpsStg[4];             // Leg number Ratio

    } ARaoCps;

} ACpsCreStg, *PACpsCreStg;

#define SIZEOF_ACpsCreStg_MIN (17)
#define SIZEOF_ACpsCreStg_MAX (17)
#define ACpsCreStg_PRINTF_FORMAT "%s""%sCIsinProdCps[%.12s];%sARaoCps.CSignKMuProdCps[%.1s];%sARaoCps.KRaoCpsStg[%.4s];""%s"
#define ACpsCreStg_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->CIsinProdCps, seperator, &obj->ARaoCps.CSignKMuProdCps.data, seperator, obj->ARaoCps.KRaoCpsStg, suffix

typedef struct tagSLE0001K               // Order Entry
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_NUM DSaiOm[8];                     // Order entry date (in the central trading system)
    T_NUM NSeqOm[6];                     // Order sequence number
    T_TXT CValIsin[12];                  // Instrument Identification
    T_TXT ISensOm;                       // Order Side
    T_NUM QTitTotOm[12];                 // Order Total Quantity
    T_TXT YPLimSaiOm;                    // Type of Limit for an Order
    struct tagPLimSaiOm                  // Original Order Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PLimSaiOm;
    T_TXT YValiOmNSC;                    // Validity Type of an Order
    T_NUM DVALIOM[8];                    // Order validity date
    T_NUM QTitMinOm[12];                 // Order Minimum Quantity
    T_NUM QTitDvlOm[12];                 // Order Disclosed Quantity
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT YOm;                           // Code for the Technical Origin of the Order
    T_NUM ICfmOm;                        // Order Confirmation Flag
    T_NUM IOmTrtPov;                     // Preopening Order Flag
    struct tagPDchOmStop                 // Original Trigger Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PDchOmStop;
    T_NUM TValiOm[6];                    // Order Validity Duration
    T_NUM QRestModAtd[12];               // Expected Remaining Quantity on Order Modification
    struct tagADonCompOmAplKL            // KLSE Clearing data aggregate
    {
        T_TXT CIdNgSaiOm[8];             // ID of the trader that entered the order
        T_TXT YCpteOm;                   // Type of Clearing Account for Member that owns the order
        T_TXT NCptePosIptOm[16];         // Client Account Number
        T_TXT CIdOmNg[8];                // Trader Order Number (TON)
        T_TXT DHSaiOmAdf[14];            // Order Entry Date and Time
        struct tagCBIC                   // BIC Code of the Member
        {
            T_TXT CBq[4];                // Bank Code
            T_TXT CPyMbr[2];             // Country Code
            T_TXT CVilMbr[2];            // Town Code
            T_TXT CSec[3];               // Branch Code

        } CBIC;
        T_TXT LSaiOm[18];                // Free text entered with order
        T_TXT CIdMbrDestGupOm[8];        // ID of Clearing System Member that is the beneficiary of a give-up

    } ADonCompOmAplKL[2];

} SLE0001K, *PSLE0001K;

#define SIZEOF_SLE0001K_MIN (226)
#define SIZEOF_SLE0001K_MAX (310)
#define SLE0001K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSaiOm[%.8s];%sNSeqOm[%.6s];%sCValIsin[%.12s];%sISensOm[%.1s];%sQTitTotOm[%.12s];%sYPLimSaiOm[%.1s];%sPLimSaiOm.IFt[%.1s];%sPLimSaiOm.QMt[%.9s];%sYValiOmNSC[%.1s];%sDVALIOM[%.8s];%sQTitMinOm[%.12s];%sQTitDvlOm[%.12s];%sCIdAdfEmetOm[%.8s];%sYOm[%.1s];%sICfmOm[%.1s];%sIOmTrtPov[%.1s];%sPDchOmStop.IFt[%.1s];%sPDchOmStop.QMt[%.9s];%sTValiOm[%.6s];%sQRestModAtd[%.12s];""%s"
#define SLE0001K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSaiOm, seperator, obj->NSeqOm, seperator, obj->CValIsin, seperator, &obj->ISensOm.data, seperator, obj->QTitTotOm, seperator, &obj->YPLimSaiOm.data, seperator, &obj->PLimSaiOm.IFt.data, seperator, obj->PLimSaiOm.QMt, seperator, &obj->YValiOmNSC.data, seperator, obj->DVALIOM, seperator, obj->QTitMinOm, seperator, obj->QTitDvlOm, seperator, obj->CIdAdfEmetOm, seperator, &obj->YOm.data, seperator, &obj->ICfmOm.data, seperator, &obj->IOmTrtPov.data, seperator, &obj->PDchOmStop.IFt.data, seperator, obj->PDchOmStop.QMt, seperator, obj->TValiOm, seperator, obj->QRestModAtd, suffix

typedef struct tagSLE0002K               // Order Modification
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_NUM DSaiOm[8];                     // Order entry date (in the central trading system)
    T_NUM NSeqOm[6];                     // Order sequence number
    T_TXT CValIsin[12];                  // Instrument Identification
    T_TXT ISensOm;                       // Order Side
    T_NUM QTitTotOm[12];                 // Order Total Quantity
    T_TXT YPLimSaiOm;                    // Type of Limit for an Order
    struct tagPLimSaiOm                  // Original Order Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PLimSaiOm;
    T_TXT YValiOmNSC;                    // Validity Type of an Order
    T_NUM DVALIOM[8];                    // Order validity date
    T_NUM QTitMinOm[12];                 // Order Minimum Quantity
    T_NUM QTitDvlOm[12];                 // Order Disclosed Quantity
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT YOm;                           // Code for the Technical Origin of the Order
    T_NUM ICfmOm;                        // Order Confirmation Flag
    T_NUM IOmTrtPov;                     // Preopening Order Flag
    struct tagPDchOmStop                 // Original Trigger Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PDchOmStop;
    T_NUM TValiOm[6];                    // Order Validity Duration
    T_NUM QRestModAtd[12];               // Expected Remaining Quantity on Order Modification
    struct tagADonCompOmAplKL            // KLSE Clearing data aggregate
    {
        T_TXT CIdNgSaiOm[8];             // ID of the trader that entered the order
        T_TXT YCpteOm;                   // Type of Clearing Account for Member that owns the order
        T_TXT NCptePosIptOm[16];         // Client Account Number
        T_TXT CIdOmNg[8];                // Trader Order Number (TON)
        T_TXT DHSaiOmAdf[14];            // Order Entry Date and Time
        struct tagCBIC                   // BIC Code of the Member
        {
            T_TXT CBq[4];                // Bank Code
            T_TXT CPyMbr[2];             // Country Code
            T_TXT CVilMbr[2];            // Town Code
            T_TXT CSec[3];               // Branch Code

        } CBIC;
        T_TXT LSaiOm[18];                // Free text entered with order
        T_TXT CIdMbrDestGupOm[8];        // ID of Clearing System Member that is the beneficiary of a give-up

    } ADonCompOmAplKL[2];

} SLE0002K, *PSLE0002K;

#define SIZEOF_SLE0002K_MIN (226)
#define SIZEOF_SLE0002K_MAX (310)
#define SLE0002K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSaiOm[%.8s];%sNSeqOm[%.6s];%sCValIsin[%.12s];%sISensOm[%.1s];%sQTitTotOm[%.12s];%sYPLimSaiOm[%.1s];%sPLimSaiOm.IFt[%.1s];%sPLimSaiOm.QMt[%.9s];%sYValiOmNSC[%.1s];%sDVALIOM[%.8s];%sQTitMinOm[%.12s];%sQTitDvlOm[%.12s];%sCIdAdfEmetOm[%.8s];%sYOm[%.1s];%sICfmOm[%.1s];%sIOmTrtPov[%.1s];%sPDchOmStop.IFt[%.1s];%sPDchOmStop.QMt[%.9s];%sTValiOm[%.6s];%sQRestModAtd[%.12s];""%s"
#define SLE0002K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSaiOm, seperator, obj->NSeqOm, seperator, obj->CValIsin, seperator, &obj->ISensOm.data, seperator, obj->QTitTotOm, seperator, &obj->YPLimSaiOm.data, seperator, &obj->PLimSaiOm.IFt.data, seperator, obj->PLimSaiOm.QMt, seperator, &obj->YValiOmNSC.data, seperator, obj->DVALIOM, seperator, obj->QTitMinOm, seperator, obj->QTitDvlOm, seperator, obj->CIdAdfEmetOm, seperator, &obj->YOm.data, seperator, &obj->ICfmOm.data, seperator, &obj->IOmTrtPov.data, seperator, &obj->PDchOmStop.IFt.data, seperator, obj->PDchOmStop.QMt, seperator, obj->TValiOm, seperator, obj->QRestModAtd, suffix

typedef struct tagSLE0003K               // Order cancellation
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_NUM DSaiOm[8];                     // Order entry date (in the central trading system)
    T_NUM NSeqOm[6];                     // Order sequence number
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT CValIsin[12];                  // Instrument Identification
    T_TXT ISensOm;                       // Side of order

} SLE0003K, *PSLE0003K;

#define SIZEOF_SLE0003K_MIN (55)
#define SIZEOF_SLE0003K_MAX (55)
#define SLE0003K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSaiOm[%.8s];%sNSeqOm[%.6s];%sCIdAdfEmetOm[%.8s];%sCValIsin[%.12s];%sISensOm[%.1s];""%s"
#define SLE0003K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSaiOm, seperator, obj->NSeqOm, seperator, obj->CIdAdfEmetOm, seperator, obj->CValIsin, seperator, &obj->ISensOm.data, suffix

typedef struct tagSLE0065K               // Global order cancellation
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT CIdGrc[2];                     // Instrument group identification
    T_TXT CValIsin[12];                  // Instrument Identification
    T_TXT ISensOm;                       // Side of order
    T_TXT YCpteOm;                       // Type of Clearing Account for Member that owns the order
    T_TXT YOm;                           // Code for the Technical Origin of the Order
    T_TXT CIdItfOm[11];                  // ID of the member's order entry server

} SLE0065K, *PSLE0065K;

#define SIZEOF_SLE0065K_MIN (56)
#define SIZEOF_SLE0065K_MAX (56)
#define SLE0065K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sCIdAdfEmetOm[%.8s];%sCIdGrc[%.2s];%sCValIsin[%.12s];%sISensOm[%.1s];%sYCpteOm[%.1s];%sYOm[%.1s];%sCIdItfOm[%.11s];""%s"
#define SLE0065K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->CIdAdfEmetOm, seperator, obj->CIdGrc, seperator, obj->CValIsin, seperator, &obj->ISensOm.data, seperator, &obj->YCpteOm.data, seperator, &obj->YOm.data, seperator, obj->CIdItfOm, suffix

typedef struct tagSLE0080K               // Command to enter a request for quote
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT CValIsin[12];                  // Instrument Identification
    T_NUM QTitDemP[12];                  // Quantity of the Request For Quote
    struct tagPTITACPUP                  // Price of the Request For Quote
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PTITACPUP;
    T_TXT INonDifCAdhOn;                 // Member ID Broadcast Flag

} SLE0080K, *PSLE0080K;

#define SIZEOF_SLE0080K_MIN (55)
#define SIZEOF_SLE0080K_MAX (55)
#define SLE0080K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sCValIsin[%.12s];%sQTitDemP[%.12s];%sPTITACPUP.IFt[%.1s];%sPTITACPUP.QMt[%.9s];%sINonDifCAdhOn[%.1s];""%s"
#define SLE0080K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->CValIsin, seperator, obj->QTitDemP, seperator, &obj->PTITACPUP.IFt.data, seperator, obj->PTITACPUP.QMt, seperator, &obj->INonDifCAdhOn.data, suffix

typedef struct tagSLE0100K               // Trade cancellation notice
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT DSeaBsEven[8];                 // Date of trading session for event
    T_TXT CValIsin[12];                  // Instrument Identification
    T_NUM DSaiOm[8];                     // Order entry date (in the central trading system)
    T_NUM NSeqOm[6];                     // Order sequence number
    struct tagPTran                      // Trade Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PTran;
    T_NUM QTitTran[12];                  // Traded quantity
    T_NUM HTRAN[6];                      // Trade Time
    T_NUM NMsgRepoN[6];                  // Response Sequence Number
    T_TXT HMsgRepoN[6];                  // Response Sequence Number Time
    T_TXT CIdAdfCie[8];                  // Counterpart Member Identification
    T_NUM NTran[7];                      // Trade number
    T_TXT ISensOm;                       // Side of order
    T_NUM NTranKL[8];                    // TRS Trade Number Bis

} SLE0100K, *PSLE0100K;

#define SIZEOF_SLE0100K_MIN (118)
#define SIZEOF_SLE0100K_MAX (118)
#define SLE0100K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBsEven[%.8s];%sCValIsin[%.12s];%sDSaiOm[%.8s];%sNSeqOm[%.6s];%sPTran.IFt[%.1s];%sPTran.QMt[%.9s];%sQTitTran[%.12s];%sHTRAN[%.6s];%sNMsgRepoN[%.6s];%sHMsgRepoN[%.6s];%sCIdAdfCie[%.8s];%sNTran[%.7s];%sISensOm[%.1s];%sNTranKL[%.8s];""%s"
#define SLE0100K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBsEven, seperator, obj->CValIsin, seperator, obj->DSaiOm, seperator, obj->NSeqOm, seperator, &obj->PTran.IFt.data, seperator, obj->PTran.QMt, seperator, obj->QTitTran, seperator, obj->HTRAN, seperator, obj->NMsgRepoN, seperator, obj->HMsgRepoN, seperator, obj->CIdAdfCie, seperator, obj->NTran, seperator, &obj->ISensOm.data, seperator, obj->NTranKL, suffix

typedef struct tagSLE0101K               // Group state change notice
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT CIdGrc[2];                     // Instrument group identification
    T_TXT CEtaGrc;                       // Instrument group state
    T_TXT DSeaBs[8];                     // Trading date
    T_TXT HSeaBs[6];                     // Trading time
    T_TXT DHEmisMsgSpi[14];              // Transmission Date Time
    T_TXT IClGrc;                        // Closing Indicator

} SLE0101K, *PSLE0101K;

#define SIZEOF_SLE0101K_MIN (52)
#define SIZEOF_SLE0101K_MAX (52)
#define SLE0101K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sCIdGrc[%.2s];%sCEtaGrc[%.1s];%sDSeaBs[%.8s];%sHSeaBs[%.6s];%sDHEmisMsgSpi[%.14s];%sIClGrc[%.1s];""%s"
#define SLE0101K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->CIdGrc, seperator, &obj->CEtaGrc.data, seperator, obj->DSeaBs, seperator, obj->HSeaBs, seperator, obj->DHEmisMsgSpi, seperator, &obj->IClGrc.data, suffix

typedef struct tagSLE0103K               // Trade creation notice
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT DSeaBs[8];                     // Trading date
    T_TXT CValIsin[12];                  // Instrument Identification
    struct tagPTran                      // Trade Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PTran;
    T_NUM QTitTran[12];                  // Traded quantity
    T_TXT DHTran[14];                    // Trade Date and Time
    T_TXT ISensOm;                       // Side of order
    T_TXT CIdAdfCie[8];                  // Counterpart Member Identification
    T_TXT YOm;                           // Code for the Technical Origin of the Order
    T_NUM NMsgRepoN[6];                  // Response Sequence Number
    T_TXT HMsgRepoN[6];                  // Response Sequence Number Time
    T_NUM NTran[7];                      // Trade number
    T_TXT YOmOrgTran;                    // Type of orders at the origin of a trade
    T_NUM NTranKL[8];                    // TRS Trade Number Bis
    struct tagADonCompOmAplKL            // KLSE Clearing data aggregate
    {
        T_TXT CIdNgSaiOm[8];             // ID of the trader that entered the order
        T_TXT YCpteOm;                   // Type of Clearing Account for Member that owns the order
        T_TXT NCptePosIptOm[16];         // Client Account Number
        T_TXT CIdOmNg[8];                // Trader Order Number (TON)
        T_TXT DHSaiOmAdf[14];            // Order Entry Date and Time
        struct tagCBIC                   // BIC Code of the Member
        {
            T_TXT CBq[4];                // Bank Code
            T_TXT CPyMbr[2];             // Country Code
            T_TXT CVilMbr[2];            // Town Code
            T_TXT CSec[3];               // Branch Code

        } CBIC;
        T_TXT LSaiOm[18];                // Free text entered with order
        T_TXT CIdMbrDestGupOm[8];        // ID of Clearing System Member that is the beneficiary of a give-up

    } ADonCompOmAplKL;
    T_NUM ZProdCpsTran[2];               // Number of Underlying Instruments
    struct tagADonProdCpsTran            // Underlying Instrument Aggregate
    {
        T_TXT CIsinProdCps[12];          // Underlying Identification Code
        struct tagARaoCps                // Leg ratio description aggregate
        {
            T_TXT CSignKMuProdCps;       // Component product multiplication coefficient sign
            T_NUM KRaoCpsStg[4];         // Leg number Ratio

        } ARaoCps;
        struct tagPTranProdCps           // Underlying Instruments Trade Price
        {
            T_TXT IFt;                   // Decimal point locator (AtosEuronext)
            T_NUM QMt[9];                // Amount

        } PTranProdCps;
        T_NUM NTranKLProdCps[8];         // Underlying Instruments TRS Trade Number Bis

    } ADonProdCpsTran[40];

} SLE0103K, *PSLE0103K;

#define SIZEOF_SLE0103K_MIN (200)
#define SIZEOF_SLE0103K_MAX (1600)
#define SLE0103K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBs[%.8s];%sCValIsin[%.12s];%sPTran.IFt[%.1s];%sPTran.QMt[%.9s];%sQTitTran[%.12s];%sDHTran[%.14s];%sISensOm[%.1s];%sCIdAdfCie[%.8s];%sYOm[%.1s];%sNMsgRepoN[%.6s];%sHMsgRepoN[%.6s];%sNTran[%.7s];%sYOmOrgTran[%.1s];%sNTranKL[%.8s];%sADonCompOmAplKL.CIdNgSaiOm[%.8s];%sADonCompOmAplKL.YCpteOm[%.1s];%sADonCompOmAplKL.NCptePosIptOm[%.16s];%sADonCompOmAplKL.CIdOmNg[%.8s];%sADonCompOmAplKL.DHSaiOmAdf[%.14s];%sADonCompOmAplKL.CBIC.CBq[%.4s];%sADonCompOmAplKL.CBIC.CPyMbr[%.2s];%sADonCompOmAplKL.CBIC.CVilMbr[%.2s];%sADonCompOmAplKL.CBIC.CSec[%.3s];%sADonCompOmAplKL.LSaiOm[%.18s];%sADonCompOmAplKL.CIdMbrDestGupOm[%.8s];%sZProdCpsTran[%.2s];""%s"
#define SLE0103K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBs, seperator, obj->CValIsin, seperator, &obj->PTran.IFt.data, seperator, obj->PTran.QMt, seperator, obj->QTitTran, seperator, obj->DHTran, seperator, &obj->ISensOm.data, seperator, obj->CIdAdfCie, seperator, &obj->YOm.data, seperator, obj->NMsgRepoN, seperator, obj->HMsgRepoN, seperator, obj->NTran, seperator, &obj->YOmOrgTran.data, seperator, obj->NTranKL, seperator, obj->ADonCompOmAplKL.CIdNgSaiOm, seperator, &obj->ADonCompOmAplKL.YCpteOm.data, seperator, obj->ADonCompOmAplKL.NCptePosIptOm, seperator, obj->ADonCompOmAplKL.CIdOmNg, seperator, obj->ADonCompOmAplKL.DHSaiOmAdf, seperator, obj->ADonCompOmAplKL.CBIC.CBq, seperator, obj->ADonCompOmAplKL.CBIC.CPyMbr, seperator, obj->ADonCompOmAplKL.CBIC.CVilMbr, seperator, obj->ADonCompOmAplKL.CBIC.CSec, seperator, obj->ADonCompOmAplKL.LSaiOm, seperator, obj->ADonCompOmAplKL.CIdMbrDestGupOm, seperator, obj->ZProdCpsTran, suffix

typedef struct tagSLE0105K               // Execution notice
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_NUM DSaiOm[8];                     // Order entry date (in the central trading system)
    T_NUM NSeqOm[6];                     // Order sequence number
    T_TXT CValIsin[12];                  // Instrument Identification
    T_TXT CIdGrc[2];                     // Instrument group identification
    T_TXT ISensOm;                       // Side of order
    T_NUM QTitTran[12];                  // Traded quantity
    struct tagPTran                      // Trade Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PTran;
    T_TXT IPrsQTitRest;                  // Indicator for Remaining Quantity on an Order
    T_NUM QTitRestOm[12];                // Remaining Quantity of the Order
    T_TXT CIdAdfCie[8];                  // Counterpart Member Identification
    T_TXT YOm;                           // Code for the Technical Origin of the Order
    T_NUM NMsgRepoN[6];                  // Response Sequence Number
    T_TXT DMsgRepoN[8];                  // Response Sequence Number Date
    T_TXT HMsgRepoN[6];                  // Response Sequence Number Time
    T_NUM NTran[7];                      // Trade number
    T_TXT DTran[8];                      // Trade Date
    T_TXT YPLimSaiOm;                    // Type of Limit for an Order
    T_TXT YValiOmNSC;                    // Validity Type of an Order
    T_TXT CGdSVal;                       // Code of the instrument category
    T_NUM NTranKL[8];                    // TRS Trade Number Bis
    struct tagADonCompOmAplKL            // KLSE Clearing data aggregate
    {
        T_TXT CIdNgSaiOm[8];             // ID of the trader that entered the order
        T_TXT YCpteOm;                   // Type of Clearing Account for Member that owns the order
        T_TXT NCptePosIptOm[16];         // Client Account Number
        T_TXT CIdOmNg[8];                // Trader Order Number (TON)
        T_TXT DHSaiOmAdf[14];            // Order Entry Date and Time
        struct tagCBIC                   // BIC Code of the Member
        {
            T_TXT CBq[4];                // Bank Code
            T_TXT CPyMbr[2];             // Country Code
            T_TXT CVilMbr[2];            // Town Code
            T_TXT CSec[3];               // Branch Code

        } CBIC;
        T_TXT LSaiOm[18];                // Free text entered with order
        T_TXT CIdMbrDestGupOm[8];        // ID of Clearing System Member that is the beneficiary of a give-up

    } ADonCompOmAplKL;
    T_NUM ZProdCpsTran[2];               // Number of Underlying Instruments
    struct tagADonProdCpsTran            // Underlying Instrument Aggregate
    {
        T_TXT CIsinProdCps[12];          // Underlying Identification Code
        struct tagARaoCps                // Leg ratio description aggregate
        {
            T_TXT CSignKMuProdCps;       // Component product multiplication coefficient sign
            T_NUM KRaoCpsStg[4];         // Leg number Ratio

        } ARaoCps;
        struct tagPTranProdCps           // Underlying Instruments Trade Price
        {
            T_TXT IFt;                   // Decimal point locator (AtosEuronext)
            T_NUM QMt[9];                // Amount

        } PTranProdCps;
        T_NUM NTranKLProdCps[8];         // Underlying Instruments TRS Trade Number Bis

    } ADonProdCpsTran[40];

} SLE0105K, *PSLE0105K;

#define SIZEOF_SLE0105K_MIN (225)
#define SIZEOF_SLE0105K_MAX (1625)
#define SLE0105K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSaiOm[%.8s];%sNSeqOm[%.6s];%sCValIsin[%.12s];%sCIdGrc[%.2s];%sISensOm[%.1s];%sQTitTran[%.12s];%sPTran.IFt[%.1s];%sPTran.QMt[%.9s];%sIPrsQTitRest[%.1s];%sQTitRestOm[%.12s];%sCIdAdfCie[%.8s];%sYOm[%.1s];%sNMsgRepoN[%.6s];%sDMsgRepoN[%.8s];%sHMsgRepoN[%.6s];%sNTran[%.7s];%sDTran[%.8s];%sYPLimSaiOm[%.1s];%sYValiOmNSC[%.1s];%sCGdSVal[%.1s];%sNTranKL[%.8s];%sADonCompOmAplKL.CIdNgSaiOm[%.8s];%sADonCompOmAplKL.YCpteOm[%.1s];%sADonCompOmAplKL.NCptePosIptOm[%.16s];%sADonCompOmAplKL.CIdOmNg[%.8s];%sADonCompOmAplKL.DHSaiOmAdf[%.14s];%sADonCompOmAplKL.CBIC.CBq[%.4s];%sADonCompOmAplKL.CBIC.CPyMbr[%.2s];%sADonCompOmAplKL.CBIC.CVilMbr[%.2s];%sADonCompOmAplKL.CBIC.CSec[%.3s];%sADonCompOmAplKL.LSaiOm[%.18s];%sADonCompOmAplKL.CIdMbrDestGupOm[%.8s];%sZProdCpsTran[%.2s];""%s"
#define SLE0105K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSaiOm, seperator, obj->NSeqOm, seperator, obj->CValIsin, seperator, obj->CIdGrc, seperator, &obj->ISensOm.data, seperator, obj->QTitTran, seperator, &obj->PTran.IFt.data, seperator, obj->PTran.QMt, seperator, &obj->IPrsQTitRest.data, seperator, obj->QTitRestOm, seperator, obj->CIdAdfCie, seperator, &obj->YOm.data, seperator, obj->NMsgRepoN, seperator, obj->DMsgRepoN, seperator, obj->HMsgRepoN, seperator, obj->NTran, seperator, obj->DTran, seperator, &obj->YPLimSaiOm.data, seperator, &obj->YValiOmNSC.data, seperator, &obj->CGdSVal.data, seperator, obj->NTranKL, seperator, obj->ADonCompOmAplKL.CIdNgSaiOm, seperator, &obj->ADonCompOmAplKL.YCpteOm.data, seperator, obj->ADonCompOmAplKL.NCptePosIptOm, seperator, obj->ADonCompOmAplKL.CIdOmNg, seperator, obj->ADonCompOmAplKL.DHSaiOmAdf, seperator, obj->ADonCompOmAplKL.CBIC.CBq, seperator, obj->ADonCompOmAplKL.CBIC.CPyMbr, seperator, obj->ADonCompOmAplKL.CBIC.CVilMbr, seperator, obj->ADonCompOmAplKL.CBIC.CSec, seperator, obj->ADonCompOmAplKL.LSaiOm, seperator, obj->ADonCompOmAplKL.CIdMbrDestGupOm, seperator, obj->ZProdCpsTran, suffix

typedef struct tagSLE0106K               // Instrument state change notice
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT DSeaBs[8];                     // Trading date
    T_TXT HSeaBs[6];                     // Trading time
    T_TXT CValIsin[12];                  // Instrument Identification
    T_TXT CEtaVal[2];                    // Code indicating the state of the instrument in NSC
    T_TXT CActModEtaVal;                 // Action code for the instrument state change
    T_TXT HOvPgmVal[6];                  // Programmed opening time for instrument
    T_TXT DHEmisMsgSpi[14];              // Transmission Date Time

} SLE0106K, *PSLE0106K;

#define SIZEOF_SLE0106K_MIN (69)
#define SIZEOF_SLE0106K_MAX (69)
#define SLE0106K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBs[%.8s];%sHSeaBs[%.6s];%sCValIsin[%.12s];%sCEtaVal[%.2s];%sCActModEtaVal[%.1s];%sHOvPgmVal[%.6s];%sDHEmisMsgSpi[%.14s];""%s"
#define SLE0106K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBs, seperator, obj->HSeaBs, seperator, obj->CValIsin, seperator, obj->CEtaVal, seperator, &obj->CActModEtaVal.data, seperator, obj->HOvPgmVal, seperator, obj->DHEmisMsgSpi, suffix

typedef struct tagSLE0098K               // User Defined Strategy creation
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT CIdNgSaiOm[8];                 // ID of the trader that entered the order
    T_TXT YQStg[3];                      // Strategy Quantity Type
    T_TXT YStg[2];                       // Strategy Type
    struct tagPCpsDrvObl                 // Future-type leg price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PCpsDrvObl;
    struct tagXDtaStg                    // Delta Strategy percentage
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[18];                   // Amount

    } XDtaStg;
    T_NUM ZCpsStg[2];                    // Strategy Leg Number
    struct tagACpsCreStg                 // Leg description aggregate
    {
        T_TXT CIsinProdCps[12];          // Underlying Identification Code
        struct tagARaoCps                // Leg ratio description aggregate
        {
            T_TXT CSignKMuProdCps;       // Component product multiplication coefficient sign
            T_NUM KRaoCpsStg[4];         // Leg number Ratio

        } ARaoCps;

    } ACpsCreStg[40];
    T_TXT IInOmStg;                      // Including Order Indicator
    struct tagAInOmStg                   // Order entry aggregate
    {
        T_TXT YPLimSaiOm;                // Type of Limit for an Order
        struct tagPLimSaiOm              // Original Order Price
        {
            T_TXT IFt;                   // Decimal point locator (AtosEuronext)
            T_NUM QMt[9];                // Amount

        } PLimSaiOm;
        T_TXT YValiOmNSC;                // Validity Type of an Order
        T_NUM DVALIOM[8];                // Order validity date
        T_NUM TValiOm[6];                // Order Validity Duration
        T_NUM QTitDvlOm[12];             // Order Disclosed Quantity
        T_NUM QTitTotOm[12];             // Order Total Quantity
        struct tagPDchOmStop             // Original Trigger Price
        {
            T_TXT IFt;                   // Decimal point locator (AtosEuronext)
            T_NUM QMt[9];                // Amount

        } PDchOmStop;
        T_TXT ISensOm;                   // Order Side
        struct tagADonCompOmAplKL        // KLSE Clearing data aggregate
        {
            T_TXT CIdNgSaiOm[8];         // ID of the trader that entered the order
            T_TXT YCpteOm;               // Type of Clearing Account for Member that owns the order
            T_TXT NCptePosIptOm[16];     // Client Account Number
            T_TXT CIdOmNg[8];            // Trader Order Number (TON)
            T_TXT DHSaiOmAdf[14];        // Order Entry Date and Time
            struct tagCBIC               // BIC Code of the Member
            {
                T_TXT CBq[4];            // Bank Code
                T_TXT CPyMbr[2];         // Country Code
                T_TXT CVilMbr[2];        // Town Code
                T_TXT CSec[3];           // Branch Code

            } CBIC;
            T_TXT LSaiOm[18];            // Free text entered with order
            T_TXT CIdMbrDestGupOm[8];    // ID of Clearing System Member that is the beneficiary of a give-up

        } ADonCompOmAplKL;

    } AInOmStg;

} SLE0098K, *PSLE0098K;

#define SIZEOF_SLE0098K_MIN (898)
#define SIZEOF_SLE0098K_MAX (898)
#define SLE0098K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sCIdAdfEmetOm[%.8s];%sCIdNgSaiOm[%.8s];%sYQStg[%.3s];%sYStg[%.2s];%sPCpsDrvObl.IFt[%.1s];%sPCpsDrvObl.QMt[%.9s];%sXDtaStg.IFt[%.1s];%sXDtaStg.QMt[%.18s];%sZCpsStg[%.2s];%sIInOmStg[%.1s];%sAInOmStg.YPLimSaiOm[%.1s];%sAInOmStg.PLimSaiOm.IFt[%.1s];%sAInOmStg.PLimSaiOm.QMt[%.9s];%sAInOmStg.YValiOmNSC[%.1s];%sAInOmStg.DVALIOM[%.8s];%sAInOmStg.TValiOm[%.6s];%sAInOmStg.QTitDvlOm[%.12s];%sAInOmStg.QTitTotOm[%.12s];%sAInOmStg.PDchOmStop.IFt[%.1s];%sAInOmStg.PDchOmStop.QMt[%.9s];%sAInOmStg.ISensOm[%.1s];%sAInOmStg.ADonCompOmAplKL.CIdNgSaiOm[%.8s];%sAInOmStg.ADonCompOmAplKL.YCpteOm[%.1s];%sAInOmStg.ADonCompOmAplKL.NCptePosIptOm[%.16s];%sAInOmStg.ADonCompOmAplKL.CIdOmNg[%.8s];%sAInOmStg.ADonCompOmAplKL.DHSaiOmAdf[%.14s];%sAInOmStg.ADonCompOmAplKL.CBIC.CBq[%.4s];%sAInOmStg.ADonCompOmAplKL.CBIC.CPyMbr[%.2s];%sAInOmStg.ADonCompOmAplKL.CBIC.CVilMbr[%.2s];%sAInOmStg.ADonCompOmAplKL.CBIC.CSec[%.3s];%sAInOmStg.ADonCompOmAplKL.LSaiOm[%.18s];%sAInOmStg.ADonCompOmAplKL.CIdMbrDestGupOm[%.8s];""%s"
#define SLE0098K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->CIdAdfEmetOm, seperator, obj->CIdNgSaiOm, seperator, obj->YQStg, seperator, obj->YStg, seperator, &obj->PCpsDrvObl.IFt.data, seperator, obj->PCpsDrvObl.QMt, seperator, &obj->XDtaStg.IFt.data, seperator, obj->XDtaStg.QMt, seperator, obj->ZCpsStg, seperator, &obj->IInOmStg.data, seperator, &obj->AInOmStg.YPLimSaiOm.data, seperator, &obj->AInOmStg.PLimSaiOm.IFt.data, seperator, obj->AInOmStg.PLimSaiOm.QMt, seperator, &obj->AInOmStg.YValiOmNSC.data, seperator, obj->AInOmStg.DVALIOM, seperator, obj->AInOmStg.TValiOm, seperator, obj->AInOmStg.QTitDvlOm, seperator, obj->AInOmStg.QTitTotOm, seperator, &obj->AInOmStg.PDchOmStop.IFt.data, seperator, obj->AInOmStg.PDchOmStop.QMt, seperator, &obj->AInOmStg.ISensOm.data, seperator, obj->AInOmStg.ADonCompOmAplKL.CIdNgSaiOm, seperator, &obj->AInOmStg.ADonCompOmAplKL.YCpteOm.data, seperator, obj->AInOmStg.ADonCompOmAplKL.NCptePosIptOm, seperator, obj->AInOmStg.ADonCompOmAplKL.CIdOmNg, seperator, obj->AInOmStg.ADonCompOmAplKL.DHSaiOmAdf, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CBq, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CPyMbr, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CVilMbr, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CSec, seperator, obj->AInOmStg.ADonCompOmAplKL.LSaiOm, seperator, obj->AInOmStg.ADonCompOmAplKL.CIdMbrDestGupOm, suffix

typedef struct tagSLE0138K               // Order elimination
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_NUM DSaiOm[8];                     // Order entry date (in the central trading system)
    T_NUM NSeqOm[6];                     // Order sequence number
    T_TXT YMajOmNSC;                     // Order Status
    T_TXT CValIsin[12];                  // Instrument Identification
    T_NUM QTitRestOm[12];                // Remaining Quantity of the Order
    struct tagPLimSaiOm                  // Original Order Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PLimSaiOm;
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT YValiOmNSC;                    // Validity Type of an Order
    T_TXT Filler1;                       // Filler1
    T_NUM DVALIOM[8];                    // Order validity date
    T_TXT ISensOm;                       // Side of order
    T_TXT YPLimSaiOm;                    // Type of Limit for an Order
    T_NUM NMsgRepoN[6];                  // Response Sequence Number
    T_TXT HMsgRepoN[6];                  // Response Sequence Number Time
    T_TXT Filler2[3];                    // Filler2
    struct tagADonCompOmAplKL            // KLSE Clearing data aggregate
    {
        T_TXT CIdNgSaiOm[8];             // ID of the trader that entered the order
        T_TXT YCpteOm;                   // Type of Clearing Account for Member that owns the order
        T_TXT NCptePosIptOm[16];         // Client Account Number
        T_TXT CIdOmNg[8];                // Trader Order Number (TON)
        T_TXT DHSaiOmAdf[14];            // Order Entry Date and Time
        struct tagCBIC                   // BIC Code of the Member
        {
            T_TXT CBq[4];                // Bank Code
            T_TXT CPyMbr[2];             // Country Code
            T_TXT CVilMbr[2];            // Town Code
            T_TXT CSec[3];               // Branch Code

        } CBIC;
        T_TXT LSaiOm[18];                // Free text entered with order
        T_TXT CIdMbrDestGupOm[8];        // ID of Clearing System Member that is the beneficiary of a give-up

    } ADonCompOmAplKL;

} SLE0138K, *PSLE0138K;

#define SIZEOF_SLE0138K_MIN (188)
#define SIZEOF_SLE0138K_MAX (188)
#define SLE0138K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSaiOm[%.8s];%sNSeqOm[%.6s];%sYMajOmNSC[%.1s];%sCValIsin[%.12s];%sQTitRestOm[%.12s];%sPLimSaiOm.IFt[%.1s];%sPLimSaiOm.QMt[%.9s];%sCIdAdfEmetOm[%.8s];%sYValiOmNSC[%.1s];%sFiller1[%.1s];%sDVALIOM[%.8s];%sISensOm[%.1s];%sYPLimSaiOm[%.1s];%sNMsgRepoN[%.6s];%sHMsgRepoN[%.6s];%sFiller2[%.3s];%sADonCompOmAplKL.CIdNgSaiOm[%.8s];%sADonCompOmAplKL.YCpteOm[%.1s];%sADonCompOmAplKL.NCptePosIptOm[%.16s];%sADonCompOmAplKL.CIdOmNg[%.8s];%sADonCompOmAplKL.DHSaiOmAdf[%.14s];%sADonCompOmAplKL.CBIC.CBq[%.4s];%sADonCompOmAplKL.CBIC.CPyMbr[%.2s];%sADonCompOmAplKL.CBIC.CVilMbr[%.2s];%sADonCompOmAplKL.CBIC.CSec[%.3s];%sADonCompOmAplKL.LSaiOm[%.18s];%sADonCompOmAplKL.CIdMbrDestGupOm[%.8s];""%s"
#define SLE0138K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSaiOm, seperator, obj->NSeqOm, seperator, &obj->YMajOmNSC.data, seperator, obj->CValIsin, seperator, obj->QTitRestOm, seperator, &obj->PLimSaiOm.IFt.data, seperator, obj->PLimSaiOm.QMt, seperator, obj->CIdAdfEmetOm, seperator, &obj->YValiOmNSC.data, seperator, &obj->Filler1.data, seperator, obj->DVALIOM, seperator, &obj->ISensOm.data, seperator, &obj->YPLimSaiOm.data, seperator, obj->NMsgRepoN, seperator, obj->HMsgRepoN, seperator, obj->Filler2, seperator, obj->ADonCompOmAplKL.CIdNgSaiOm, seperator, &obj->ADonCompOmAplKL.YCpteOm.data, seperator, obj->ADonCompOmAplKL.NCptePosIptOm, seperator, obj->ADonCompOmAplKL.CIdOmNg, seperator, obj->ADonCompOmAplKL.DHSaiOmAdf, seperator, obj->ADonCompOmAplKL.CBIC.CBq, seperator, obj->ADonCompOmAplKL.CBIC.CPyMbr, seperator, obj->ADonCompOmAplKL.CBIC.CVilMbr, seperator, obj->ADonCompOmAplKL.CBIC.CSec, seperator, obj->ADonCompOmAplKL.LSaiOm, seperator, obj->ADonCompOmAplKL.CIdMbrDestGupOm, suffix

typedef struct tagSLE0144K               // Error message after an external trader or Market Control request
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_NUM CFonOrg[4];                    // Original NSC function code
    T_NUM CErNSC[6];                     // Error Code
    T_TXT LErNSC[71];                    // Error Text

} SLE0144K, *PSLE0144K;

#define SIZEOF_SLE0144K_MIN (101)
#define SIZEOF_SLE0144K_MAX (101)
#define SLE0144K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sCFonOrg[%.4s];%sCErNSC[%.6s];%sLErNSC[%.71s];""%s"
#define SLE0144K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->CFonOrg, seperator, obj->CErNSC, seperator, obj->LErNSC, suffix

typedef struct tagSLE0172K               // Confirmation of order creation, modification or cancellation
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_NUM DSaiOm[8];                     // Order entry date (in the central trading system)
    T_NUM NSeqOm[6];                     // Order sequence number
    T_TXT YMajOmNSC;                     // Order Status
    T_TXT CValIsin[12];                  // Instrument Identification
    T_NUM QTitTotOm[12];                 // Order Total Quantity
    T_TXT ISensOm;                       // Order Side
    struct tagPLimSaiOm                  // Original Order Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PLimSaiOm;
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_NUM NMsgRepoN[6];                  // Response Sequence Number
    T_TXT HMsgRepoN[6];                  // Response Sequence Number Time
    T_TXT YPLimSaiOm;                    // Type of Limit for an Order
    T_NUM QTitXteIntrOm[12];             // Matched Quantity at order entry
    T_NUM CFonOrg[4];                    // Original NSC function code
    T_TXT DSaiOmIni[8];                  // Original Order Date
    T_NUM NSeqOmIni[6];                  // Original order sequence number
    T_TXT YValiOmNSC;                    // Validity Type of an Order
    T_NUM DVALIOM[8];                    // Order validity date
    T_NUM QTitMinOm[12];                 // Order Minimum Quantity
    T_NUM QTitDvlOm[12];                 // Order Disclosed Quantity
    T_TXT YOm;                           // Code for the Technical Origin of the Order
    T_NUM ICfmOm;                        // Order Confirmation Flag
    T_NUM QTitRestOmIni[12];             // Original order remaining quantity
    struct tagPDchOmStop                 // Original Trigger Price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PDchOmStop;
    T_NUM TValiOm[6];                    // Order Validity Duration
    struct tagADonCompOmAplKL            // KLSE Clearing data aggregate
    {
        T_TXT CIdNgSaiOm[8];             // ID of the trader that entered the order
        T_TXT YCpteOm;                   // Type of Clearing Account for Member that owns the order
        T_TXT NCptePosIptOm[16];         // Client Account Number
        T_TXT CIdOmNg[8];                // Trader Order Number (TON)
        T_TXT DHSaiOmAdf[14];            // Order Entry Date and Time
        struct tagCBIC                   // BIC Code of the Member
        {
            T_TXT CBq[4];                // Bank Code
            T_TXT CPyMbr[2];             // Country Code
            T_TXT CVilMbr[2];            // Town Code
            T_TXT CSec[3];               // Branch Code

        } CBIC;
        T_TXT LSaiOm[18];                // Free text entered with order
        T_TXT CIdMbrDestGupOm[8];        // ID of Clearing System Member that is the beneficiary of a give-up

    } ADonCompOmAplKL;
    T_TXT DHPriOm[20];                   // Order priority date time

} SLE0172K, *PSLE0172K;

#define SIZEOF_SLE0172K_MIN (288)
#define SIZEOF_SLE0172K_MAX (288)
#define SLE0172K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSaiOm[%.8s];%sNSeqOm[%.6s];%sYMajOmNSC[%.1s];%sCValIsin[%.12s];%sQTitTotOm[%.12s];%sISensOm[%.1s];%sPLimSaiOm.IFt[%.1s];%sPLimSaiOm.QMt[%.9s];%sCIdAdfEmetOm[%.8s];%sNMsgRepoN[%.6s];%sHMsgRepoN[%.6s];%sYPLimSaiOm[%.1s];%sQTitXteIntrOm[%.12s];%sCFonOrg[%.4s];%sDSaiOmIni[%.8s];%sNSeqOmIni[%.6s];%sYValiOmNSC[%.1s];%sDVALIOM[%.8s];%sQTitMinOm[%.12s];%sQTitDvlOm[%.12s];%sYOm[%.1s];%sICfmOm[%.1s];%sQTitRestOmIni[%.12s];%sPDchOmStop.IFt[%.1s];%sPDchOmStop.QMt[%.9s];%sTValiOm[%.6s];%sADonCompOmAplKL.CIdNgSaiOm[%.8s];%sADonCompOmAplKL.YCpteOm[%.1s];%sADonCompOmAplKL.NCptePosIptOm[%.16s];%sADonCompOmAplKL.CIdOmNg[%.8s];%sADonCompOmAplKL.DHSaiOmAdf[%.14s];%sADonCompOmAplKL.CBIC.CBq[%.4s];%sADonCompOmAplKL.CBIC.CPyMbr[%.2s];%sADonCompOmAplKL.CBIC.CVilMbr[%.2s];%sADonCompOmAplKL.CBIC.CSec[%.3s];%sADonCompOmAplKL.LSaiOm[%.18s];%sADonCompOmAplKL.CIdMbrDestGupOm[%.8s];%sDHPriOm[%.20s];""%s"
#define SLE0172K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSaiOm, seperator, obj->NSeqOm, seperator, &obj->YMajOmNSC.data, seperator, obj->CValIsin, seperator, obj->QTitTotOm, seperator, &obj->ISensOm.data, seperator, &obj->PLimSaiOm.IFt.data, seperator, obj->PLimSaiOm.QMt, seperator, obj->CIdAdfEmetOm, seperator, obj->NMsgRepoN, seperator, obj->HMsgRepoN, seperator, &obj->YPLimSaiOm.data, seperator, obj->QTitXteIntrOm, seperator, obj->CFonOrg, seperator, obj->DSaiOmIni, seperator, obj->NSeqOmIni, seperator, &obj->YValiOmNSC.data, seperator, obj->DVALIOM, seperator, obj->QTitMinOm, seperator, obj->QTitDvlOm, seperator, &obj->YOm.data, seperator, &obj->ICfmOm.data, seperator, obj->QTitRestOmIni, seperator, &obj->PDchOmStop.IFt.data, seperator, obj->PDchOmStop.QMt, seperator, obj->TValiOm, seperator, obj->ADonCompOmAplKL.CIdNgSaiOm, seperator, &obj->ADonCompOmAplKL.YCpteOm.data, seperator, obj->ADonCompOmAplKL.NCptePosIptOm, seperator, obj->ADonCompOmAplKL.CIdOmNg, seperator, obj->ADonCompOmAplKL.DHSaiOmAdf, seperator, obj->ADonCompOmAplKL.CBIC.CBq, seperator, obj->ADonCompOmAplKL.CBIC.CPyMbr, seperator, obj->ADonCompOmAplKL.CBIC.CVilMbr, seperator, obj->ADonCompOmAplKL.CBIC.CSec, seperator, obj->ADonCompOmAplKL.LSaiOm, seperator, obj->ADonCompOmAplKL.CIdMbrDestGupOm, seperator, obj->DHPriOm, suffix

typedef struct tagSLE0175K               // Confirmation of global order cancellation
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT DSeaBs[8];                     // Trading date
    T_TXT HSeaBs[6];                     // Trading time
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT CIdGrc[2];                     // Instrument group identification
    T_TXT CValIsin[12];                  // Instrument Identification
    T_TXT ISensOm;                       // Side of order
    T_TXT YCpteOm;                       // Type of Clearing Account for Member that owns the order
    T_TXT YOm;                           // Code for the Technical Origin of the Order
    T_TXT CIdItfOm[11];                  // ID of the member's order entry server

} SLE0175K, *PSLE0175K;

#define SIZEOF_SLE0175K_MIN (70)
#define SIZEOF_SLE0175K_MAX (70)
#define SLE0175K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBs[%.8s];%sHSeaBs[%.6s];%sCIdAdfEmetOm[%.8s];%sCIdGrc[%.2s];%sCValIsin[%.12s];%sISensOm[%.1s];%sYCpteOm[%.1s];%sYOm[%.1s];%sCIdItfOm[%.11s];""%s"
#define SLE0175K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBs, seperator, obj->HSeaBs, seperator, obj->CIdAdfEmetOm, seperator, obj->CIdGrc, seperator, obj->CValIsin, seperator, &obj->ISensOm.data, seperator, &obj->YCpteOm.data, seperator, &obj->YOm.data, seperator, obj->CIdItfOm, suffix

typedef struct tagSLE0191K               // Confirmation of command to enter a request for quote
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT DSeaBs[8];                     // Trading date
    T_TXT HSeaBs[6];                     // Trading time
    T_TXT CValIsin[12];                  // Instrument Identification
    T_NUM QTitDemP[12];                  // Quantity of the Request For Quote
    struct tagPTITACPUP                  // Price of the Request For Quote
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PTITACPUP;
    T_TXT INonDifCAdhOn;                 // Member ID Broadcast Flag

} SLE0191K, *PSLE0191K;

#define SIZEOF_SLE0191K_MIN (69)
#define SIZEOF_SLE0191K_MAX (69)
#define SLE0191K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBs[%.8s];%sHSeaBs[%.6s];%sCValIsin[%.12s];%sQTitDemP[%.12s];%sPTITACPUP.IFt[%.1s];%sPTITACPUP.QMt[%.9s];%sINonDifCAdhOn[%.1s];""%s"
#define SLE0191K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBs, seperator, obj->HSeaBs, seperator, obj->CValIsin, seperator, obj->QTitDemP, seperator, &obj->PTITACPUP.IFt.data, seperator, obj->PTITACPUP.QMt, seperator, &obj->INonDifCAdhOn.data, suffix

typedef struct tagSLE0460K               // User Defined Strategy creation notice
{
    T_TXT LRfIntAdfMsg[16];              // Internal subscriber reference
    T_NUM CFon[4];                       // NSC function code
    T_TXT CIdAdfEmetOm[8];               // ID of trading system member that issued the order
    T_TXT CIdNgSaiOm[8];                 // ID of the trader that entered the order
    T_TXT YQStg[3];                      // Strategy Quantity Type
    T_TXT YStg[2];                       // Strategy Type
    struct tagPCpsDrvObl                 // Future-type leg price
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[9];                    // Amount

    } PCpsDrvObl;
    struct tagXDtaStg                    // Delta Strategy percentage
    {
        T_TXT IFt;                       // Decimal point locator (AtosEuronext)
        T_NUM QMt[18];                   // Amount

    } XDtaStg;
    T_NUM ZCpsStg[2];                    // Strategy Leg Number
    T_TXT CIsinStg[12];                  // Strategy Identification Code
    struct tagACpsCreStg                 // Leg description aggregate
    {
        T_TXT CIsinProdCps[12];          // Underlying Identification Code
        struct tagARaoCps                // Leg ratio description aggregate
        {
            T_TXT CSignKMuProdCps;       // Component product multiplication coefficient sign
            T_NUM KRaoCpsStg[4];         // Leg number Ratio

        } ARaoCps;

    } ACpsCreStg[40];
    T_TXT IInOmStg;                      // Including Order Indicator
    struct tagAInOmStg                   // Order entry aggregate
    {
        T_TXT YPLimSaiOm;                // Type of Limit for an Order
        struct tagPLimSaiOm              // Original Order Price
        {
            T_TXT IFt;                   // Decimal point locator (AtosEuronext)
            T_NUM QMt[9];                // Amount

        } PLimSaiOm;
        T_TXT YValiOmNSC;                // Validity Type of an Order
        T_NUM DVALIOM[8];                // Order validity date
        T_NUM TValiOm[6];                // Order Validity Duration
        T_NUM QTitDvlOm[12];             // Order Disclosed Quantity
        T_NUM QTitTotOm[12];             // Order Total Quantity
        struct tagPDchOmStop             // Original Trigger Price
        {
            T_TXT IFt;                   // Decimal point locator (AtosEuronext)
            T_NUM QMt[9];                // Amount

        } PDchOmStop;
        T_TXT ISensOm;                   // Order Side
        struct tagADonCompOmAplKL        // KLSE Clearing data aggregate
        {
            T_TXT CIdNgSaiOm[8];         // ID of the trader that entered the order
            T_TXT YCpteOm;               // Type of Clearing Account for Member that owns the order
            T_TXT NCptePosIptOm[16];     // Client Account Number
            T_TXT CIdOmNg[8];            // Trader Order Number (TON)
            T_TXT DHSaiOmAdf[14];        // Order Entry Date and Time
            struct tagCBIC               // BIC Code of the Member
            {
                T_TXT CBq[4];            // Bank Code
                T_TXT CPyMbr[2];         // Country Code
                T_TXT CVilMbr[2];        // Town Code
                T_TXT CSec[3];           // Branch Code

            } CBIC;
            T_TXT LSaiOm[18];            // Free text entered with order
            T_TXT CIdMbrDestGupOm[8];    // ID of Clearing System Member that is the beneficiary of a give-up

        } ADonCompOmAplKL;

    } AInOmStg;

} SLE0460K, *PSLE0460K;

#define SIZEOF_SLE0460K_MIN (910)
#define SIZEOF_SLE0460K_MAX (910)
#define SLE0460K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sCIdAdfEmetOm[%.8s];%sCIdNgSaiOm[%.8s];%sYQStg[%.3s];%sYStg[%.2s];%sPCpsDrvObl.IFt[%.1s];%sPCpsDrvObl.QMt[%.9s];%sXDtaStg.IFt[%.1s];%sXDtaStg.QMt[%.18s];%sZCpsStg[%.2s];%sCIsinStg[%.12s];%sIInOmStg[%.1s];%sAInOmStg.YPLimSaiOm[%.1s];%sAInOmStg.PLimSaiOm.IFt[%.1s];%sAInOmStg.PLimSaiOm.QMt[%.9s];%sAInOmStg.YValiOmNSC[%.1s];%sAInOmStg.DVALIOM[%.8s];%sAInOmStg.TValiOm[%.6s];%sAInOmStg.QTitDvlOm[%.12s];%sAInOmStg.QTitTotOm[%.12s];%sAInOmStg.PDchOmStop.IFt[%.1s];%sAInOmStg.PDchOmStop.QMt[%.9s];%sAInOmStg.ISensOm[%.1s];%sAInOmStg.ADonCompOmAplKL.CIdNgSaiOm[%.8s];%sAInOmStg.ADonCompOmAplKL.YCpteOm[%.1s];%sAInOmStg.ADonCompOmAplKL.NCptePosIptOm[%.16s];%sAInOmStg.ADonCompOmAplKL.CIdOmNg[%.8s];%sAInOmStg.ADonCompOmAplKL.DHSaiOmAdf[%.14s];%sAInOmStg.ADonCompOmAplKL.CBIC.CBq[%.4s];%sAInOmStg.ADonCompOmAplKL.CBIC.CPyMbr[%.2s];%sAInOmStg.ADonCompOmAplKL.CBIC.CVilMbr[%.2s];%sAInOmStg.ADonCompOmAplKL.CBIC.CSec[%.3s];%sAInOmStg.ADonCompOmAplKL.LSaiOm[%.18s];%sAInOmStg.ADonCompOmAplKL.CIdMbrDestGupOm[%.8s];""%s"
#define SLE0460K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->CIdAdfEmetOm, seperator, obj->CIdNgSaiOm, seperator, obj->YQStg, seperator, obj->YStg, seperator, &obj->PCpsDrvObl.IFt.data, seperator, obj->PCpsDrvObl.QMt, seperator, &obj->XDtaStg.IFt.data, seperator, obj->XDtaStg.QMt, seperator, obj->ZCpsStg, seperator, obj->CIsinStg, seperator, &obj->IInOmStg.data, seperator, &obj->AInOmStg.YPLimSaiOm.data, seperator, &obj->AInOmStg.PLimSaiOm.IFt.data, seperator, obj->AInOmStg.PLimSaiOm.QMt, seperator, &obj->AInOmStg.YValiOmNSC.data, seperator, obj->AInOmStg.DVALIOM, seperator, obj->AInOmStg.TValiOm, seperator, obj->AInOmStg.QTitDvlOm, seperator, obj->AInOmStg.QTitTotOm, seperator, &obj->AInOmStg.PDchOmStop.IFt.data, seperator, obj->AInOmStg.PDchOmStop.QMt, seperator, &obj->AInOmStg.ISensOm.data, seperator, obj->AInOmStg.ADonCompOmAplKL.CIdNgSaiOm, seperator, &obj->AInOmStg.ADonCompOmAplKL.YCpteOm.data, seperator, obj->AInOmStg.ADonCompOmAplKL.NCptePosIptOm, seperator, obj->AInOmStg.ADonCompOmAplKL.CIdOmNg, seperator, obj->AInOmStg.ADonCompOmAplKL.DHSaiOmAdf, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CBq, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CPyMbr, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CVilMbr, seperator, obj->AInOmStg.ADonCompOmAplKL.CBIC.CSec, seperator, obj->AInOmStg.ADonCompOmAplKL.LSaiOm, seperator, obj->AInOmStg.ADonCompOmAplKL.CIdMbrDestGupOm, suffix

typedef struct tagSLE0500K               // Client Amendment
{
    T_TXT LRfIntAdfMsg[16];              // Internal Subscriber Reference
    T_NUM CFon[4];                       // NSC Function Code
    T_TXT DSeaBs[8];                     // Trading Date
    T_TXT HSeaBs[6];                     // Trading Time
    T_NUM NSeqOm[6];                     // Order Sequential Number
    T_TXT ISensOm;                       // Order Side
    T_TXT CIdNgSaiOm[8];                 // ID of the trader that entered the order
    T_TXT NCptePosIptOm[16];             // New Client account number
    T_TXT NCptePosIptOmOrig[16];         // Original Client account number
    T_TXT CValIsin[12];                  // Instrument Identification
    struct tagCBIC                       // BIC Code of the Member
    {
        T_TXT CBank[4];                  // Bank Identifier Code
        T_TXT Country[2];                // Country Code
        T_TXT CTown[2];                  // Town Code
        T_TXT CBranch[3];                // Branch Code

    } CBIC;
    T_TXT CIdOmNg[8];                    // Trader Order Number (TON)
    T_TXT LSaiOm[18];                    // Free text entered with order
    T_TXT NCpteTraIptOrig[9];            // Original Trading Client Id
    T_TXT HAckMess[6];                   // Acknowledgment time

} SLE0500K, *PSLE0500K;

#define SIZEOF_SLE0500K_MIN (145)
#define SIZEOF_SLE0500K_MAX (145)
#define SLE0500K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBs[%.8s];%sHSeaBs[%.6s];%sNSeqOm[%.6s];%sISensOm[%.1s];%sCIdNgSaiOm[%.8s];%sNCptePosIptOm[%.16s];%sNCptePosIptOmOrig[%.16s];%sCValIsin[%.12s];%sCBIC.CBank[%.4s];%sCBIC.Country[%.2s];%sCBIC.CTown[%.2s];%sCBIC.CBranch[%.3s];%sCIdOmNg[%.8s];%sLSaiOm[%.18s];%sNCpteTraIptOrig[%.9s];%sHAckMess[%.6s];""%s"
#define SLE0500K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBs, seperator, obj->HSeaBs, seperator, obj->NSeqOm, seperator, &obj->ISensOm.data, seperator, obj->CIdNgSaiOm, seperator, obj->NCptePosIptOm, seperator, obj->NCptePosIptOmOrig, seperator, obj->CValIsin, seperator, obj->CBIC.CBank, seperator, obj->CBIC.Country, seperator, obj->CBIC.CTown, seperator, obj->CBIC.CBranch, seperator, obj->CIdOmNg, seperator, obj->LSaiOm, seperator, obj->NCpteTraIptOrig, seperator, obj->HAckMess, suffix

typedef struct tagSLE0501K               // Client Amendment Notice
{
    T_TXT LRfIntAdfMsg[16];              // Internal Subscriber Reference
    T_NUM CFon[4];                       // NSC Function Code
    T_TXT DSeaBs[8];                     // Trading Date
    T_TXT HSeaBs[6];                     // Trading Time
    T_NUM NSeqOm[6];                     // Order Sequential Number
    T_TXT ISensOm;                       // Order Side
    T_TXT CIdNgSaiOm[8];                 // ID of the trader that entered the order
    T_TXT NCptePosIptOm[16];             // New Client account number
    T_TXT NCptePosIptOmOrig[16];         // Original Client account number
    T_TXT CValIsin[12];                  // Instrument Identification
    struct tagCBIC                       // BIC Code of the Member
    {
        T_TXT CBank[4];                  // Bank Identifier Code
        T_TXT Country[2];                // Country Code
        T_TXT CTown[2];                  // Town Code
        T_TXT CBranch[3];                // Branch Code

    } CBIC;
    T_TXT CIdOmNg[8];                    // Trader Order Number (TON)
    T_TXT LSaiOm[18];                    // Free text entered with order
    T_TXT NCpteTraIptOrig[9];            // Original Trading Client Id
    T_TXT HAckMess[6];                   // Acknowledgment time

} SLE0501K, *PSLE0501K;

#define SIZEOF_SLE0501K_MIN (145)
#define SIZEOF_SLE0501K_MAX (145)
#define SLE0501K_PRINTF_FORMAT "%s""%sLRfIntAdfMsg[%.16s];%sCFon[%.4s];%sDSeaBs[%.8s];%sHSeaBs[%.6s];%sNSeqOm[%.6s];%sISensOm[%.1s];%sCIdNgSaiOm[%.8s];%sNCptePosIptOm[%.16s];%sNCptePosIptOmOrig[%.16s];%sCValIsin[%.12s];%sCBIC.CBank[%.4s];%sCBIC.Country[%.2s];%sCBIC.CTown[%.2s];%sCBIC.CBranch[%.3s];%sCIdOmNg[%.8s];%sLSaiOm[%.18s];%sNCpteTraIptOrig[%.9s];%sHAckMess[%.6s];""%s"
#define SLE0501K_PRINTF_ARGS(obj, prefix, seperator, suffix) , prefix, seperator, obj->LRfIntAdfMsg, seperator, obj->CFon, seperator, obj->DSeaBs, seperator, obj->HSeaBs, seperator, obj->NSeqOm, seperator, &obj->ISensOm.data, seperator, obj->CIdNgSaiOm, seperator, obj->NCptePosIptOm, seperator, obj->NCptePosIptOmOrig, seperator, obj->CValIsin, seperator, obj->CBIC.CBank, seperator, obj->CBIC.Country, seperator, obj->CBIC.CTown, seperator, obj->CBIC.CBranch, seperator, obj->CIdOmNg, seperator, obj->LSaiOm, seperator, obj->NCpteTraIptOrig, seperator, obj->HAckMess, suffix


#pragma pack()


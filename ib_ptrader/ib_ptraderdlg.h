/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include "resource.h"
#include "afxwin.h"
#include "DataTypes.h"
#include "Ticker.h"
#include "TickerListBox.h"
#include "contract.h"
#include "share.h"
#include "OutstandingOrder.h"
#include "Stats.h"
#include "ChartViewDlg.h"
#include "TechnicalAnalysis.h"
#include <hash_map>
#include <xercesc/util/XMLString.hpp>
#include <cppconn/driver.h>



const UINT_PTR TWS_MESSAGES_REFRESH_TIMER = 1;
const UINT_PTR CHECKSELLSTARTS_TIMER = 2;
const UINT_PTR RECORDPRICES_TIMER = 3;


class AppSettings;


class IB_PTraderDlg : public CDialog, public EWrapper
{
private:
    #pragma region "Interactive Broker Connection Values"    
    int _CLIENTID;
    UINT _PORTID;
    #pragma endregion

// Construction
public:
    IB_PTraderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
    enum { IDD = IDD_IB_TRYAGAIN_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

private:
    EClient *_pclient;

private:
    #pragma region "Helpers"
    CString GetTickTypeDisplay (TickType tickType);
    void InvalidateTickerDisplay (Ticker *ticker);
    #pragma endregion

private:
    #pragma region "Interactive Broker TWS Codes"
    CString GetFuncHeaderForLogging (const CString &funcname);

    enum TWSCodeType
    {
        TWS_Error, TWS_System, TWS_Warning
    };

    TWSCodeType GetTWSCodeType (const int errorCode);
    CString GetTWSCodeType_AsString (TWSCodeType twscodetype);
    #pragma endregion
                         
public:
    #pragma region "Interactive Broker CallBacks"
    void tickPrice (TickerId ddeId, TickType field, double price, int canAutoExecute);
    void tickSize (TickerId ddeId, TickType field, int size);
    void tickOptionComputation (TickerId ddeId, TickType field, double impliedVol,
                                double delta, double optPrice, double pvDividend,
                                double gamma, double vega, double theta, double undPrice);
    void tickGeneric (TickerId tickerId, TickType tickType, double value);
    void tickString (TickerId tickerId, TickType tickType, const IBString& value);
    void tickEFP (TickerId tickerId, TickType tickType, double basisPoints,
                  const IBString& formattedBasisPoints, double totalDividends, int holdDays,
                  const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry);
    void orderStatus (OrderId orderId, const IBString &status, int filled, int remaining, 
                      double avgFillPrice, int permId, int parentId, double lastFillPrice,
                      int clientId, const IBString& whyHeld);
    void openOrder (OrderId orderId, const Contract&, const Order&, const OrderState&);
    void openOrderEnd ();
    void winError (const IBString &str, int lastError);
    void connectionClosed ();
    void updateAccountValue (const IBString &key, const IBString &val,
                             const IBString &currency, const IBString &accountName);
    void updatePortfolio (const Contract& contract, int position,
                          double marketPrice, double marketValue, double averageCost,
                          double unrealizedPNL, double realizedPNL, const IBString &accountName);
    void updateAccountTime (const IBString &timeStamp);
    void accountDownloadEnd (const IBString &accountName);
    void nextValidId (OrderId orderId);
    void contractDetails (int reqId, const ContractDetails& contractDetails);
    void bondContractDetails (int reqId, const ContractDetails& contractDetails);
    void contractDetailsEnd (int reqId);
    void execDetails (int reqId, const Contract& contract, const Execution& execution);
    void execDetailsEnd (int reqId);
    void error (const int id, const int errorCode, const IBString errorString);
    void error (const IBString errorString);
    void updateMktDepth (TickerId id, int position, int operation, int side, 
                         double price, int size);
    void updateMktDepthL2 (TickerId id, int position, IBString marketMaker, int operation, 
                          int side, double price, int size);
    void updateNewsBulletin (int msgId, int msgType, const IBString& newsMessage, const IBString& originExch);
    void managedAccounts (const IBString& accountsList);
    void receiveFA (faDataType pFaDataType, const IBString& cxml);
    void historicalData (TickerId reqId, const IBString& date, double open, double high, double low,
                         double close, int volume, int barCount, double WAP, int hasGaps) ;
    void scannerParameters (const IBString &xml);
    void scannerData (int reqId, int rank, const ContractDetails &contractDetails, const IBString &distance,
                      const IBString &benchmark, const IBString &projection, const IBString &legsStr);
    void scannerDataEnd (int reqId);
    void realtimeBar (TickerId reqId, long time, double open, double high, double low, double close,
                      long volume, double wap, int count);
    void currentTime (long time);
    void fundamentalData (TickerId reqId, const IBString& data);
    void deltaNeutralValidation (int reqId, const UnderComp& underComp);
    void tickSnapshotEnd (int reqId);
    void marketDataType (TickerId reqId, int marketDataType);
    void commissionReport (const CommissionReport &commissionReport);
    void position (const IBString &account, const Contract &contract, int position, double avgCost);
    void positionEnd ();
    void accountSummary (int reqId, const IBString &account, const IBString &tag, const IBString &value, const IBString &curency);
    void accountSummaryEnd (int reqId);
    #pragma endregion

    #pragma region "Account Data"
    class AccountValue
    {
    public:
        std::string _accountname;
        std::string _key;

    private:
        std::string _value;

    public:
        std::string _currency;
        std::string _hashkey;
        long _numberhits;
        CTime _lasthittime;

        static std::string GetHashKey (const IBString &key, const IBString &currency, const IBString &accountName);
        static std::string GetHashKey (AccountValue *accountvalue);

        inline std::string GetValue () { return _value; }
        double GetAdjustedCashValue ();
        void Initialise (std::string accountname, std::string key, std::string value, std::string currency);
        AccountValue ();
    };

private:
    typedef std::map <std::string, AccountValue *>::iterator AccountValuesIter;

    CCriticalSection _account_critsection;
    std::map <std::string, AccountValue *> *_accountvalues; // <accountname+key+currency, AccountValue>

public:
    AccountValue *GetAccountValue (std::string key, std::string currency);
    AccountValue *GetCashBalance (std::string &currency);
    #pragma endregion

    #pragma region "Portfolio Data"
public:
    class PortfolioPart
    {
    public:
        Ticker *_ticker;
        Contract _contract;
        std::string _accountname;
        int _position;
        double _marketPrice;
        IBString _longname;

    private:
        double _marketValue; 

    public:
        double _averageCost;
        double _initial_averageCost;
        double _unrealizedPNL; 
        double _realizedPNL;
        long _numberhits;
        CTime _lasthittime;

        static std::string GetHashKey (Contract &contract);
        static std::string GetHashKey (Ticker *ticker);

        inline double GetMarketValue () { return _marketValue; }
        double GetAdjustedMarketValue ();
        void Initialise (Contract &contract, int position, double marketPrice, double marketValue, 
                         double averageCost, double unrealizedPNL, double realizedPNL);
        PortfolioPart (Contract &contract, std::string accountname,
                       int position, double marketPrice, double marketValue, 
                       double averageCost, double unrealizedPNL, double realizedPNL);
    };

private:
    typedef std::map <std::string, std::map <std::string, PortfolioPart *> *>::iterator AccountPortfolioPartsIter;
    typedef std::map <std::string, PortfolioPart *>::iterator PortfolioPartsIter;

    CCriticalSection _portfolio_critsection;
    std::map <std::string, std::map <std::string, PortfolioPart *> *> *_portfolio; // <account name, <portfoliohash, PortfolioPart>>

public:
    PortfolioPart *GetPortfolioPart (std::string &hashkey);
    PortfolioPart *GetPortfolioPart (Contract &contract);
    PortfolioPart *GetPortfolioPart (Ticker *ticker);

private:
    int _contractREQID;
    #pragma endregion

    #pragma region "Share Support"
private:
    typedef std::map <std::string, Share *>::iterator SharesIter;

    std::map <std::string, Share *> *_shares; // <ticker+primaryexch+sectype, Share>

    void ReadSharesFromDB ();
    void SetShare_Buy_DayLimit (Share *share, double daylimit);

public:
    void SetShare_Buy_DayLimit (std::string ticker, std::string primaryexch, double daylimit);
    void SetShare_Sell_DayLimit (std::string ticker, std::string primaryexch, double daylimit);
    void AdjustShareBuyDayLimits ();

public:
    Share *GetShare (Ticker *ticker, std::string &hashkey);
    Share *GetShare (Ticker *ticker, Contract &contract);
    inline Share *GetShare (Ticker *ticker) { return ticker->_share; }

public:
    void IncrementShareCurrentOI (Ticker *ticker, Share *share, long oi, double buycost);
    void DecrementShareCurrentOI (Ticker *ticker, Share *share, long oi, double buycost, bool cancelorder);
    #pragma endregion

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedDbStart();
    afx_msg void OnBnClickedXmlStart();

private:
    #pragma region "ListBoxes"
    TickerListBox _mainlistbox;
    CListBox _statusmessageslistbox;
    #pragma endregion

    #pragma region "GetEditFieldAsNumber"
    long GetEditFieldAsNumber (int editctrlid, bool &valid); 
    #pragma endregion

    #pragma region "Command Line Support"
private:
    bool Parse_TWS_CmdLine (const CString &str);
    bool Parse_DB_CmdLine (const CString &str);
    #pragma endregion

    #pragma region "Logging"
private:
    bool _have_shown_LB_ERR;
    bool _have_shown_LB_ERRSPACE;

public:
    void Log (LPCTSTR message, bool log);
    void Log (LPCTSTR message);
    void Log (CString &message);
    void Log (const wchar_t *message);
    void LogDateTime ();
    #pragma endregion

    #pragma region "DB Interface"
private:
    sql::Driver *_dbdriver;
    sql::Connection *_dbcon;

    bool ConnectToDatabase ();
    void DisconnectFromDatabase ();

    CString _dbschema, _dbusername, _dbpwd, _dbhostname;
    #pragma endregion

    #pragma region "Ticker Support"
private:
    enum TickersInitialisation
    {
        PGen, DB, Both
    };

    Tickers *_tickers;
   
public:
    inline Tickers *GetTickers () { return _tickers; }

private:
    void InitialiseTickers (TickersInitialisation initialisation);
    void StartUp (TickersInitialisation initialisation);

    enum TickerListBoxSort
    {
        AsIs, Alpha, ByGroup
    };

    static int AssignCurrencyWeighting (std::string currency);
    static bool CompareAlphaTickers (const Ticker *a, const Ticker *b);
    static int AssignTickerStateWeighting (Ticker::ActiveState state);
    static bool CompareGroupByTickers (const Ticker *a, const Ticker *b);

    void UpdateTickerListBox (TickerListBoxSort sortorder);
    #pragma endregion

    #pragma region "Read/Write to DB"
    bool UpdateOI_OnDB (sql::Statement *stmt, PGenDatabaseRows::PGenDatabaseRow *dbrow, long newoi, bool updatehitcount);
    bool Update_AvgFillPrice_LastBuyDate_OnDB (sql::Statement *stmt, PGenDatabaseRows::PGenDatabaseRow *dbrow, double avgFillPrice, double avgcommission);
    PGenDatabaseRows *ReadDBRows_FromXML ();
    void WriteDBRows_ToDB (PGenDatabaseRows *dbrows, bool emptydbfirst);
    PGenDatabaseRows *ReadDBRows_FromDB ();
    #pragma endregion

    #pragma region "Trades"
private:
    double GetCommissionPerTrade (double commission, int numberoftrades);
    void Record_BuyTrade (OutStandingOrder *outstandingorder, OrderId orderId, const IBString &status, 
                          int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice,
                          int clientId, const IBString &whyHeld);
    void Record_SellTrade (OutStandingOrder *outstandingorder, OrderId orderId, const IBString &status, 
                           int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice,
                           int clientId, const IBString &whyHeld);
    #pragma endregion

    #pragma region "Buy/Sell"
    #pragma region "Commissions"
private:
    typedef stdext::hash_map <std::string, bool>::iterator ZeroCommsIter;
    stdext::hash_map <std::string, bool> _zero_usd_commissionshares;

    double AdjustTWSCommission (double com, double com_multiplier);
    void InitialiseZeroCommissions ();
    bool IsZeroCommission (std::string ticker, stdext::hash_map <std::string, bool> &zerocommissions);
    #pragma endregion

    #pragma region "Spreads"
private:
    double GetExchangeSpread (Ticker *ticker, bool &spreadcalculated);

    typedef stdext::hash_map <std::string, double>::iterator CustomSpreadIter;
    stdext::hash_map <std::string, double> _custom_allowed_spreads;

    std::string GetKeyForAllowedCustomSpreads (std::string ticker, std::string exchange);
    std::string GetKeyForAllowedCustomSpreads (Ticker *ticker);

    double GetSpreadForTicker (Ticker *ticker);
    bool SpreadAllowed (Ticker *ticker, double currentspread);

public:
    void AddCustomSpread (std::string ticker, std::string exchange, double allowedspread);
    #pragma endregion

    #pragma region "Max Global Currency Buys"
private:
    class GlobalCurrencyBuy
    {
    public:
        double _currenttotal;
        double _maxallowedbuys;

        GlobalCurrencyBuy () : _currenttotal (0), _maxallowedbuys (DBL_MAX)
        {
        }
    };

    typedef std::map <std::string, GlobalCurrencyBuy *>::iterator MaxBuysPerCurrencyIter;
    std::map <std::string, GlobalCurrencyBuy *> _maxbuyspercurrency;

public:
    GlobalCurrencyBuy *GetGlobalCurrencyBuy (std::string currency);
    #pragma endregion

    #pragma region "Buy Averages"
private:
    void SetBuyAverageButtonText (Ticker *ticker);

public:
    void SetBuyBelowAverage (std::string ticker, std::string primaryexch, Ticker::BuyAverages buyaverages);
    void SetAllBuyBelowAverage (std::string currency);
    #pragma endregion

    #pragma region "Calculate Commissions"
private:
    double GetPercentage (double value, double percentage);

    double USD_ExchangeFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue);
    double USD_TransactionFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue);

    double CAD_ExchangeFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue);
    double CAD_ClearingFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue);
    double CAD_TransactionFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue);

    void EnsureMinMaxComs (double minallowed, double maxallowed, double &value, bool &roundedup);
    void EnsureMaxCom (double maxallowed, double &value);
    bool CalculateCommission (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double &min, double &max, bool &roundedup, bool &stampdutyadded);
    #pragma endregion

    #pragma region "Order Support"
    template <class T> static void PushOrder (T *order, T * &single, std::list <T *> * &list)
    {
        if (list)
        {
            list->push_back (order);
            return;
        }
        if (single == 0)
        {
            single = order;
            return;
        }
        list = new std::list <T *> ();
        list->push_back (single);
        list->push_back (order);   
        single = 0;
    }

    static IBString GetPrimaryExchangeForContract (std::string &exchange);
    static void MakeContract (Ticker *ticker, Contract &contract, double strike);

    bool AllowTrade (OutStandingOrder::OrderType ordertype, Ticker *ticker, std::string statstext);
    double RoundBidOffer (Ticker *ticker, double value);

    void PlaceOrder (Contract &contract, Order &order);
    #pragma endregion

    #pragma region "BuySellResult"
    enum BuySellResult
    {
        OK, LackOfFunds, MinOIBreached, MaxOIExceeded, MaxCashExceeded, ShareNotFound, NoCashBalanceFound, 
        PortfolioPart_NotFound, DB_Error, MaxCashDayLimitExceeded, MaxGlobalCurrencyCashExceeded,
        UNKNOWN
    };

private:
    CString ToString (BuySellResult buysellresult)
    {
        switch (buysellresult)
        {
            case OK: { return "OK"; }
            case LackOfFunds: { return "LackOfFunds"; }
            case MinOIBreached: { return "MinOIBreached"; }
            case MaxOIExceeded: { return "MaxOIExceeded"; }
            case MaxCashExceeded: { return "MaxCashExceeded"; }
            case ShareNotFound: { return "ShareNotFound"; }
            case NoCashBalanceFound: { return "NoCashBalanceFound"; }
            case PortfolioPart_NotFound: { return "PortfolioPart_NotFound"; }
            case DB_Error: { return "DB_Error"; }
            case MaxCashDayLimitExceeded: { return "MaxCashDayLimitExceeded"; }
            case MaxGlobalCurrencyCashExceeded: { return "MaxGlobalCurrencyCashExceeded"; }
            case UNKNOWN: { return "UNKNOWN"; }
        }
        return "?";
    }

    bool LogError (BuySellResult result);
    void LogBuySellError (CString buyorsell, BuySellResult result);
    #pragma endregion

    #pragma region "Buy"
private:
    BuySellResult SendBuyOrder_DB (sql::Statement *stmt, Ticker *ticker, Order &order, BuyPricePair *buyprice, int batchsize, double buycost);
    void SendBuyOrder_InMemory (Ticker *ticker, Contract &contract, Order &order, Share *share, BuyPricePair *buyprice, double buycost);
    void Place_BuyOrder (Ticker *ticker, Contract &contract, Order &order, Share *share, BuyPricePair *buyprice, int batchsize);
    BuySellResult SendBuyOrder (Ticker *ticker, Contract &contract, double offer, long buysize, 
                                BuyPricePair *singleorder, std::list <BuyPricePair *> *orderlist, int batchsize);
    bool TradeWouldBeALoss (BuyPricePair *buyprice, Contract &contract, int batchsize, bool &stampdutyadded);
    bool Allow_CheckForBelowAverageBuy (Ticker *ticker, double offer);
    int CheckTechnicalIndicators_OK (Ticker *ticker, std::list <TechnicalIndicator::Indicators> &indicators);
    void Buy_IndividualOrders (Ticker *ticker, std::list <BuyPricePair *> *orderlist, double offer, int &sentordercount, Contract &contract,
                               int &pendingbuys, int &pendingsells, BuySellResult &result);
    void Buy (Ticker *ticker);
    #pragma endregion

    #pragma region "Sell"
private:
    BuySellResult SendSellOrder_DB (sql::Statement *stmt, Ticker *ticker, Order &order, SellPricePair *sellprice, int batchsize);
    void SendSellOrder_InMemory (Ticker *ticker, Contract &contract, Order &order, Share *share, SellPricePair *sellprice);
    void Place_SellOrder (Ticker *ticker, Contract &contract, Order &order, Share *share, bool trendingsellorder, SellPricePair *singleorder, std::list <SellPricePair *> *orderlist, int batchsize);
    BuySellResult SendSellOrder (Ticker *ticker, Contract &contract, double bid, long sellsize,
                                 SellPricePair *singleorder, std::list <SellPricePair *> *orderlist, int batchsize);
    void GetSellSearchParts (Ticker *ticker, int &startsearch, int &endsearch);
    void Sell_IndividualOrders (Ticker *ticker, std::list <SellPricePair *> *orderlist, double bid, int &sentordercount, Contract &contract,
                                int &pendingbuys, int &pendingsells, BuySellResult &result);
    void Sell (Ticker *ticker);
    #pragma endregion
    #pragma endregion
    
    #pragma region "Order Status/Maps"
private:
    OrderId _currentorderid;
    OrderMaps _ordermaps;

public:
    inline OrderMaps &GetOrderMaps () { return _ordermaps; }

private:
    enum OrderStatusKind
    {
        Filled, Cancelled
    };

    bool ResetBuyOrders (OutStandingOrder *outstandingorder, OrderId orderId , Share * &share);
    void CancelBuyOrder (OutStandingOrder *outstandingorder, OrderId orderId);
    void CancelSellOrder (OutStandingOrder *outstandingorder, OrderId orderId);
    void CancelOrder (OutStandingOrder *outstandingorder, OrderId orderId);
    void DisplayOrderCompleted (OutStandingOrder *outstandingorder, LPCTSTR message, bool recalculateavg);
    CString FormatHandleOrderStatusMessage_ForFilled (OutStandingOrder::OrderType ordertype, OutStandingOrder *outstandingorder);
    void HandleOrderStatus (OrderStatusKind statuskind, OutStandingOrder *outstandingorder, OrderId orderId, const IBString &status, int filled, int remaining, 
                            double avgFillPrice, int permId, int parentId, double lastFillPrice,
                            int clientId, const IBString &whyHeld);
    void KillOrder (OutStandingOrder::OrderType expectedordertype);
    #pragma endregion

    #pragma region "Account Type"
private:
    enum AccountType
    {
        Paper, Demo, Live, NONE
    };

    static CString ToString (AccountType accounttype)
    {
        switch (accounttype)
        {
            case Paper: { return "Paper"; }
            case Demo: { return "Demo"; }
            case Live: { return "Live"; }
        }
        return "?";
    }

    AccountType _accounttype;
    std::string _username;
    std::string _accountnumber;
    AppSettings *_appsettings;

public:
    inline const std::string &UserName () { return _username; }
    inline const std::string &AccountNumber () { return _accountnumber; }
    #pragma endregion

    #pragma region "Data Dumping"
private:
    enum DumpFlags
    {
        None = 0, ToFile = 1, Screen = 2, Shares = 4, Orders = 8, BuyTickers = 16, SellTickers = 32
    };

    void DumpNewLineToFile (DumpFlags dumpflags, CFile *file);
    void DumpLineToFile (DumpFlags dumpflags, CFile *file, CString &str);
    void DumpLineToFile (DumpFlags dumpflags, CFile *file, LPCTSTR buf);
    void DumpLineToFile (DumpFlags dumpflags, CFile *file, std::string str);
    void DumpLineToFile (DumpFlags dumpflags, CFile *file, long value);
    void DumpLineToFile (DumpFlags dumpflags, CFile *file, LPCTSTR prefixtext, unsigned long value, LPCSTR postfixtext);
    void DumpLineToFile (DumpFlags dumpflags, CFile *file, unsigned long value);
    void DumpLineToFile (DumpFlags dumpflags, CFile *file, double value);
    void DumpSep (DumpFlags dumpflags, CFile *file);

    void DumpListBoxContents (LPCTSTR filename, CListBox &listbox);

    void DumpData (DumpFlags dumpflags);
    #pragma endregion

    #pragma region "Market Details"
private:
    typedef std::map <std::string, MarketDetails *>::iterator MarketDetailsIter;
    std::map <std::string, MarketDetails *> _marketdetails;

    MarketDetails *GetMarketDetails (std::string key);
    MarketDetails *GetMarketDetails (Ticker *ticker);
    bool IsMarketOpen (std::string currency);
    bool IsMarketOnHoliday (std::string currency);
    MarketDetails *AddMarketDetails (MarketDetails::Market market, std::string currency, int starthour, int startminute, int finishhour, int finishminute);
    void InitialiseMarketDetails ();
    bool IsMarketOpen ();
    #pragma endregion

    #pragma region "Start/Stop Orders"
private:
    enum AllowOrders
    {
        Allow, Stopped
    };

    AllowOrders _allowedorders;
    bool _locked;

    void SetStopGoButtonText ();
    #pragma endregion

    #pragma region "Stats"
private:
    Stats _stats;

public:
    inline Stats &GetStats () { return _stats; }
    #pragma endregion

    #pragma region "Excluded/Allow Early Trade Tickers"
private:
    typedef std::map <std::string, Ticker::ActiveState>::iterator TickersEntryIter;

    std::string GetTickerEntryKey (std::string ticker, std::string exchange);
    std::string GetTickerEntryKey (Ticker *ticker);

    std::map <std::string, Ticker::ActiveState> _inactivetickers;

    void SetTickerActive (Ticker *ticker);

    typedef std::map <std::string, bool>::iterator EarlyBuysTickersEntryIter;

    std::map <std::string, bool> _allow_earlyopen_buy_tickers;  

    void SetAllowEarlyBuyTicker (Ticker *ticker);

public:
    void AddInActiveTicker (std::string ticker, std::string exchange, Ticker::ActiveState state);
    void AddGlobalInActiveTickers (std::list <std::string> &tickerstoignore, Ticker::ActiveState state);
    void AddAllowEarlyBuyTicker (std::string ticker, std::string exchange);
    #pragma endregion

    #pragma region "Breaks"
#if ! PUBLIC_BUILD
private:
    typedef std::map <std::string, bool>::iterator BreaksIter;

    std::map <std::string, bool> _buybreaks;
    std::map <std::string, bool> _sellbreaks;

public:
    void AddBuyBreak (std::string ticker) { _buybreaks [ticker] = true;}
    void AddSellBreak (std::string ticker) { _sellbreaks [ticker] = true; }
#endif
    #pragma endregion

    #pragma region "Fast Sell Searches"
#if ! PUBLIC_BUILD
private:
    CString _lastsearchspeedchecktime;
    int _numberofsellstarts_fails;
    int _max_number_of_sellstarts_fails;

    typedef std::list <Ticker *>::iterator FailedSellStartsIter;
    std::list <Ticker *> _failedsellstarts;

    void SetSellStartsButtonText ();

    int DumpSellStarts (CFile *file, bool wantlogging);
#endif
    #pragma endregion

    #pragma region "Ticker State Changes"
private:
    void Set_DoBuys_DoSells_OnTickerStateChange (Ticker *ticker);
    void Set_DoBuys_DoSells_OnAllsTickersStateChange ();   

    void SetTickerButtonText (Ticker *ticker);
    Ticker *GetSelectedTicker ();
    #pragma endregion

    #pragma region "Trending"
private:
    typedef std::map <std::string, int>::iterator TrendsIter;

    std::map <std::string, int> _trends_for_tickers;

    std::string GetTrendTickerKey (std::string ticker, std::string exchange);

    bool AllowTickerTrendCount (Ticker *ticker, SellPricePair *sellprice, double bid);

public:
    void AddTickerTrend (std::string ticker, std::string exchange, std::string currency, int trendcount);
    #pragma endregion

    #pragma region "User Defined Ticker Averages"
private:
    void SetUserDefinedTickerAverage (std::string ticker, std::string exchange, double average);
    void SetUserDefinedTickerAverage (Ticker * ticker);
    void SetUserDefinedTickerAverages ();
    #pragma endregion

    #pragma region "Account Value Support"
private:
    double _minaccountvaluebalance, _maxaccountvaluebalance;
    #pragma endregion

    #pragma region "Min/Max PnLs"
private:
    double _minPnL, _maxPnL;
    #pragma endregion

    #pragma region "Context Menu Support"
private:
    class DynamicMenuTickerStateChange
    {
    public:
        std::string _currency;
        Ticker::ActiveState _newstate;
        Ticker::BuyAverages _buyaverages;
    };

    typedef std::map <int, DynamicMenuTickerStateChange *> ContextMenuDynamicMaps;
    typedef std::map <int, DynamicMenuTickerStateChange *>::iterator ContextMenuDynamicMapsIter;

    void AddDynamicMenuItems_TidyUp (ContextMenuDynamicMaps &dynamicmenumaps);

    void AddDynamicMenuItem (CMenu &menu, std::string currency, bool somemarketsonholiday, std::string menutext, UINT stateflags, Ticker::ActiveState newstate,
                             Ticker::BuyAverages newbuyaverage, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);

    void AddDynamicMenuItem_TickerStates (CMenu &menu, std::string currency, bool somemarketsonholiday, UINT stateflags, Ticker::ActiveState newstate, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);
    void AddDynamicMenuItem_TickerStates (CMenu &menu, std::string currency, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps, bool somemarketsonholiday, UINT stateflags, ...);
    void AddDynamicMenuItems_TickerStates (CMenu &menu, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);

    void AddDynamicMenuItem_ToggleEarlyBuys (CMenu &menu, std::string currency, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);
    void AddDynamicMenuItems_ToggleEarlyBuys (CMenu &menu, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);

    void AddDynamicMenuItem_BuyAverages (CMenu &menu, std::string currency, Ticker::BuyAverages newbuyaverage, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);
    void AddDynamicMenuItems_BuyAverages (CMenu &menu, bool somemarketsonholiday, UINT stateflags, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);

    void AddDynamicMenuItems_UpdateGlobalCashLimit (std::string currency, long newlimit);
    void AddDynamicMenuItems_ChangeGlobalCashLimit (CMenu &menu, std::string currency, bool validnumber, long number, int &currentmenuid, ContextMenuDynamicMaps &dynamicmenumaps);

    UINT GetContextMenuFlags (Ticker *ticker, UINT inputflags, bool enableconditions, CString formattext, bool validnumber, long number, CString &outputtext);
    UINT GetContextMenuFlags (Ticker *ticker, CString &outputtext);
    void ContextMenuChangeTickersState (Ticker::ActiveState newstate, std::string currency);
    void ContextMenuToggleEarlyBuy (Ticker *ticker, CString &tickerlist);
    void ContextMenuToggleEarlyBuys (std::string currency);
    void ContextMenuToggleEarlyBuys_Log (Ticker *ticker, std::string currency, CString &tickerlist);
    void ContextMenuSetBuyAverages (DynamicMenuTickerStateChange *tickerstatechange);

    bool TidyUp_Handle_OnContextMenu (std::list <ContextMenuDynamicMaps> &dynamicmapslist);
    bool Handle_OnContextMenu (CWnd *pwnd, CPoint &point);
    #pragma endregion

    #pragma region "File Support"
private:
    CString _currentworkingdir;

    CString GetFileName (CString filename);

public:
    inline CString &CurrentWorkingDir () { return _currentworkingdir; }
    #pragma endregion

    #pragma region "Set Window Title"
private:
    bool _titletextsetfirsttime;
    CString _accountselection;
    CString _orignalwindowtitletext, _extrawindowtitletext;

    void SetWindowTitle (int numberitemsrecorded, CTime *recordtime);
    void SetWindowTitle ();
    #pragma endregion

    #pragma region "Record Prices"
private:
    typedef std::list <Ticker *>::iterator RecordTickersIter;
    std::list <Ticker *> *_tickers_to_record;

public:
    static void Write_ToFile (CFile &file, CString &str);
    static void Write_NewLineToFile (CFile &file);

private:
#if ! PUBLIC_BUILD
    bool RecordPrice_ToDB (Ticker *ticker);
    bool RecordPrice_ToFile (Ticker *ticker, CTime &now);
#endif
    void RecordPrices (CTime &now);

public:
    void AddTickerToRecord (std::string ticker, std::string primaryexch, std::string sectype, Ticker::PriceRecordType pricerecordtype);
    #pragma endregion

    #pragma region "Display"
#if ! PUBLIC_BUILD
private:
    bool _displayquotesdiagnostics;
#endif
    #pragma endregion

    #pragma region "Liquidation Support"
private:
    class Liquidations
    {
    public:
        std::list <BuyPricePair *> _liquidations;
        double _totalsum;
        double _PnL;

        int Count ();
        long NumberOfShares ();
        void Reset ();
        void DivideDown (double byamount);

        Liquidations ();
    };

    class LiquidationData
    {
    public:
        Liquidations _profits;
        Liquidations _losses;

        bool Empty ();
        void Reset ();
    };

    bool GetLiquidationHrsMins (long time, long &hr, long &min);
    CTime GetLiquidationTime (long hr, long min);

    void GetSellOrdersToLiquidate (std::list <BuyPricePair *> &liquidations, SellPricePair * &singleorder, std::list <SellPricePair *> * &orderlist);
    void GetSellOrdersToLiquidate (Ticker::LiquidateOptions liquidateoption, LiquidationData &data, SellPricePair * &singleorder, std::list <SellPricePair *> * &orderlist);
    void GetLiquidationsForTicker (Ticker *ticker, bool todayonly, LiquidationData &data);
    void CheckForLiquidation (CTime &now);
    void DoLiquidation (Ticker::LiquidateOptions liquidateoption, Ticker *ticker, LiquidationData &data, bool todayonly, Contract &contract, bool silent, std::string displaytext, std::string callingfunction);
    CString GetLiquidatePnLText (Ticker *ticker, CString text, long numbershares, double profits, double losses, double profits_totalsum, double losses_totalsum, double maxcom, double mincom, 
                                 Contract &contract, bool log);
    CString IB_PTraderDlg::GetLiquidatePnLText (Ticker *ticker, LiquidationData &data, Contract &contract, bool log);
    static bool SortLiquidations (const BuyPricePair *d1, const BuyPricePair *d2);
    CString DumpLiquidationStats (Ticker *ticker, Liquidations &liquidations, std::string text, bool log);
    void GoLiquidate (bool todayonly);

public:
    void InitialiseLiquidation (std::string ticker, std::string primaryexch, std::string sectype, Ticker::LiquidateOptions liquidateption, long time);
    #pragma endregion

    #pragma region "Chart View"
public:
    CChartViewDlg *GetChartViewDlg (Ticker *ticker, CChartViewDlg::ChartKind chartkind, CChartViewDlg::ChartPeriod chartperiod, int day = 0, int month = 0, int year = 0);

private:
    void CreateChartDataView (Ticker *ticker, CChartViewDlg::ChartKind chartkind, CChartViewDlg::ChartPeriod chartperiod, int day = 0, int month = 0, int year = 0);
    #pragma endregion

    #pragma region "Tickers Even Buys/On/Off Support"
public:
    void SetTickersBuyOnOff (Ticker *ticker, Ticker::OnOffOptions onoffoptions, bool wantbuysellcheck);
    void SetTickersBuyOnOff (std::string ticker, std::string exchange, Ticker::OnOffOptions onoffoptions);
    #pragma endregion

    #pragma region "Technical Analysis"
public:
    void SetTickersTA (std::string ticker, std::string exchange, std::string sectype, TechnicalIndicator::Indicators TAindicator);
    #pragma endregion

    #pragma region "Friends of the Family"
    friend class IB_DataDumpDlg;
    friend class Stats;
    friend class OrderMaps;
    friend class TickerListBox;
    #pragma endregion

    afx_msg void OnBnClickedDatadump();
    afx_msg void OnBnClickedListdump();
    afx_msg void OnBnClickedInternaldata();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnLbnDblclkQuoteslist();
    afx_msg void OnBnClickedKillbuyorder();
    afx_msg void OnBnClickedKillsellorder();
    afx_msg void OnBnClickedStartstop();
    afx_msg void OnLbnSelchangeMessages();
    afx_msg void OnLbnDblclkMessages();
    afx_msg void OnBnClickedStatsscreen();
    afx_msg void OnBnClickedStatsfile();
    afx_msg void OnBnClickedSellstarts();
    afx_msg void OnBnClickedSellstartsfile();
    afx_msg void OnBnClickedTogglesellspeed();
    afx_msg void OnBnClickedKillallorders();
    afx_msg void OnBnClickedShowallorders();
    afx_msg void OnBnClickedTickeractive();
    afx_msg void OnBnClickedScrollmessages();
    afx_msg void OnBnClickedClearmessages();
    afx_msg void OnLbnSelcancelQuoteslist();
    afx_msg void OnLbnSelchangeQuoteslist();
    afx_msg void OnBnClickedGotoActive();
    afx_msg void OnBnClickedGotoInactive();
    afx_msg void OnBnClickedGotoBuy();
    afx_msg void OnBnClickedGotoSell();
    afx_msg void OnBnClickedDumpquotes();
    afx_msg void OnBnClickedUnselectQuotes();
    afx_msg void OnBnClickedUnselectMessages();
    afx_msg void OnBnClickedAvgAction();
    afx_msg void BatchBuyClick();
    afx_msg void OnBnClickedBatchbuycheck();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnBnClickedLiquidate();
    afx_msg void OnBnClickedAllliquidate();
};

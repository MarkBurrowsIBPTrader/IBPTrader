/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "ib_ptraderdlg.h"



CString IB_PTraderDlg::GetFuncHeaderForLogging (const CString &funcname)
{
    return "**********IB_PTraderDlg::" + funcname;
}


void IB_PTraderDlg::tickPrice (TickerId ddeId, TickType field, double price, int canAutoExecute)
{
    if (_tickers == 0 || price <= 0)
        return;
    Ticker *ticker;
    switch (field)
    {
        #pragma region "BID"
        case BID:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                ticker->_currentbid = price;
                ticker->_tickvolume_bid++;

                if (! ticker->_minmaxes_set_bid)
                {
                    MarketDetails *marketdetails = GetMarketDetails (ticker);
                    CTime now = CTime::GetCurrentTime ();
                    MarketDetails::MarketStatus marketstatus = MarketDetails::GetMarketStatus (marketdetails, now); 
                    if (MarketDetails::IsMarketOpen (marketstatus))
                    {
                        ticker->_minmaxes_set_bid = true;
                    }
                }
                if (ticker->_minmaxes_set_bid)
                {
                    if (price > ticker->_maxbid)
                        ticker->_maxbid = price;

                    if (price < ticker->_minbid)
                        ticker->_minbid = price;
                }
                if (ticker->_livepriceset)
                {
                    ticker->_livepriceset->PriceChange ();
                }
                Sell (ticker);
            }
            else
            {
                CString format;
                format.Format ("BID TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickPrice", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
        #pragma region "ASK"
        case ASK:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                ticker->_currentoffer = price;
                ticker->_tickvolume_offer++;

                if (! ticker->_minmaxes_set_offer)
                {
                    MarketDetails *marketdetails = GetMarketDetails (ticker);
                    CTime now = CTime::GetCurrentTime ();
                    MarketDetails::MarketStatus marketstatus = MarketDetails::GetMarketStatus (marketdetails, now); 
                    if (MarketDetails::IsMarketOpen (marketstatus))
                    {
                        ticker->_minmaxes_set_offer = true;
                    }
                }
                if (ticker->_minmaxes_set_offer)
                {
                    if (price > ticker->_maxoffer)
                        ticker->_maxoffer = price;

                    if (price < ticker->_minoffer)
                        ticker->_minoffer = price;
                }
                bool addednewprice = false;
                if (ticker->_livepriceset)
                {
                    addednewprice = ticker->_livepriceset->PriceChange () != 0;
                }
                if (ticker->_technicalindicators && addednewprice)
                {
                    ticker->_technicalindicators->Calculate ();
                }
                Buy (ticker);
            }
            else
            {
                CString format;
                format.Format ("ASK TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickPrice", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
        #pragma region "LOW"
        case LOW:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                if (price < ticker->_ib_low)
                    ticker->_ib_low = price;
            }
            else
            {
                CString format;
                format.Format ("LOW TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickPrice", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
        #pragma region "HIGH"
        case HIGH:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                if (price > ticker->_ib_high)
                    ticker->_ib_high = price;
            }
            else
            {
                CString format;
                format.Format ("HIGH TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickPrice", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
        #pragma region "OPEN"
        case OPEN:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                ticker->_ib_open = price;
            }
            else
            {
                CString format;
                format.Format ("OPEN TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickPrice", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
    }
}


void IB_PTraderDlg::tickSize (TickerId ddeId, TickType field, int size)
{
    Ticker *ticker;
    switch (field)
    {
        #pragma region "BID_SIZE"
        case BID_SIZE:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                ticker->_bidsize = size;
                ticker->_tickdepth_bid++;
            }
            else
            {
                CString format;
                format.Format ("BID_SIZE TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickSize", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
        #pragma region "ASK_SIZE"
        case ASK_SIZE:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                ticker->_offersize = size;
                ticker->_tickdepth_offer++;
            }
            else
            {
                CString format;
                format.Format ("ASK_SIZE TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickSize", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
        #pragma region "VOLUME"
        case VOLUME:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                ticker->_volume = (size * ticker->_volumemultiplier);
            }
            else
            {
                CString format;
                format.Format ("VOLUME TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickSize", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
        #pragma region "AVG_VOLUME"
        case AVG_VOLUME:
        {
            ticker = _tickers->GetTicker (ddeId);
            if (ticker)
            {
                ticker->_avgvolume = (size * ticker->_volumemultiplier);
            }
            else
            {
                CString format;
                format.Format ("AVG_VOLUME TickerId %ld not found", ddeId);
                _stats.IncrementStat ("tickSize", (LPCTSTR) format, "", "");
            }
            break;
        }
        #pragma endregion
    }
}


void IB_PTraderDlg::tickOptionComputation (TickerId ddeId, TickType field, double impliedVol,
                                           double delta, double optPrice, double pvDividend,
                                           double gamma, double vega, double theta, double undPrice)
{
}


void IB_PTraderDlg::tickGeneric (TickerId tickerId, TickType tickType, double value)
{
}


void IB_PTraderDlg::tickString (TickerId tickerId, TickType tickType, const IBString& value)
{
}


void IB_PTraderDlg::tickEFP (TickerId tickerId, TickType tickType, double basisPoints,
                             const IBString& formattedBasisPoints, double totalDividends, int holdDays,
                             const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry)
{
}


bool IB_PTraderDlg::IsMarketOpen ()
{
    MarketDetails *marketdetails = GetMarketDetails ("USD");
    CTime now =  CTime::GetCurrentTime ();
    MarketDetails::MarketStatus marketstatus = MarketDetails::GetMarketStatus (marketdetails, now);
    return MarketDetails::IsMarketOpen (marketstatus);
}


void IB_PTraderDlg::orderStatus (OrderId orderId, const IBString &status, int filled, int remaining, 
                                 double avgFillPrice, int permId, int parentId, double lastFillPrice,
                                 int clientId, const IBString &whyHeld)
{
    OutStandingOrder *outstandingorder = _ordermaps.GetOutStandingOrder (orderId);
    if (outstandingorder == 0)
    {
        if (! IsMarketOpen ())
        {
            _stats.IncrementStat ("orderStatus", "MarketClosed", "", "");
            return;
        }
    }
    CString message = GetFuncHeaderForLogging ("orderStatus") + " for ", str;
    str.Format ("%ld (", orderId);
    message += str;
    message += status;
    str.Format (") %d filled, %d remaining", filled, remaining);
    message += str;
    Log (message);
    str.Format ("AvgFill %g, Last Fill %g, ", avgFillPrice, lastFillPrice);
    message = str;
    str.Format ("PermId %d, ParentId %d, ClientId %d", permId, parentId, clientId);
    message += str;
    Log (message);
    if (outstandingorder)
    {
        if (status == "Filled")
        {
            if (remaining == 0)
            {
                HandleOrderStatus (Filled, outstandingorder, orderId, status, filled, remaining, 
                                   avgFillPrice, permId, parentId, lastFillPrice,
                                   clientId, whyHeld);
            }
        }
        else if (status == "Cancelled" || status == "Inactive")
        {
            HandleOrderStatus (Cancelled, outstandingorder, orderId, status, filled, remaining, 
                               avgFillPrice, permId, parentId, lastFillPrice,
                               clientId, whyHeld);
        }
    }
    else
    {
        Log ("OrderId NOT FOUND - Duplicate Message?");
    }
}


double IB_PTraderDlg::AdjustTWSCommission (double com, double com_multiplier)
{
    if (OutStandingOrder::IsCommissionValid (com))
    {
        return com * com_multiplier;
    }
    return com;
}


void IB_PTraderDlg::openOrder (OrderId orderId, const Contract &contract, const Order &order, const OrderState &orderstate)
{
    OutStandingOrder *outstandingorder = _ordermaps.GetOutStandingOrder (orderId);
    if (outstandingorder)
    {
        CString message = GetFuncHeaderForLogging ("openOrder") + " for ", str;
        str.Format ("%ld (", orderId);
        message += str;
        message += orderstate.status;
        message += ")";    
        Log (message);
        Log ("status: " + orderstate.status);
        Log ("initMarg: " + orderstate.initMargin + ", maintMarg: " + orderstate.maintMargin + ", equityLoan: " + orderstate.equityWithLoan);

        double com_multiplier;
        if (orderstate.commissionCurrency == "GBP")
            com_multiplier = 100;
        else
            com_multiplier = 1;
        double com = AdjustTWSCommission (orderstate.commission, com_multiplier);
        double min_com = AdjustTWSCommission (orderstate.minCommission, com_multiplier);
        double max_com = AdjustTWSCommission (orderstate.maxCommission, com_multiplier);

        str.Format ("com: %g, mincom: %g, maxcom: %g %s", com, min_com, max_com, (com_multiplier != 1 ? "(ADJUSTED)" : ""));
        Log (str);
        Log ("comCurr: " + orderstate.commissionCurrency + ", warning: " + orderstate.warningText);

        outstandingorder->_orderstate.status = orderstate.status;
        outstandingorder->_orderstate.initMargin = orderstate.initMargin;
        outstandingorder->_orderstate.maintMargin = orderstate.maintMargin;
        outstandingorder->_orderstate.equityWithLoan = orderstate.equityWithLoan;
        outstandingorder->_orderstate.commission = com;
        outstandingorder->_orderstate.minCommission = min_com;
        outstandingorder->_orderstate.maxCommission = max_com;
        outstandingorder->_orderstate.commissionCurrency = orderstate.commissionCurrency;
        outstandingorder->_orderstate.warningText = orderstate.warningText;
    }
    else
    {
        if (! IsMarketOpen ())
        {
            _stats.IncrementStat ("openOrder", "MarketClosed", (LPCTSTR) contract.symbol, (LPCTSTR) contract.currency);
        }
    }
}


void IB_PTraderDlg::openOrderEnd ()
{
    Log (GetFuncHeaderForLogging ("openOrderEnd"));
}


IB_PTraderDlg::TWSCodeType IB_PTraderDlg::GetTWSCodeType (const int errorCode)
{
    if (errorCode < 1100)
        return TWS_Error;
    if (errorCode < 2100)
        return TWS_System;
    return TWS_Warning;
}


CString IB_PTraderDlg::GetTWSCodeType_AsString (TWSCodeType twscodetype)
{
    if (twscodetype == TWS_Error)
        return "TWS Error";
    else if (twscodetype == TWS_System)
        return "TWS System";
    else
        return "Warning";
}


void IB_PTraderDlg::winError (const IBString &errorstr, int lastError)
{
    CString str;
    str.Format ("%d", lastError);
    TWSCodeType twscodetype = GetTWSCodeType (lastError);
    CString prefix = GetTWSCodeType_AsString (twscodetype);
    Log (GetFuncHeaderForLogging ("winError") + " (" + str + "), " + prefix);
    OnBnClickedScrollmessages ();
}


void IB_PTraderDlg::connectionClosed ()
{
}


void IB_PTraderDlg::updateAccountValue (const IBString &key, const IBString &val,
                                        const IBString &currency, const IBString &accountName)
{
    CSingleLock crit (&_account_critsection);
    crit.Lock ();
    try
    {
        if (_accountvalues == 0)
            _accountvalues = new std::map <std::string, AccountValue *> ();
        AccountValue *accountvalue;
        std::string hashkey = AccountValue::GetHashKey (key, currency, accountName);
        AccountValuesIter entryiter = _accountvalues->find (hashkey);
        if (entryiter != _accountvalues->end ())
        {
            accountvalue = entryiter->second;
        }
        else
        {
            accountvalue = new AccountValue ();
            (*_accountvalues) [hashkey] = accountvalue;
        }
        accountvalue->Initialise ((const char*) accountName, (const char*) key, (const char*) val, (const char*) currency);
        if (currency == "BASE")
        {
            if (key == "NetLiquidationByCurrency")
            {
                double accountliquidvalue = accountvalue->GetAdjustedCashValue ();
                if (accountliquidvalue < _minaccountvaluebalance)
                    _minaccountvaluebalance = accountliquidvalue;
                if (accountliquidvalue > _maxaccountvaluebalance)
                    _maxaccountvaluebalance = accountliquidvalue;
                SetWindowTitle ();
            }
            else if (key == "UnrealizedPnL")
            {
                double uPnL = accountvalue->GetAdjustedCashValue ();
                if (uPnL < _minPnL)
                    _minPnL = uPnL;
                if (accountvalue->_numberhits == 1)
                    _maxPnL = uPnL;                    
                else if (uPnL > _maxPnL)
                    _maxPnL = uPnL;
                SetWindowTitle ();
             }
        }
    }
    catch (std::exception &e) 
    {
        Log ("updateAccountValue - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("updateAccountValue", e.what (), "", "");
    }
    catch (...)
    {
        Log ("updateAccountValue - exception thrown");
        _stats.IncrementStat ("updateAccountValue", "exception ...", "", "");
    }
    crit.Unlock ();
}


void IB_PTraderDlg::updatePortfolio (const Contract &contract, int position,
                                     double marketPrice, double marketValue, double averageCost,
                                     double unrealizedPNL, double realizedPNL, const IBString &accountName)
{
    CSingleLock crit (&_portfolio_critsection);
    crit.Lock ();
    try
    {
        if (_portfolio == 0)
            _portfolio = new std::map <std::string, std::map <std::string, PortfolioPart *> *> ();

        // Get Portfolio for account
        std::string hashkey = accountName;
        AccountPortfolioPartsIter entryiter = _portfolio->find (hashkey);
        std::map <std::string, PortfolioPart *> *accountportfolio;
        if (entryiter != _portfolio->end ())
        {
            accountportfolio = entryiter->second;
        }
        else
        {
            accountportfolio = new std::map <std::string, PortfolioPart *> ();
            (*_portfolio) [hashkey] = accountportfolio;
        }
        
        // Set Portfolio part
        hashkey = PortfolioPart::GetHashKey (const_cast <Contract &> (contract));
        PortfolioPart *portfoliopart;
        PortfolioPartsIter portpartiter = accountportfolio->find (hashkey);
        if (portpartiter != accountportfolio->end ())
        {
            portfoliopart = portpartiter->second;
            portfoliopart->Initialise (const_cast <Contract &> (contract), position, marketPrice, marketValue, 
                                       averageCost, unrealizedPNL, realizedPNL);
        }
        else
        {
            portfoliopart = new PortfolioPart (const_cast <Contract &> (contract), (const char*) accountName, position, 
                                               marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL);
            (*accountportfolio) [hashkey] = portfoliopart;
            _pclient->reqContractDetails (_contractREQID++, contract);
        }
    }
    catch (std::exception &e) 
    {
        Log ("updatePortfolio - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("updatePortfolio", e.what (), "", "");
    }
    catch (...)
    {
        Log ("updatePortfolio - exception thrown");
        _stats.IncrementStat ("updatePortfolio", "exception ...", "", "");
    }
    crit.Unlock ();
}


void IB_PTraderDlg::updateAccountTime (const IBString &timeStamp)
{
}


void IB_PTraderDlg::accountDownloadEnd (const IBString &accountName)
{
}


void IB_PTraderDlg::nextValidId (OrderId orderId)
{
    _currentorderid = orderId;
    CString str;
    str.Format (GetFuncHeaderForLogging ("nextValidId") + " - Current order Id is %ld", orderId);
    Log (str);
}


void IB_PTraderDlg::contractDetails (int reqId, const ContractDetails& contractDetails)
{
    PortfolioPart *part = GetPortfolioPart (const_cast <Contract &> (contractDetails.summary));
    if (part)
        part->_longname = contractDetails.longName;
}


void IB_PTraderDlg::bondContractDetails (int reqId, const ContractDetails& contractDetails)
{
}


void IB_PTraderDlg::contractDetailsEnd (int reqId)
{
}


void IB_PTraderDlg::execDetails (int reqId, const Contract& contract, const Execution& execution)
{
}


void IB_PTraderDlg::execDetailsEnd (int reqId)
{
}


void IB_PTraderDlg::error (const int id, const int errorCode, const IBString errorString)
{
    CString str;
    TWSCodeType twscodetype = GetTWSCodeType (errorCode);
    CString prefix = GetTWSCodeType_AsString (twscodetype);
    str.Format (" id=%i, code=%i:-", id, errorCode);
    Log (GetFuncHeaderForLogging ("error") + " (const int id, const int errorCode, const IBString errorString), " + prefix + str);
    Log (errorString);
    str.Format ("%i_%i_%s", id, errorCode, (LPCTSTR) errorString);
    // TWS error codes are:-
    // 201 - Order rejected - Reason
    // 202 - Order cancelled - Reason
    // 110 - The price does not conform to the minimum price variation for this contract
    // 200 - No security definition has been found for the request
    if (errorCode == 201 || errorCode == 202 || errorCode == 110)
    {
        OutStandingOrder *outstandingorder = _ordermaps.GetOutStandingOrder (id);
        if (outstandingorder)
        {
            _stats.IncrementStat ("error", (LPCTSTR) str, outstandingorder->_ticker->_ticker, outstandingorder->_ticker->_firstdbrow->_currency);
            if (errorCode == 110)
            {
                CancelOrder (outstandingorder, id);
                switch (outstandingorder->GetOrderType ())
                {
                    case OutStandingOrder::BUY:
                        _stats.IncrementStat ("error", (LPCTSTR) (str + "_Buy"), outstandingorder->_ticker->_ticker, outstandingorder->_ticker->_firstdbrow->_currency);
                        break;
                    case OutStandingOrder::SELL:
                        _stats.IncrementStat ("error", (LPCTSTR) (str + "_Sell"), outstandingorder->_ticker->_ticker, outstandingorder->_ticker->_firstdbrow->_currency);
                        break;
                }
            }
            return;
        }
    }
    else if (errorCode == 200)
    {
        Ticker *ticker = _tickers ? _tickers->GetTicker (id) : 0;
        if (ticker)
        {
            str.Format ("Security is %s (%s)", ticker->_ticker.c_str (), ticker->_share->_primaryexchange.c_str ());
            Log (str);
        }
        else
        {
            Log ("Could not find security definition");
        }
    }
    _stats.IncrementStat ("error", (LPCTSTR) str, "", "");
    OnBnClickedScrollmessages ();
}


void IB_PTraderDlg::error (const IBString errorString)
{
    Log (GetFuncHeaderForLogging ("error") + " (const IBString errorString), TWS Error:-");
    Log (errorString);
    OnBnClickedScrollmessages ();
}


void IB_PTraderDlg::updateMktDepth (TickerId id, int position, int operation, int side, 
                                    double price, int size)
{
}


void IB_PTraderDlg::updateMktDepthL2 (TickerId id, int position, IBString marketMaker, int operation, 
                                      int side, double price, int size)
{
}


void IB_PTraderDlg::updateNewsBulletin (int msgId, int msgType, const IBString& newsMessage, const IBString& originExch)
{
}


void IB_PTraderDlg::managedAccounts (const IBString& accountsList)
{
}


void IB_PTraderDlg::receiveFA (faDataType pFaDataType, const IBString& cxml)
{
}


void IB_PTraderDlg::historicalData (TickerId reqId, const IBString& date, double open, double high, double low,
                                    double close, int volume, int barCount, double WAP, int hasGaps)
{
}


void IB_PTraderDlg::scannerParameters (const IBString &xml)
{
}


void IB_PTraderDlg::scannerData (int reqId, int rank, const ContractDetails &contractDetails, const IBString &distance,
                                 const IBString &benchmark, const IBString &projection, const IBString &legsStr)
{
}


void IB_PTraderDlg::scannerDataEnd (int reqId)
{
}


void IB_PTraderDlg::realtimeBar (TickerId reqId, long time, double open, double high, double low, double close,
                                 long volume, double wap, int count)
{
}


void IB_PTraderDlg::currentTime (long time)
{
}


void IB_PTraderDlg::fundamentalData (TickerId reqId, const IBString& data)
{
}


void IB_PTraderDlg::deltaNeutralValidation (int reqId, const UnderComp& underComp)
{
}


void IB_PTraderDlg::tickSnapshotEnd (int reqId)
{
}


void IB_PTraderDlg::marketDataType (TickerId reqId, int marketDataType)
{
}


void IB_PTraderDlg::commissionReport (const CommissionReport &commissionReport)
{
}


void IB_PTraderDlg::position (const IBString &account, const Contract &contract, int position, double avgCost)
{
}


void IB_PTraderDlg::positionEnd ()
{
}


void IB_PTraderDlg::accountSummary (int reqId, const IBString &account, const IBString &tag, const IBString &value, const IBString &curency)
{
}


void IB_PTraderDlg::accountSummaryEnd (int reqId)
{
}

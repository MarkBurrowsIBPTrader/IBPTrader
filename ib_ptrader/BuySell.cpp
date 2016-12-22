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
#include <math.h>



#pragma region "Spreads"
double IB_PTraderDlg::GetExchangeSpread (Ticker *ticker, bool &spreadcalculated)
{
    double price = ticker->_currentoffer;
    if (ticker->_firstdbrow->_currency == "USD")
    {
        spreadcalculated = true;
        if (ticker->_firstdbrow->_primaryexchange == "OTCBB" ||
            ticker->_firstdbrow->_primaryexchange == "PINK.LIMITED" ||
            ticker->_firstdbrow->_primaryexchange == "PINK.CURRENT")
        {
            if (price > 1)
            {
                return 0.01;
            }
            else if (price > 0.1)
            {
                return 0.005;
            }
            else if (price > 0.005)
            {
                return 0.0005;
            }
            else
            {
                return 0.0001;
            }
        }
        else
        {
            return 0.01;
        }
    }
    else if (ticker->_firstdbrow->_currency == "CAD")
    {
        spreadcalculated = true;
        if (price > 0.5)
        {
            return 0.01;
        }
        else
        {
            return 0.005;
        }
    }
    else if (ticker->_firstdbrow->_currency == "GBP")
    {
        spreadcalculated = true;
        return 1;
    }
    spreadcalculated = false;
    return 0;
}


std::string IB_PTraderDlg::GetKeyForAllowedCustomSpreads (std::string ticker, std::string exchange)
{
    return ticker + "_" + exchange;
}


std::string IB_PTraderDlg::GetKeyForAllowedCustomSpreads (Ticker *ticker)
{
    return GetKeyForAllowedCustomSpreads (ticker->_ticker, ticker->_firstdbrow->_primaryexchange);
}


void IB_PTraderDlg::AddCustomSpread (std::string ticker, std::string exchange, double allowedspread)
{
    std::string key = GetKeyForAllowedCustomSpreads (ticker, exchange);
    _custom_allowed_spreads [key] = allowedspread;    
}


double IB_PTraderDlg::GetSpreadForTicker (Ticker *ticker)
{
    std::string key = GetKeyForAllowedCustomSpreads (ticker);
    CustomSpreadIter iter = _custom_allowed_spreads.find (key);
    if (iter != _custom_allowed_spreads.end ())
    {
        return iter->second;
    }
    if (ticker->_firstdbrow->_currency == "GBP")
    {
        return 5;
    }
    bool spreadcalculated;
    double exchspread = GetExchangeSpread (ticker, spreadcalculated);
    if (spreadcalculated)
    {
        if (exchspread >= 0.01)
        {
            return 0.05;
        }
        else if (ticker->_firstdbrow->_currency == "CAD")
        {
            return 0.01;
        }
        else
        {
            return 0.005;
        }
    }
    _stats.IncrementStat ("GetSpreadForTicker", "NoSpreadCalculated", ticker->_ticker, ticker->_firstdbrow->_currency);
    return 0.01;
}


bool IB_PTraderDlg::SpreadAllowed (Ticker *ticker, double currentspread)
{
    double spreadforticker = GetSpreadForTicker (ticker);
    return currentspread <= spreadforticker;
}
#pragma endregion


#pragma region "Helpers"
bool IB_PTraderDlg::LogError (BuySellResult result)
{
    switch (result)
    {
        case LackOfFunds:
        case MinOIBreached:
        case MaxOIExceeded:
        case MaxCashExceeded:
        case PortfolioPart_NotFound:
        case MaxCashDayLimitExceeded:
        case MaxGlobalCurrencyCashExceeded:
            return false;
    }
    return true;
}


void IB_PTraderDlg::LogBuySellError (CString buyorsell, BuySellResult result)
{
    if (LogError (result))
    {
        Log (buyorsell + CString (" failed with error: ") + ToString (result));
    }
}


IBString IB_PTraderDlg::GetPrimaryExchangeForContract (std::string &exchange)
{
    if (exchange == "NASDAQ.NMS")
    {
        return "AMEX";
    }
    else
    {
        return exchange.c_str ();
    }
}


void IB_PTraderDlg::MakeContract (Ticker *ticker, Contract &contract, double strike)
{
    contract.conId = 0;
    contract.symbol = ticker->_ticker.c_str ();
    contract.secType = ticker->_firstdbrow->_sectype.c_str ();
    contract.expiry = _T ("");
    contract.strike = strike;
    contract.right = _T ("");
    contract.multiplier = _T ("");
    contract.exchange = ticker->_firstdbrow->_exchange.c_str ();
    contract.primaryExchange = GetPrimaryExchangeForContract (ticker->_firstdbrow->_primaryexchange);
    contract.currency = ticker->_firstdbrow->_currency.c_str ();
    contract.localSymbol = _T ("");
    contract.includeExpired = false;
    contract.secIdType = ticker->_firstdbrow->_secidtype.c_str ();
    contract.secId = ticker->_firstdbrow->_secid.c_str ();
}


double IB_PTraderDlg::RoundBidOffer (Ticker *ticker, double value)
{
    if (ticker->_firstdbrow->_currency == "GBP")
    {
        double fractpart, intpart;
        fractpart = ::modf (value, &intpart);     
        if (fractpart > 0.5)
            intpart += 1;
        return intpart;
    }
    return value;
}
#pragma endregion


#pragma region "PlaceOrder"
void IB_PTraderDlg::PlaceOrder (Contract &contract, Order &order)
{
    _pclient->placeOrder (order.orderId, contract, order);
    _currentorderid++;
}
#pragma endregion


#pragma region "AllowTrade"
bool IB_PTraderDlg::AllowTrade (OutStandingOrder::OrderType ordertype, Ticker *ticker, std::string statstext)
{
    MarketDetails::MarketStatus marketstatus = ticker->_marketstatus;
    switch (marketstatus)
    {
        case MarketDetails::PreMarket:
        case MarketDetails::EarlyOpen:
        case MarketDetails::Open:
        {
            MarketDetails *marketdetails = GetMarketDetails (ticker);
            CTime now = CTime::GetCurrentTime ();
            ticker->_marketstatus = MarketDetails::GetMarketStatus (marketdetails, now);            
            if (ticker->_marketstatus == MarketDetails::EarlyOpen && ordertype == OutStandingOrder::BUY)
            {
                bool stoptrade = true;
                if (ticker->_allow_early_buys)
                {
                    int pendingbuys, pendingsells;
                    OutStandingOrder::GetPendingBuysSells (ticker, pendingbuys, pendingsells);  
                    stoptrade = pendingsells > 0;   
                    if (stoptrade)
                    {
                        _stats.IncrementStat (statstext + "_AllowTrade", "EarlyOpenBuyRejected_TickerAllows_PendingSells", ticker->_ticker, ticker->_firstdbrow->_currency);
                    }
                    else
                    {
                        _stats.IncrementStat (statstext + "_AllowTrade", "EarlyOpenBuyAllowed_TickerAllowsEarlyBuys", ticker->_ticker, ticker->_firstdbrow->_currency);
                    }
                }
                if (stoptrade)
                {
                    _stats.IncrementStat (statstext + "_AllowTrade", "EarlyOpenBuyRejected", ticker->_ticker, ticker->_firstdbrow->_currency);
                    return false;
                }
            }
            return MarketDetails::IsMarketOpen (ticker->_marketstatus);
        }
        case MarketDetails::AfterHours:
        case MarketDetails::OnHoliday:
        {
            return false;
        }
    }
    return false;
}
#pragma endregion

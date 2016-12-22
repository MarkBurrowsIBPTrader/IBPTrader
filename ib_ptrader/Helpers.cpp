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
#include "Ticker.h"



void IB_PTraderDlg::InvalidateTickerDisplay (Ticker *ticker)
{
    _mainlistbox.Invalidate (0);
}


CString IB_PTraderDlg::GetTickTypeDisplay (TickType tickType) 
{
    switch (tickType)
    {
        case BID_SIZE:                      return "bidSize";
        case BID:                           return "bidPrice";
        case ASK:                           return "askPrice";
        case ASK_SIZE:                      return "askSize";
        case LAST:                          return "lastPrice";
        case LAST_SIZE:                     return "lastSize";
        case HIGH:                          return "high";
        case LOW:                           return "low";
        case VOLUME:                        return "volume";
        case CLOSE:                         return "close";
        case BID_OPTION_COMPUTATION:        return "bidOptComp";
        case ASK_OPTION_COMPUTATION:        return "askOptComp";
        case LAST_OPTION_COMPUTATION:       return "lastOptComp";
        case MODEL_OPTION:                  return "optionModel";
        case OPEN:                          return "open";
        case LOW_13_WEEK:                   return "13WeekLow";
        case HIGH_13_WEEK:                  return "13WeekHigh";
        case LOW_26_WEEK:                   return "26WeekLow";
        case HIGH_26_WEEK:                  return "26WeekHigh";
        case LOW_52_WEEK:                   return "52WeekLow";
        case HIGH_52_WEEK:                  return "52WeekHigh";
        case AVG_VOLUME:                    return "AvgVolume";
        case OPEN_INTEREST:                 return "OpenInterest";
        case OPTION_HISTORICAL_VOL:         return "OptionHistoricalVolatility";
        case OPTION_IMPLIED_VOL:            return "OptionImpliedVolatility";
        case OPTION_BID_EXCH:               return "OptionBidExchStr";
        case OPTION_ASK_EXCH:               return "OptionAskExchStr";
        case OPTION_CALL_OPEN_INTEREST:     return "OptionCallOpenInterest";
        case OPTION_PUT_OPEN_INTEREST:      return "OptionPutOpenInterest";
        case OPTION_CALL_VOLUME:            return "OptionCallVolume";
        case OPTION_PUT_VOLUME:             return "OptionPutVolume";
        case INDEX_FUTURE_PREMIUM:          return "IndexFuturePremium";
        case BID_EXCH:                      return "bidExch";
        case ASK_EXCH:                      return "askExch";
        case AUCTION_VOLUME:                return "auctionVolume";
        case AUCTION_PRICE:                 return "auctionPrice";
        case AUCTION_IMBALANCE:             return "auctionImbalance";
        case MARK_PRICE:                    return "markPrice";
        case BID_EFP_COMPUTATION:           return "bidEFP";
        case ASK_EFP_COMPUTATION:           return "askEFP";
        case LAST_EFP_COMPUTATION:          return "lastEFP";
        case OPEN_EFP_COMPUTATION:          return "openEFP";
        case HIGH_EFP_COMPUTATION:          return "highEFP";
        case LOW_EFP_COMPUTATION:           return "lowEFP";
        case CLOSE_EFP_COMPUTATION:         return "closeEFP";
        case LAST_TIMESTAMP:                return "lastTimestamp";
        case SHORTABLE:                     return "shortable";
        case FUNDAMENTAL_RATIOS:            return "fundamentals";
        case RT_VOLUME:                     return "RTVolume";
        case HALTED:                        return "halted";
        case BID_YIELD:                     return "bidYield";
        case ASK_YIELD:                     return "askYield";
        case LAST_YIELD:                    return "lastYield";
        case CUST_OPTION_COMPUTATION:       return "custOptComp";
        default:                            return "unknown";
    }
}


IB_PTraderDlg::PortfolioPart *IB_PTraderDlg::GetPortfolioPart (std::string &hashkey)
{
    PortfolioPart *part = 0;
    CSingleLock crit (&_portfolio_critsection);
    crit.Lock ();
    try
    {
        if (_portfolio)
        {
            AccountPortfolioPartsIter entryiter = _portfolio->find (_accountnumber);
            if (entryiter != _portfolio->end ())
            {
                std::map <std::string, PortfolioPart *> *accountportfolio = entryiter->second;
                PortfolioPartsIter portpartiter = accountportfolio->find (hashkey);
                if (portpartiter != accountportfolio->end ())
                {
                    part = portpartiter->second;
                }
            }
        }
    }
    catch (std::exception &e) 
    {
        part = 0;
        Log ("GetPortfolioPart - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("GetPortfolioPart", e.what (), "", "");
    }
    catch (...)
    {
        part = 0;
        Log ("GetPortfolioPart - exception thrown");
        _stats.IncrementStat ("GetPortfolioPart", "exception ...", "", "");
    }
    crit.Unlock ();
    return part;
}


IB_PTraderDlg::PortfolioPart *IB_PTraderDlg::GetPortfolioPart (Contract &contract)
{
    std::string hashkey = PortfolioPart::GetHashKey (const_cast <Contract &> (contract));
    return GetPortfolioPart (hashkey);
}


IB_PTraderDlg::PortfolioPart *IB_PTraderDlg::GetPortfolioPart (Ticker *ticker)
{
    std::string hashkey = PortfolioPart::GetHashKey (ticker);
    return GetPortfolioPart (hashkey);
}


IB_PTraderDlg::AccountValue *IB_PTraderDlg::GetAccountValue (std::string key, std::string currency)
{
    AccountValue *result = 0;
    CSingleLock crit (&_account_critsection);
    crit.Lock ();
    try
    {
        if (_accountvalues)
        {
            std::string hashkey = AccountValue::GetHashKey (key.c_str (), currency.c_str (), _accountnumber.c_str ());
            AccountValuesIter entryiter = _accountvalues->find (hashkey);
            if (entryiter != _accountvalues->end ())
            {
                result = entryiter->second;
            }
        }
    }
    catch (std::exception &e) 
    {
        result = 0;
        Log ("GetAccountValue - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("GetAccountValue", e.what (), "", "");
    }
    catch (...)
    {
        result = 0;
        Log ("GetAccountValue - exception thrown");
        _stats.IncrementStat ("GetAccountValue", "exception ...", "", "");
    }
    crit.Unlock ();
    return result;
}


IB_PTraderDlg::AccountValue *IB_PTraderDlg::GetCashBalance (std::string &currency)
{
    return GetAccountValue ("CashBalance", currency);
}


CString IB_PTraderDlg::GetFileName (CString filename)
{
    CString fullname = _currentworkingdir + "\\" + filename;
    if (_username.length () > 0)
    {
        fullname += "_";
        fullname += _username.c_str ();
    }
    else
    {
        fullname += "_NA";
    }
    fullname += ".txt";
    Log ("FileName " + fullname + " requested");
    _stats.IncrementStat ("GetFileName", (LPCTSTR) fullname, "", "");
    return fullname;
}


#pragma region "Active Tickers"
std::string IB_PTraderDlg::GetTickerEntryKey (std::string ticker, std::string exchange)
{
    return ticker + "_" + exchange;
}


std::string IB_PTraderDlg::GetTickerEntryKey (Ticker *ticker)
{
    return GetTickerEntryKey (ticker->_ticker, ticker->_firstdbrow->_primaryexchange);
}


void IB_PTraderDlg::AddGlobalInActiveTickers (std::list <std::string> &tickerstoignore, Ticker::ActiveState state)
{
    if (_tickers)
    {
        for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
        {
            Ticker *ticker = iter->second;
            if (ticker->_activestate != Ticker::DISABLED)
            {
                bool matched = false;
                for each (std::string excep in tickerstoignore)
                {
                    if (excep == ticker->_ticker)
                    {
                        matched = true;
                        break;
                    }
                }
                if (! matched)
                    ticker->_activestate = state;
            }
        }
    }
}


void IB_PTraderDlg::AddInActiveTicker (std::string ticker, std::string exchange, Ticker::ActiveState state)
{
    std::string key = GetTickerEntryKey (ticker, exchange);
    _inactivetickers [key] = state;
}


void IB_PTraderDlg::SetTickerActive (Ticker *ticker)
{
    std::string key = GetTickerEntryKey (ticker);
    TickersEntryIter iter = _inactivetickers.find (key);
    if (iter != _inactivetickers.end ())
    {
        ticker->_activestate = iter->second;
    }
}


void IB_PTraderDlg::AddAllowEarlyBuyTicker (std::string ticker, std::string exchange)
{
    std::string key = GetTickerEntryKey (ticker, exchange);
    _allow_earlyopen_buy_tickers [key] = true;
    if (_allow_earlyopen_buy_tickers.size () == 1)
    {
        Log ("Shares allowed early open:-");
    }
    std::string message = ticker + std::string (" (") + exchange + std::string (")");
    Log (message.c_str ());
}


void IB_PTraderDlg::SetAllowEarlyBuyTicker (Ticker *ticker)
{
    std::string key = GetTickerEntryKey (ticker);
    EarlyBuysTickersEntryIter iter = _allow_earlyopen_buy_tickers.find (key);
    if (iter != _allow_earlyopen_buy_tickers.end ())
    {
        ticker->_allow_early_buys = iter->second;
    }
}


void IB_PTraderDlg::SetBuyBelowAverage (std::string ticker, std::string primaryexch, Ticker::BuyAverages buyaverages)
{
    std::string hashkey = Share::GetHashKey (ticker, primaryexch, "STK");
    Share *share = GetShare (0, hashkey);
    if (share && share->_tickerptr)
    {
        share->_tickerptr->_buyaverages = buyaverages;
    }
}


void IB_PTraderDlg::SetAllBuyBelowAverage (std::string currency)
{
    if (_shares)
    {
        for (SharesIter iter = _shares->begin ();  iter != _shares->end ();  ++iter)
        {
            Share *share = iter->second;
            if (share->_tickerptr && share->_tickerptr->_firstdbrow->_currency == currency)
            {
                share->_tickerptr->_buyaverages = Ticker::OnlyIfBelow;
            }
        }
    }
}


void IB_PTraderDlg::SetUserDefinedTickerAverage (std::string ticker, std::string exchange, double average)
{
    std::string hashkey = Share::GetHashKey (ticker, exchange, "STK");
    Share *share = GetShare (0, hashkey);
    if (share && share->_tickerptr)
    {
        share->_tickerptr->_userdefined_avg = average;
    }
}


void IB_PTraderDlg::SetUserDefinedTickerAverage (Ticker * ticker)
{
    if (ticker)
    {
        int start, end;
        ticker->_buys.GetStartEndDataRange (start, end);      
        double runningtotal = 0;      
        long numberbought = 0;
        for (int i = start;  i <= end;  i++)
        {
            BuyPricePair *buyprice = ticker->_buys.GetData (i);
            if (buyprice->GetStatus () == PricePair::BOUGHT)
            {
                double cost = buyprice->GetCost ();
                runningtotal += cost;
                numberbought += buyprice->_size; 
            }
        }           
        if (numberbought > 0)
        {
            double average = runningtotal / (double) numberbought;
            SetUserDefinedTickerAverage (ticker->_share->_ticker, ticker->_share->_primaryexchange, average);
        }
    }
}


void IB_PTraderDlg::SetUserDefinedTickerAverages ()
{
    if (_shares)
    {
        for (SharesIter iter = _shares->begin ();  iter != _shares->end ();  ++iter)
        {
            Share *share = iter->second;
            SetUserDefinedTickerAverage (share->_tickerptr);
        }
    }
}


Ticker *IB_PTraderDlg::GetSelectedTicker ()
{
    int cursel = _mainlistbox.GetCurSel ();
    if (cursel != LB_ERR)
    {
        Ticker *ticker = (Ticker *) _mainlistbox.GetItemData (cursel);
        return ticker->_nullticker ? 0 : ticker;
    }
    return 0;
}


void IB_PTraderDlg::SetTickersBuyOnOff (Ticker *ticker, Ticker::OnOffOptions onoffoptions, bool wantbuysellcheck)
{
    CString format;
    format.Format ("On/Off buys set to %s for %s", Ticker::ToDisplayString (onoffoptions).c_str (),
                   ticker->_ticker.c_str ());
    Log (format);
    ticker->SetOnOffBuyOptions (onoffoptions);
    if (wantbuysellcheck)
        Set_DoBuys_DoSells_OnTickerStateChange (ticker);
}


void IB_PTraderDlg::SetTickersBuyOnOff (std::string ticker, std::string exchange, Ticker::OnOffOptions onoffoptions)
{
    Ticker *foundticker = _tickers ? _tickers->GetTickerByContract (ticker, exchange, "STK") : 0;
    if (foundticker)
    {
        SetTickersBuyOnOff (foundticker, onoffoptions, false);
    }
}
#pragma endregion

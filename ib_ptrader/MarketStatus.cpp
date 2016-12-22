/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "MarketStatus.h"



void MarketDetails::SetOnHoliday (bool onholiday)
{
    _onholiday = onholiday;
}


namespace
{
    const int EARLY_OPEN_TIME_IN_MINS = 5;
}


CTime &MarketDetails::GetEarlyOpenTime ()
{
    if (! _earlytimeset)
    {
        _earlytimeset = true;
        CTime now = CTime::GetCurrentTime ();
        CTime starttime (now.GetYear (), now.GetMonth (), now.GetDay (), _start_hour, _start_minute, 0);
        CTimeSpan timespan (0, 0, max (EARLY_OPEN_TIME_IN_MINS, 0), 0);
        _earlyopentime = starttime + timespan;
        CString format;
        format.Format ("Early open time for %s set to %0.2d:%0.2d", _currency.c_str (), _earlyopentime.GetHour (), _earlyopentime.GetMinute ());
        _parent->Log (format);
        _parent->GetStats ().IncrementStat ("GetEarlyOpenTime", (LPCTSTR) format, "", _currency);
    }
    return _earlyopentime;
}


std::string MarketDetails::GetKey (MarketDetails *marketdetails)
{
    return marketdetails->_currency;
}


std::string MarketDetails::GetKey (Ticker *ticker)
{
    if (ticker->FirstDBRow ())
        return ticker->FirstDBRow ()->_currency;
    return "";
}


MarketDetails::MarketStatus MarketDetails::GetMarketStatus (MarketDetails *marketdetails, CTime &time)
{
    if (marketdetails == 0 || marketdetails->_onholiday)
    {
        return OnHoliday;
    }
    int dayofweek = time.GetDayOfWeek ();
    if (dayofweek == 1 || dayofweek == 7)
    {
        return OnHoliday;
    }
    int hour = time.GetHour ();
    if (hour < marketdetails->_start_hour)
    {
        return PreMarket;
    }

    int minute = time.GetMinute ();
    if (hour == marketdetails->_start_hour)
    {
        if (minute < marketdetails->_start_minute)
        {
            return PreMarket;
        }
        if (marketdetails->_earlytimeexceeded)
        {
            return Open;
        }
        CTime &earlyopentime = marketdetails->GetEarlyOpenTime ();
        if (time < earlyopentime)
        {
            return EarlyOpen;
        }
        marketdetails->_earlytimeexceeded = true;
        return Open;
    }
    else
    {
        if (hour > marketdetails->_finish_hour)
        {
            return AfterHours;
        }
        else if (hour == marketdetails->_finish_hour)
        {
            if (minute <= marketdetails->_finish_minute)
            {
                return Open;
            }
            else
            {
                return AfterHours;
            }   
        }
        else
        {
            if (marketdetails->_earlytimeexceeded)
            {
                return Open;
            }
            CTime &earlyopentime = marketdetails->GetEarlyOpenTime ();
            if (time < earlyopentime)
            {
                return EarlyOpen;
            }
            marketdetails->_earlytimeexceeded = true;
            return Open;
        }
    }
}


MarketDetails::MarketDetails (IB_PTraderDlg *parent, MarketDetails::Market market, std::string currency, 
                              int starthour, int startminute, int finishhour, int finishminute) : _parent (parent), _market (market), _currency (currency), 
                                                                                                  _start_hour (starthour), _start_minute (startminute), 
                                                                                                  _finish_hour (finishhour), _finish_minute (finishminute), _onholiday (false),
                                                                                                  _earlytimeset (false), _earlytimeexceeded (false)
{
}


MarketDetails *IB_PTraderDlg::GetMarketDetails (std::string key)
{
    if (key != "")
    {
        MarketDetailsIter iter = _marketdetails.find (key);
        if (iter != _marketdetails.end ())
        {
            return iter->second;
        }
    }
    return 0;
}


MarketDetails *IB_PTraderDlg::GetMarketDetails (Ticker *ticker)
{
    std::string key = MarketDetails::GetKey (ticker);
    return GetMarketDetails (key);
}


bool IB_PTraderDlg::IsMarketOpen (std::string currency)
{
    MarketDetails *marketdetails = GetMarketDetails (currency);
    CTime now = CTime::GetCurrentTime ();
    MarketDetails::MarketStatus marketstatus = MarketDetails::GetMarketStatus (marketdetails, now);
    return MarketDetails::IsMarketOpen (marketstatus);
}


bool IB_PTraderDlg::IsMarketOnHoliday (std::string currency)
{
    MarketDetails *marketdetails = GetMarketDetails (currency);
    if (marketdetails == 0 || marketdetails->IsOnHoliday ())
    {
        return true;
    }
    return false;
}


MarketDetails *IB_PTraderDlg::AddMarketDetails (MarketDetails::Market market, std::string currency, int starthour, int startminute, int finishhour, int finishminute)
{
    MarketDetails *marketdetails = new MarketDetails (this, market, currency, starthour, startminute, finishhour, finishminute);
    std::string key = MarketDetails::GetKey (marketdetails);
    _marketdetails [key] = marketdetails;
    return marketdetails;
}


void IB_PTraderDlg::InitialiseMarketDetails ()
{
    // trade hours in UK time
    // change 1 to 0 depending on time zone differences between UK and other markets
#if 1
    AddMarketDetails (MarketDetails::US, "USD", 14, 30, 21, 00)->SetOnHoliday (false);
    AddMarketDetails (MarketDetails::CAD, "CAD", 14, 30, 21, 00)->SetOnHoliday (false);
#else
    AddMarketDetails (MarketDetails::US, "USD", 13, 30, 20, 00)->SetOnHoliday (false);
    AddMarketDetails (MarketDetails::CAD, "CAD", 13, 30, 20, 00)->SetOnHoliday (false);
#endif

#if 1
    AddMarketDetails (MarketDetails::UK, "GBP", 8, 0, 16, 30)->SetOnHoliday (false);
#else
    AddMarketDetails (MarketDetails::UK, "GBP", 8, 0, 16, 30)->SetOnHoliday (true);
#endif
    AddMarketDetails (MarketDetails::AUD, "AUD", 24, 0, 6, 0)->SetOnHoliday (false);
}

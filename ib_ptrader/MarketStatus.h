/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once



class Ticker;
class IB_PTraderDlg;


class MarketDetails
{
public:
    enum Market
    {
        US, CAD, UK, AUD
    };

    enum MarketStatus
    {
        PreMarket, EarlyOpen, Open, AfterHours, OnHoliday
    };

    static std::string ToDisplayString (MarketStatus status)
    {
        switch (status)
        {   
            case PreMarket: { return "Pre"; }
            case EarlyOpen: { return "Early"; }
            case Open: { return "Open"; }
            case AfterHours: { return "After"; }
            case OnHoliday: { return "ON HOLIDAY"; }
        }
        return "???";
    }

    static bool IsMarketOpen (MarketStatus status)
    {
        return status == Open || status == EarlyOpen;
    }

private:
    IB_PTraderDlg *_parent;
    Market _market;
    std::string _currency;

    int _start_hour;
    int _start_minute;

    int _finish_hour;
    int _finish_minute;

    bool _onholiday;

    bool _earlytimeset;
    bool _earlytimeexceeded;
    CTime _earlyopentime;

public:
    inline bool IsOnHoliday () { return _onholiday; }
    void SetOnHoliday (bool onholiday);
    CTime &GetEarlyOpenTime ();

    static std::string GetKey (MarketDetails *marketdetails);
    static std::string GetKey (Ticker *ticker);
    static MarketStatus GetMarketStatus (MarketDetails *marketdetails, CTime &time);
    
    MarketDetails (IB_PTraderDlg *parent, MarketDetails::Market market, std::string currency, 
                   int starthour, int startminute, int finishhour, int finishminute);    
};


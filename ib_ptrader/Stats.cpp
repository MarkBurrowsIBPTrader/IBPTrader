/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "Stats.h"



Stats::Stats (IB_PTraderDlg *parent) : _parent (parent)
{
}


class MoneyStats
{
public:
    std::string _currency;
    double _profits;
    double _totalbuycosts;
    double _salestotal;
    int _numberofbuys;
    int _numberofsells;
    Ticker *_lastbuytimeticker;

    MoneyStats (std::string currency) : _currency (currency), _profits (0), _totalbuycosts (0), _salestotal (0), _numberofbuys (0), _numberofsells (0)
    {
    }

    typedef std::map <std::string, MoneyStats *>::iterator MoneyStatsIter;
    static std::map <std::string, MoneyStats *> _moneystats; // <currency, MoneyStats *>

    static MoneyStats *GetMoneyStats (std::string currency)
    {
        MoneyStatsIter iter = _moneystats.find (currency);
        if (iter == _moneystats.end ())
        {
            MoneyStats *moneystats = new MoneyStats (currency);
            _moneystats [currency] = moneystats;
            return moneystats;
        }
        return iter->second;
    }

    static void ClearUp ()
    {
        for (MoneyStatsIter iter = _moneystats.begin ();  iter != _moneystats.end ();  ++iter)
        {
            delete iter->second;
        }
        _moneystats.clear ();
    }
};


std::map <std::string, MoneyStats *> MoneyStats::_moneystats;


void Stats::GetCurrencies (CStringList &stringlist)
{
    for (MoneyStats::MoneyStatsIter iter = MoneyStats::_moneystats.begin ();  iter != MoneyStats::_moneystats.end ();  ++iter)
    {
        MoneyStats *moneystats = iter->second;
        double divider;
        std::string currencysymbol;
        if (moneystats->_currency == "GBP")
        {
            divider = 100;
            currencysymbol = "£";
        }
        else
        {
            divider = 1;
            currencysymbol = "$";
        }
        IB_PTraderDlg::GlobalCurrencyBuy *maxcurrencybuys = _parent->GetGlobalCurrencyBuy (moneystats->_currency);
        CString str;
        str.Format ("Current Total Profits %s %s%0.2f (%s%0.2f (%0.0fK) buys, %s%0.2f (%0.0fK) sales), buycount=%d, sellcount=%d, MaxAllowedBuys=%s%g (%0.2f)", 
                    moneystats->_currency.c_str (), 
                    currencysymbol.c_str (), (moneystats->_profits / divider), 
                    currencysymbol.c_str (), (moneystats->_totalbuycosts / divider), (moneystats->_totalbuycosts / divider) / 1000,
                    currencysymbol.c_str (), (moneystats->_salestotal / divider), (moneystats->_salestotal / divider) / 1000,
                    moneystats->_numberofbuys, moneystats->_numberofsells, currencysymbol.c_str (), maxcurrencybuys->_maxallowedbuys, maxcurrencybuys->_currenttotal);
        if (moneystats->_numberofbuys > 0 && moneystats->_lastbuytimeticker)
        {
            str += ", last buy at ";
            CString format;
            format.Format ("%0.2d:%0.2d:%0.2d (%s)", 
                           moneystats->_lastbuytimeticker->_lastbuytime.GetHour (), 
                           moneystats->_lastbuytimeticker->_lastbuytime.GetMinute (), 
                           moneystats->_lastbuytimeticker->_lastbuytime.GetSecond (),
                           moneystats->_lastbuytimeticker->_ticker.c_str ());
            str += format;
        }
        stringlist.AddTail (str);
    }
}


void Stats::Dump (CFile *file, CStringList &stringlist)
{
    POSITION pos = stringlist.GetHeadPosition ();
    while (pos != NULL)
    {
        CString &str = stringlist.GetAt (pos);
        _parent->DumpLineToFile (IB_PTraderDlg::None, file, str);
        _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
        stringlist.GetNext (pos);
    }
}


void Stats::Dump (CFile *file)
{
    CString str;
    _parent->DumpLineToFile (IB_PTraderDlg::None, file, "----------------------------------------");
    _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
    if (_parent->_tickers == 0)
    {
        _parent->DumpLineToFile (IB_PTraderDlg::None, file, "No Ticker stats available");
        _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
    }
    else
    {
        for (TickersIter iter = _parent->_tickers->GetTickers ()->begin ();  iter != _parent->_tickers->GetTickers ()->end ();  ++iter)
        {
            Ticker *ticker = iter->second;
            MoneyStats *moneystats = MoneyStats::GetMoneyStats (ticker->_firstdbrow->_currency);
            if (ticker->_numberofbuys > 0)
            {
                if (moneystats->_numberofbuys == 0)
                {   
                    moneystats->_lastbuytimeticker = ticker;
                }
                else
                {
                    if (ticker->_lastbuytime > moneystats->_lastbuytimeticker->_lastbuytime)
                        moneystats->_lastbuytimeticker = ticker;
                }
                moneystats->_totalbuycosts += ticker->_totalbuycost;
                moneystats->_numberofbuys += ticker->_numberofbuys;
            }
            if (ticker->_numberofsells > 0)
            {
                moneystats->_profits += ticker->_netprofits;
                moneystats->_salestotal += ticker->_salestotal;
                moneystats->_numberofsells += ticker->_numberofsells;
            }
        }
        _parent->DumpLineToFile (IB_PTraderDlg::None, file, "Stats:-");
        _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
        CStringList currencies;
        GetCurrencies (currencies);
        Dump (file, currencies);
        for (StatsIter iter = _stats.begin ();  iter != _stats.end ();  ++iter)
        {
            Stat *stat = iter->second;
            str.Format (" (%0.2d:%0.2d:%0.2d)", stat->_lasttimestamp.GetHour (), stat->_lasttimestamp.GetMinute (), stat->_lasttimestamp.GetSecond ());
            _parent->DumpLineToFile (IB_PTraderDlg::None, file, (iter->first + std::string (" = ")).c_str (), stat->_count, (LPCTSTR) str);
            _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
        }
        Dump (file, currencies);
    }

    const double DIVIDER = 1;

    IB_PTraderDlg::AccountValue *currentaccountvalue = _parent->GetAccountValue ("NetLiquidationByCurrency", "BASE");
    str.Format ("CurrentAccountValue=£%0.0f, MinAccountValue=£%0.0f, MaxAccountValue=£%0.0f", 
                currentaccountvalue ? currentaccountvalue->GetAdjustedCashValue () / DIVIDER : -1,
                _parent->_minaccountvaluebalance / DIVIDER, _parent->_maxaccountvaluebalance / DIVIDER);
    _parent->DumpLineToFile (IB_PTraderDlg::None, file, str);
    _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);

    currentaccountvalue = _parent->GetAccountValue ("UnrealizedPnL", "BASE");
    str.Format ("CurrentPnL=£%0.0f, MinPnL=£%0.0f, MaxPnL=£%0.0f", 
                currentaccountvalue ? currentaccountvalue->GetAdjustedCashValue () / DIVIDER : -1,
                _parent->_minPnL / DIVIDER, _parent->_maxPnL / DIVIDER);
    _parent->DumpLineToFile (IB_PTraderDlg::None, file, str);
    _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);

    int numberofbuys, numberofsells;
    _parent->_ordermaps.GetBuySellCounts (numberofbuys, numberofsells);
    str.Format ("%d buys orders active, %d sell orders active, current orderid is %ld, list box items %d", numberofbuys, numberofsells, _parent->_currentorderid, _parent->_statusmessageslistbox.GetCount () + 1);
    _parent->DumpLineToFile (IB_PTraderDlg::None, file, str);
    _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);

    if (_parent->_tickers)
    {
        for (TickersIter iter = _parent->_tickers->GetTickers ()->begin ();  iter != _parent->_tickers->GetTickers ()->end ();  ++iter)
        {
            Ticker *ticker = iter->second;
            if (ticker->_share)
            {
                IB_PTraderDlg::PortfolioPart *part = _parent->GetPortfolioPart (ticker);
                if (part)
                {
                    long sharecount = part->_position - ticker->_share->_currentoi;
                    if (sharecount < 0)
                    {
                        str.Format ("%s (%s) has a negative share count of %ld, Holdings=%d, OI=%ld", ticker->_ticker.c_str (), 
                                    (LPCTSTR) Ticker::ToString (ticker->_activestate),
                                    sharecount, part->_position, ticker->_share->_currentoi);
                        _parent->DumpLineToFile (IB_PTraderDlg::None, file, str);
                        _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
                    }
                }
            }
        }
    }

    MoneyStats::ClearUp ();
    _parent->OnBnClickedScrollmessages ();
}


std::string Stats::GetKeyForTickerStats (std::string ticker, std::string currency)
{
    return ticker + std::string ("_") + currency;
}


bool Stats::CompareStats (const Stat *d1, const Stat *d2)
{
    return d1->_lasttimestamp > d2->_lasttimestamp;
}


void Stats::DumpStatsForTicker (Ticker *ticker, CFile *file)
{
    CString str;
    _parent->DumpLineToFile (IB_PTraderDlg::None, file, "----------------------------------------");
    _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
    if (ticker)
    {
        std::string key = GetKeyForTickerStats (ticker->_ticker, ticker->_firstdbrow->_currency);
        StatsPerTickerIter statstickeriter = _statsperticker.find (key);
        std::list <Stat *> *tickerstats;
        if (statstickeriter != _statsperticker.end ())
        {
            str.Format ("Ticker stats for %s", ticker->_ticker.c_str ());
            _parent->DumpLineToFile (IB_PTraderDlg::None, file, str);
            _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
            tickerstats = statstickeriter->second;
            tickerstats->sort (CompareStats);
            for (StatIter iter = tickerstats->begin ();  iter != tickerstats->end ();  ++iter)
            {
                Stat *stat = *iter;
                str.Format (" (%0.2d:%0.2d:%0.2d)", stat->_lasttimestamp.GetHour (), stat->_lasttimestamp.GetMinute (), stat->_lasttimestamp.GetSecond ());
                _parent->DumpLineToFile (IB_PTraderDlg::None, file, (stat->_function + std::string ("_") + stat->_statname + std::string (" = ")).c_str (), stat->_count, (LPCTSTR) str);
                _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
            }
        }
        else
        {
            str.Format ("No Ticker stats for %s found", ticker->_ticker.c_str ());
            _parent->DumpLineToFile (IB_PTraderDlg::None, file, str);
            _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
        }
        if (ticker->_technicalindicators)
        {
            CString message;
            bool fp = true;
            for each (TechnicalIndicator *ti in ticker->_technicalindicators->TechnicalIndicators ())
            {
                if (fp)
                {
                    fp = false;
                }
                else
                {
                    message += ", ";
                }
                str.Format ("%s=%d items", ti->Name ().c_str (), (int) ti->Results ().size ());
                message += str;
            }
            _parent->DumpLineToFile (IB_PTraderDlg::None, file, message);
            _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
        }
    }
    else
    {
        _parent->DumpLineToFile (IB_PTraderDlg::None, file, "No ticker found");
        _parent->DumpNewLineToFile (IB_PTraderDlg::None, file);
    }
    _parent->OnBnClickedScrollmessages ();
}


unsigned long Stats::IncrementStat (std::string function, std::string statname, std::string ticker, std::string currency)
{
    std::string key = function + "_" + statname;
    if (ticker.length () != 0)
    {
        key += std::string ("_") + ticker;
    }
    if (currency.length () != 0)
    {
        key += std::string ("_") + currency;
    }
    bool added;
    Stat *stat;
    StatsIter iter = _stats.find (key);
    if (iter == _stats.end ())
    {
        stat = new Stat ();     
        stat->_function = function;
        stat->_statname = statname;
        stat->_count = 1;
        _stats [key] = stat;
        added = true;
    }
    else
    {
        stat = iter->second;
        stat->_count++;
        added = false;
    }
    stat->_lasttimestamp = CTime::GetCurrentTime ();
    if (added && ticker.length () != 0 && currency.length () != 0)
    {
        key = GetKeyForTickerStats (ticker, currency);
        StatsPerTickerIter statstickeriter = _statsperticker.find (key);
        std::list <Stat *> *tickerstats;
        if (statstickeriter == _statsperticker.end ())
        {
            tickerstats = new std::list <Stat *> ();
            _statsperticker [key] = tickerstats;
        }
        else
        {
            tickerstats = statstickeriter->second;
        }
        tickerstats->push_back (stat);
    }
    return stat->_count;
}


unsigned long Stats::IncrementStat (std::string function, const wchar_t *message, std::string ticker, std::string currency)
{
    char *charmess = xercesc::XMLString::transcode (message);
    unsigned long result = IncrementStat (function, charmess, ticker, currency);
    xercesc::XMLString::release (&charmess);    
    return result;
}

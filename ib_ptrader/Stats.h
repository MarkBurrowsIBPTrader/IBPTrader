/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include <map>
#include <list>



class IB_PTraderDlg;
class Ticker;


class Stat
{
private:
    std::string _function;
    std::string _statname;
    unsigned long _count;
    CTime _lasttimestamp;

    friend class Stats;
};


class Stats
{
private:
    IB_PTraderDlg *_parent;

    typedef std::map <std::string, Stat *>::iterator StatsIter;
    std::map <std::string, Stat *> _stats; // <function+statname+ticker+currency, Stat *>

    typedef std::map <std::string, std::list <Stat *> *>::iterator StatsPerTickerIter;
    typedef std::list <Stat *>::iterator StatIter;
    std::map <std::string, std::list <Stat *> *> _statsperticker; // <ticker+currency, Stat *>

    std::string GetKeyForTickerStats (std::string ticker, std::string currency);

    void GetCurrencies (CStringList &stringlist);    

    static bool CompareStats (const Stat *d1, const Stat *d2);

public:
    void Dump (CFile *file, CStringList &stringlist);
    void Dump (CFile *file);
    void DumpStatsForTicker (Ticker *ticker, CFile *file);
    unsigned long IncrementStat (std::string function, std::string statname, std::string ticker, std::string currency);
    unsigned long IncrementStat (std::string function, const wchar_t *message, std::string ticker, std::string currency);

    Stats (IB_PTraderDlg *parent);
};

/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "Ticker.h"




Ticker *Tickers::GetTickerByContract (std::string hashkey)
{
    TickersByContractIter iter = _tickers_by_contract->find (hashkey);
    if (iter != _tickers_by_contract->end ())
        return iter->second;
    return 0;
}


Ticker *Tickers::GetTickerByContract (std::string ticker, std::string primaryexch, std::string sectype)
{
    std::string hashkey = ticker + "_" + sectype + "_" + primaryexch;
    return GetTickerByContract (hashkey);
}


void Tickers::AddTickerByContract (Ticker *ticker)
{
   std::string hashkey = IB_PTraderDlg::PortfolioPart::GetHashKey (ticker);
   Ticker *testticker = GetTickerByContract (hashkey);
   if (testticker == 0)
        (*_tickers_by_contract) [hashkey] = ticker;
}


Ticker *Tickers::AddTicker (TickerId tickerid, std::string ticker, PGenDatabaseRows::PGenDatabaseRow *dbrow)
{
    if (_tickers == 0)
        return 0;
    TickersIter entryiter = _tickers->find (tickerid);
    if (entryiter == _tickers->end ())
    {
        Ticker *tickerptr = new Ticker (tickerid, ticker, dbrow);
        (*_tickers) [tickerid] = tickerptr;
        AddTickerByContract (tickerptr);
        return tickerptr;
    }
    return entryiter->second;
}


Tickers::Tickers (PGenDatabaseRows *initialisation, IB_PTraderDlg *parent)
{
    _tickers = new std::map <TickerId, Ticker *> ();
    _tickers_by_contract = new std::map <std::string, Ticker *> ();
    if (initialisation)
    {
        for (PGenDatabaseRows::DBRowsIter iter = initialisation->_dbrows.begin ();  iter != initialisation->_dbrows.end ();  ++iter)
        {
            TickerId tickerid = iter->second->_tickerid;
            Ticker *ticker = AddTicker (tickerid, iter->second->_ticker, iter->second);
            if (ticker)
                ticker->AddPricePair (iter->second, parent);
        }
    }
}


Tickers::~Tickers ()
{
}

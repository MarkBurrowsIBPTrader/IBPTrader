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



#pragma region "TickerStats"
void TickerStats::Reset ()
{
    _numberactive = 0;
    _numberpending = 0;
    _numberbought = 0;
}


CString TickerStats::GetLogMessage (CString prefix)
{
    CString str;
    str.Format (": Active %ld, Pending %ld, Bought %ld", _numberactive, _numberpending, _numberbought);
    return prefix + str;
}


TickerStats::TickerStats ()
{
    Reset ();
}


void Ticker::UpdateTickerStats (TickerStats &stats, PricePair *pricepair)
{
    switch (pricepair->GetStatus ())
    {
        case PricePair::ACTIVE:
            stats._numberactive++;
            break;
        case PricePair::PENDING:
            stats._numberpending++;
            break;
        case PricePair::BOUGHT:
            stats._numberbought++;
            break;
    }
}


void Ticker::GetTickerStats_Buy (TickerStats &stats)
{
    int size = _buys.GetDataCount ();
    for (int i = 0;  i < size;  i++)
    {
        BuyPricePair *buypricepair = _buys.GetData (i);
        UpdateTickerStats (stats, buypricepair);
    }
}


void Ticker::GetTickerStats_Sell (TickerStats &stats)
{
    int size = _sells.GetDataCount ();
    for (int i = 0;  i < size;  i++)
    {
        SellPricePair *sellprice = _sells.GetData (i);
        UpdateTickerStats (stats, sellprice);
    }
}
#pragma endregion


#pragma region "Ticker"
void Ticker::SetOnOffBuyOptions (OnOffOptions onoffoptions)
{
    int size = _buys.GetDataCount ();
    if (onoffoptions == One_In_Three || onoffoptions == One_In_Four)
    {
        int countdownLIMIT = onoffoptions == One_In_Three ? 3 : 4;
        int countdown = countdownLIMIT;
        for (int i = 0;  i < size;  i++)
        {
            BuyPricePair *buyprice = _buys.GetData (i);
            countdown--;
            if (countdown == 0)
            {
                countdown = countdownLIMIT;
                buyprice->_turnedon = true;
            }
            else
            {
                buyprice->_turnedon = false;
            }
        }    
    }
    else
    {
        bool flag;
        switch (onoffoptions)
        {
            case All_On:
                flag = true;
                break;
            case All_Off:
                flag = false;
                break;
            case Off_Even:
                flag = false;
                break;
            case Off_Odd:
            default:
                flag = true;
                break;
        }
        for (int i = 0;  i < size;  i++)
        {
            BuyPricePair *buyprice = _buys.GetData (i);
            buyprice->_turnedon = flag;
            switch (onoffoptions)
            {
                case Off_Even:
                case Off_Odd:
                    flag = ! flag;
                    break;
            }
        }
    }
}


Ticker *Ticker::GetTicker (Ticker *ticker, Share *share)
{
    if (ticker)
        return ticker;
    return share->_tickerptr;
}


Ticker::Ticker (TickerId tickerid, std::string ticker, PGenDatabaseRows::PGenDatabaseRow *dbrow) : _ticker (ticker),
                                                                                                   _activestate (Active),
                                                                                                   _tickerid (tickerid), 
                                                                                                   _nullticker (false),
                                                                                                   _tickerhasnoprices (false),
                                                                                                   _share (0),
                                                                                                   _allow_early_buys (false),
                                                                                                   _pricerecordtype (NoRecording),
                                                                                                   _firstdbrow (dbrow),
                                                                                                   _searchstartmatchindex (-1),
                                                                                                   _lastbuytime (1970, 1, 1, 0, 0, 0, 0),
                                                                                                   _sellsstartpoint (0),
                                                                                                   _sellstartpoint_startindex (-1),
                                                                                                   _sellstartpoint_endindex (-1),
                                                                                                   _currentbid (0), 
                                                                                                   _minbid (DBL_MAX), _maxbid (-1),
                                                                                                   _bidsize (0),
                                                                                                   _minmaxes_set_bid (false),
                                                                                                   _currentoffer (0), 
                                                                                                   _minoffer (DBL_MAX), _maxoffer (-1),
                                                                                                   _offersize (0),
                                                                                                   _minmaxes_set_offer (false),
                                                                                                   _tickvolume_bid (0), _tickvolume_offer (0),
                                                                                                   _tickdepth_bid (0), _tickdepth_offer (0),
                                                                                                   _ib_low (DBL_MAX), _ib_high (-1), _ib_open (-1),
                                                                                                   _numberofbuys (0), 
                                                                                                   _totalbuycost (0),
                                                                                                   _numberofsells (0),
                                                                                                   _salestotal (0),
                                                                                                   _netprofits (0),
                                                                                                   _number_of_cancelledbuys (0), _number_of_cancelledsells (0),
                                                                                                   _volume (0),
                                                                                                   _avgvolume (0),
                                                                                                   _maxglobalcurrencycashexceeded (false),
                                                                                                   _maxcashexceeded (false),
                                                                                                   _maxoiexceeded (false),
                                                                                                   _maxdailybuycashexceeded (false),
                                                                                                   _maxdailysellcashexceeded (false),
                                                                                                   _maxtrendcount (0),
                                                                                                   _buyaverages (Always),
                                                                                                   _userdefined_avg (-1),
#if ! PUBLIC_BUILD
                                                                                                   _sellsearchspeed (Slow),
#endif
                                                                                                   _makebatchbuy (false),
                                                                                                   _autoliquidate_on (false),
                                                                                                   _autoliquidate_DONE (false),
                                                                                                   _liquidationtype (NoLiquidation),
                                                                                                   _currencysymbolset (false),
                                                                                                   _livepriceset (0),
                                                                                                   _technicalindicators (0),
                                                                                                   _ignoretechnicalindicators (false)
{
}


BuyPricePair *Ticker::AddPricePair (PGenDatabaseRows::PGenDatabaseRow *dbrow, IB_PTraderDlg *parent)
{
    long oi = dbrow->_oi;
    double price = dbrow->_buyprice;
    long size = dbrow->_numberbuyshares;
    BuyPricePair *buypricepair = new BuyPricePair (this, PricePair::ACTIVE, price, size);
    buypricepair->_dbrow = dbrow;
    buypricepair->_oi = oi;
    _buys.Add (buypricepair, price);

    price = dbrow->_sellprice;
    size = dbrow->_numbersellshares;
    SellPricePair *sellpricepair = new SellPricePair (PricePair::ACTIVE, price, size);
    sellpricepair->_buypricepair = buypricepair;
    buypricepair->_sellpricepair = sellpricepair;
    BuyPricePair::Status buystatus = oi > 0 ? PricePair::BOUGHT : PricePair::ACTIVE;
    buypricepair->SetStatus (buystatus);

    if (buystatus == PricePair::BOUGHT)
    {
        bool validdate;
        CTime &lastbuydate = dbrow->GetLastBuyDate (validdate);
        if (validdate)
        {
            if (dbrow->_lastavgbuyfill > 0 && dbrow->_lastbuycommission > 0)
            {
                buypricepair->_lastbuysset = true;
                CTime now = CTime::GetCurrentTime ();
                buypricepair->_boughttoday = now.GetDay () == lastbuydate.GetDay () &&
                                             now.GetMonth () == lastbuydate.GetMonth () &&
                                             now.GetYear () == lastbuydate.GetYear ();
                buypricepair->_lastavgfill = dbrow->_lastavgbuyfill;
                buypricepair->_lastbuycommission = dbrow->_lastbuycommission;
            }
        }
    }

    _sells.Add (sellpricepair, price);

    if (dbrow->_currency == "USD")
        _volumemultiplier = 100;
    else
        _volumemultiplier = 1;

    if (_share == 0)
    {
        std::string sharehashkey = Share::GetHashKey (dbrow);
        _share = parent->GetShare (0, sharehashkey);
        if (_share)
        {
            _share->_tickerptr = this;
        }
        else
        {
            CString message;
            message.Format ("Ticker::AddPricePair could not tie up share %s", sharehashkey.c_str ());
            parent->Log (message);
            parent->GetStats ().IncrementStat ("Ticker::AddPricePair", (LPCTSTR) message, "", "");
        }
    }
    parent->IncrementShareCurrentOI (0, _share, buypricepair->_oi, 0);

    parent->GetOrderMaps ().Initialise_BuyPrice (buypricepair);

    return buypricepair;
}


void Ticker::DetermineBranches ()
{
    _buys.DetermineBranches ();
    _sells.DetermineBranches ();
}


Ticker::~Ticker ()
{
}
#pragma endregion

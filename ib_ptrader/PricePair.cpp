/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"



#pragma region "PricePair"
BuyPricePair *PricePair::FindLowestSellPriceStartPoint (Ticker *ticker, int start)
{
    int finish = ticker->_sells.GetDataCount () - 1;
    for (int i = start;  i <= finish;  i++)
    {
        SellPricePair *searchpricepair = ticker->_sells.GetData (i);
        if (searchpricepair->_buypricepair->GetStatus () == PricePair::BOUGHT && 
            searchpricepair->GetStatus () == PricePair::ACTIVE)
        {
            return searchpricepair->_buypricepair;
        }
    }
    return 0;
}


PricePair::PricePair (Status status, double price, long size) : _status (status), _price (price), _size (size), _pairindex (-1)
{
}
#pragma endregion


#pragma region "BuyPricePair"
void BuyPricePair::SetStatus (Status status)
{
    if (status == PricePair::BOUGHT)
    {
        if (_ticker->_sellsstartpoint)
        {
            if (_sellpricepair->_price < _ticker->_sellsstartpoint->_sellpricepair->_price)
            {
                _ticker->_sellsstartpoint = this;
            }
        }
        else
        {
            _ticker->_sellsstartpoint = this;
        }
        _sellpricepair->_trending_hitcount = 0;
        _sellpricepair->_min_trend_over = 0;
        _sellpricepair->_max_trend_over = 0;
        _sellpricepair->_actual_sellgap = 0;
    }
    else if (status == PricePair::CANCELLED)
    {
        status = PricePair::ACTIVE;
    }
    _status = status;
}


bool BuyPricePair::AvailableForLiquidation (bool todayonly)
{
    if (_lastbuysset && GetStatus () == BOUGHT && _sellpricepair->GetStatus () == ACTIVE)
    {
        if (todayonly)
            return _boughttoday;
        return true;
    }
    return false;
}


BuyPricePair::BuyPricePair (Ticker *ticker, Status status, double price, long size) : PricePair (status, price, size), _ticker (ticker), _lastbuysset (false),
                                                                                      _boughttoday (false), _lastavgfill (0), _lastbuycommission (0),
                                                                                      _turnedon (true)
{
}
#pragma endregion


#pragma region "SellPricePair"
void SellPricePair::SetStatus (Status status)
{
    bool gotopending = _status == PricePair::ACTIVE && status == PricePair::PENDING;
    bool gotobought = _status == PricePair::PENDING && status == PricePair::BOUGHT;
    bool cancelled = status == PricePair::CANCELLED;
    if (gotopending)
    {
        if (_buypricepair == _buypricepair->_ticker->_sellsstartpoint)
        {
            _buypricepair->_ticker->_sellsstartpoint = FindLowestSellPriceStartPoint (_buypricepair->_ticker, 
                                                                                      _buypricepair->_ticker->_sellsstartpoint->_sellpricepair->_pairindex + 1);
        }
    }
    else if (cancelled)
    {
        if (_buypricepair->_ticker->_sellsstartpoint == 0 || _price < _buypricepair->_ticker->_sellsstartpoint->_sellpricepair->_price)
        {
            _buypricepair->_ticker->_sellsstartpoint = _buypricepair;
        }
        status = PricePair::ACTIVE;
    }
    else if (gotobought)
    {
        status = PricePair::ACTIVE;
    }
    _status = status;
}


void SellPricePair::FormatTrendForDisplay (CString &format)
{
    format.Format (" (%0.2f %0.2f A=%0.2f)", _min_trend_over, _max_trend_over, _actual_sellgap);
}


SellPricePair::SellPricePair (Status status, double price, long size) : PricePair (status, price, size)
{
}
#pragma endregion
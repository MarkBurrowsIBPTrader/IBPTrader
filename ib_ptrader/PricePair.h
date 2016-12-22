/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include "DataTypes.h"



class BuyPricePair;
class Ticker;


class PricePair
{
public:
    enum Status
    {
        ACTIVE, PENDING, BOUGHT, CANCELLED
    };

    static std::string ToString (Status status)
    {
        switch (status)
        {
            case ACTIVE: { return "ACTIVE"; }
            case PENDING: { return "PENDING"; }
            case BOUGHT: { return "BOUGHT"; }
            case CANCELLED: { return "CANCELLED"; }
        }
        return "???";
    }

    static std::string ToShortString (Status status)
    {
        switch (status)
        {
            case ACTIVE: { return "a"; }
            case PENDING: { return "P"; }
            case BOUGHT: { return "B"; }
            case CANCELLED: { return "c"; }
        }
        return "???";
    }

protected:
    Status _status;
    BuyPricePair *FindLowestSellPriceStartPoint (Ticker *ticker, int start);

    // POD. Everyone needs easy access to this data. No need for get/set accessors
public:
    double _price;
    long _size;
    int _pairindex;

public:
    inline double GetCost () { return _price * (double) _size; }
    inline double GetCost (long size) { return _price * (double) size; }
    inline double GetCostFromPrice (double price) { return price * (double) _size; }
    inline double GetCostPerShare () { return _price; }

    inline Status GetStatus () { return _status; }
    virtual void SetStatus (Status status) = 0;

    PricePair (Status status, double price, long size);
};


class SellPricePair;


class BuyPricePair : public PricePair
{
    // POD. Everyone needs easy access to this data. No need for get/set accessors
public:
    Ticker *_ticker;
    PGenDatabaseRows::PGenDatabaseRow *_dbrow;
    long _oi;
    SellPricePair *_sellpricepair;

    bool _lastbuysset;
    bool _boughttoday;
    double _lastavgfill;
    double _lastbuycommission;
 
    void SetStatus (Status status);
    bool AvailableForLiquidation (bool todayonly);

    bool _turnedon;

    BuyPricePair (Ticker *ticker, Status status, double price, long size);
};


class SellPricePair : public PricePair
{
    // POD. Everyone needs easy access to this data. No need for get/set accessors
public:
    BuyPricePair *_buypricepair;

    #pragma region "Trending"
    int _trending_hitcount;
    double _min_trend_over;
    double _max_trend_over;
    double _actual_sellgap;

    void FormatTrendForDisplay (CString &format);
    #pragma endregion

    void SetStatus (Status status);

    SellPricePair (Status status, double price, long size);
};

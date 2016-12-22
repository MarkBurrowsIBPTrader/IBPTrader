/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include <list>
#include <map>
#include "order.h"
#include "orderstate.h"
#include "DataTypes.h"



class OutStandingOrder
{
public:
    enum OrderType
    {
        BUY, SELL
    };

    static std::string ToString (OrderType ordertype)
    {
        switch (ordertype)
        {
            case BUY: { return "Buy"; }
            case SELL: { return "Sell"; }
        }
        return "???";
    }

    static OrderType FromString (std::string ordertype)
    {
        if (ordertype == "Buy")
            return BUY;
        return SELL;
    }

private:
    class PendingBuySellCounts
    {
    public:
        std::string _ticker;
        int _number_activebuys;
        int _number_activesells;

        PendingBuySellCounts (std::string ticker) : _ticker (ticker), _number_activebuys (0), _number_activesells (0) {}
    };

    typedef std::map <TickerId, PendingBuySellCounts *>::iterator PendingCountsIter;
    static std::map <TickerId, PendingBuySellCounts *> _pendingbuysellcounts;

    static std::string GetBuySellCountKey (Ticker *ticker);
    static PendingBuySellCounts *GetPendingBuysSells (Ticker *ticker);

    OrderType _ordertype;

public:
    static void GetPendingBuysSells (Ticker *ticker, int &pendingbuys, int &pendingsells);
    void SetOrderType (OrderType ordertype);
    inline OrderType GetOrderType () { return _ordertype; }

private:
    Ticker *_ticker;
    OrderId _orderid;
    OrderState _orderstate;
    std::list <BuyPricePair *> _buypricepairs;
    int _batchsize;
    double _buycost;
    bool _timerecorded;
    CTime _recordedtime;

    friend class IB_PTraderDlg;
    friend class OrderMaps;

public:
    static bool IsCommissionValid (double commission);
    bool ValidComissionReceived ();

    OutStandingOrder (Ticker *ticker, int batchsize, double buycost, bool recordtime);
    virtual ~OutStandingOrder ();
};


class IB_PTraderDlg;


class OrderMaps
{
private:
    IB_PTraderDlg *_parent;

    typedef std::map <IdDeals, BuyPricePair *>::iterator IdDealsIter;
    std::map <IdDeals, BuyPricePair *> _iddeals_to_buyprice;

private:
    typedef std::map <OrderId, OutStandingOrder *>::iterator OrderIdIter;
    std::map <OrderId, OutStandingOrder *> _orderid_to_outstandingorder;

    friend class IB_PTraderDlg;

private:
    bool AddOrderToDB (sql::Statement *stmt, OutStandingOrder *outstandingorder, BuyPricePair *buyprice);
    bool AddOrderToDB (sql::Statement *stmt, OutStandingOrder *outstandingorder);

public:
    OutStandingOrder *GetOutStandingOrder (OrderId orderid);
    BuyPricePair *GetBuyPricePair (IdDeals iddeals);

    void ReadOutStandingOrdersFromDB_AndWireUp ();

    bool Initialise_BuyPrice (BuyPricePair *buypricepair);
    bool PlaceBuyOrder (sql::Statement *stmt, OrderId orderid, BuyPricePair *buyprice, int batchsize, double buycost);

    bool RemoveOrderId_FromDB (sql::Statement *stmt, OrderId orderid);
    void RemoveOrderId_FromMemory (OrderId orderid);

    bool PlaceSellOrder (sql::Statement *stmt, OrderId orderid, SellPricePair *sellprice, int batchsize);

    void GetBuySellCounts (int &numberofbuys, int &numberofsells);

    OrderMaps (IB_PTraderDlg *parent);
};

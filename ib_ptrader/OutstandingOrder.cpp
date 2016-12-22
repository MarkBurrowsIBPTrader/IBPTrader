/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "OutstandingOrder.h"
#include "mysql_connection.h"
#include <memory>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>



#pragma region "OutStandingOrder"
bool OutStandingOrder::IsCommissionValid (double commission)
{
    return commission != UNSET_DOUBLE;
}


bool OutStandingOrder::ValidComissionReceived ()
{
    return IsCommissionValid (_orderstate.commission);
}


std::map <TickerId, OutStandingOrder::PendingBuySellCounts *> OutStandingOrder::_pendingbuysellcounts;


OutStandingOrder::PendingBuySellCounts *OutStandingOrder::GetPendingBuysSells (Ticker *ticker)
{
    if (ticker)
    {
        PendingCountsIter iter = _pendingbuysellcounts.find (ticker->TheTickerId ());
        if (iter == _pendingbuysellcounts.end ())
        {
            PendingBuySellCounts *pendingsellcounts = new PendingBuySellCounts (ticker->TickerName ());
            _pendingbuysellcounts [ticker->TheTickerId ()] = pendingsellcounts;
            return pendingsellcounts;
        }
        return iter->second;
    }
    return 0;
}


void OutStandingOrder::GetPendingBuysSells (Ticker *ticker, int &pendingbuys, int &pendingsells)
{
    PendingBuySellCounts *pendingsellcounts = GetPendingBuysSells (ticker);
    if (pendingsellcounts)
    {
        pendingbuys = pendingsellcounts->_number_activebuys;
        pendingsells = pendingsellcounts->_number_activesells;
    }
    else
    {
        pendingbuys = 0;
        pendingsells = 0;
    }
}


void OutStandingOrder::SetOrderType (OrderType ordertype)
{
    _ordertype = ordertype;
    PendingBuySellCounts *pendingsellcounts = GetPendingBuysSells (_ticker);
    if (pendingsellcounts)
    {
        switch (ordertype)
        {
            case BUY:
                pendingsellcounts->_number_activebuys++;
                break;
            case SELL:
                pendingsellcounts->_number_activesells++;
                break;
        }
    }
}


OutStandingOrder::OutStandingOrder (Ticker *ticker, int batchsize, double buycost, bool recordtime) : _ticker (ticker), _batchsize (batchsize), _buycost (buycost), _timerecorded (recordtime)
{
    if (_timerecorded)
        _recordedtime = CTime::GetCurrentTime ();
}


OutStandingOrder::~OutStandingOrder ()
{
    PendingBuySellCounts *pendingsellcounts = GetPendingBuysSells (_ticker);
    if (pendingsellcounts)
    {
        switch (_ordertype)
        {
            case BUY:
                pendingsellcounts->_number_activebuys--;
                break;
            case SELL:
                pendingsellcounts->_number_activesells--;
                break;
        }
    }
}
#pragma endregion


#pragma region "OrderMaps"
OrderMaps::OrderMaps (IB_PTraderDlg *parent) : _parent (parent)
{
}


OutStandingOrder *OrderMaps::GetOutStandingOrder (OrderId orderid)
{
    OrderIdIter iter = _orderid_to_outstandingorder.find (orderid);
    if (iter != _orderid_to_outstandingorder.end ())
    {
        return iter->second;
    }
    return 0;
}


BuyPricePair *OrderMaps::GetBuyPricePair (IdDeals iddeals)
{
    IdDealsIter iter = _iddeals_to_buyprice.find (iddeals);
    if (iter != _iddeals_to_buyprice.end ())
    {
        return iter->second;
    }
    return 0;
}


bool OrderMaps::Initialise_BuyPrice (BuyPricePair *buypricepair)
{
    _iddeals_to_buyprice [buypricepair->_dbrow->_iddeals] = buypricepair;
    return true;
}


void OrderMaps::ReadOutStandingOrdersFromDB_AndWireUp ()
{
    /*
        ibrokers.orders (Table)
        ========================
        idorders         int(10) unsigned PK
        orderid          int(11)
        iddeals          int(10) unsigned
        buyorsell        text
        ticker           text
        price            double
        username         text
        accountnumber    text
        batchsize        int(11)
        createtimestamp  timestamp
    */
    try
    {
        std::auto_ptr <sql::Statement> stmt (_parent->_dbcon->createStatement ());
        std::auto_ptr <sql::ResultSet> resultset (stmt->executeQuery ("select orderid,iddeals,buyorsell,batchsize from orders where username= '" + _parent->_username + 
                                                  std::string ("' and accountnumber = '") + _parent->_accountnumber + std::string ("'")));
        while (resultset->next ()) 
        {
            OrderId orderid = resultset->getInt (1);
            IdDeals iddeals = resultset->getUInt (2);
            sql::SQLString buyorsell = resultset->getString (3);
            int batchsize = resultset->getInt (4);

            BuyPricePair *wire = GetBuyPricePair (iddeals);
            if (wire)
            {
                OutStandingOrder *outstandingorder = GetOutStandingOrder (orderid);
                if (outstandingorder == 0)
                {
                    outstandingorder = new OutStandingOrder (wire->_ticker, batchsize, 0, false);
                    outstandingorder->_orderid = orderid;
                    OutStandingOrder::OrderType ordertype = OutStandingOrder::FromString (buyorsell);
                    outstandingorder->SetOrderType (ordertype);
                    _orderid_to_outstandingorder [orderid] = outstandingorder;
                }
                outstandingorder->_buypricepairs.push_back (wire);
                switch (outstandingorder->GetOrderType ())
                {
                    case OutStandingOrder::BUY:
                        wire->SetStatus (PricePair::PENDING);
                        break;
                    case OutStandingOrder::SELL:
                        wire->_sellpricepair->SetStatus (PricePair::PENDING);
                        break;
                }
            }
            else
            {
                CString format;
                format.Format ("%lu", iddeals);
                _parent->Log ("ReadOutStandingOrdersFromDB_AndWireUp - rewire failed for " + format);
                _parent->_stats.IncrementStat ("ReadOutStandingOrdersFromDB_AndWireUp", "Wire Failure for " + std::string ((LPCTSTR) format), "", "");
            }
        }
    }
    catch (std::exception &e) 
    {
        _parent->Log ("ReadOutStandingOrdersFromDB_AndWireUp - exception thrown");
        _parent->Log (e.what ());
        _parent->_stats.IncrementStat ("ReadOutStandingOrdersFromDB_AndWireUp", e.what (), "", "");
    }
    catch (...)
    {
        _parent->Log ("ReadOutStandingOrdersFromDB_AndWireUp - exception thrown");
        _parent->_stats.IncrementStat ("ReadOutStandingOrdersFromDB_AndWireUp", "exception ...", "", "");
    }
}


bool OrderMaps::AddOrderToDB (sql::Statement *stmt, OutStandingOrder *outstandingorder, BuyPricePair *buyprice)
{
    /*
        ibrokers.orders (Table)
        ========================
        idorders         int(10) unsigned PK
        orderid          int(11)
        iddeals          int(10) unsigned
        buyorsell        text
        ticker           text
        price            double
        username         text
        accountnumber    text
        batchsize        int(11)
        createtimestamp  timestamp
    */
    try
    {
        OutStandingOrder::OrderType ordertype = outstandingorder->GetOrderType ();
        double price = (ordertype == OutStandingOrder::BUY) ? buyprice->_price : buyprice->_sellpricepair->_price;
        CString exec;
        exec.Format ("insert into orders (orderid,iddeals,buyorsell,ticker,price,username,accountnumber,batchsize) VALUES (%ld, %lu, '%s', '%s', %g, '%s', '%s', %i)", 
                     outstandingorder->_orderid, buyprice->_dbrow->_iddeals, OutStandingOrder::ToString (ordertype).c_str (),
                     buyprice->_dbrow->_ticker.c_str (), price,
                     _parent->_username.c_str (), _parent->_accountnumber.c_str (), outstandingorder->_batchsize);
        stmt->executeUpdate ((LPCTSTR) exec);
    }
    catch (std::exception &e) 
    {
        _parent->Log ("AddOrderToDB - exception thrown");
        _parent->Log (e.what ());
        _parent->_stats.IncrementStat ("AddOrderToDB", e.what (), "", "");
        return false;
    }
    catch (...) 
    {
        _parent->Log ("AddOrderToDB - exception thrown");
        _parent->_stats.IncrementStat ("AddOrderToDB", "exception ...", "", "");
        return false;
    }
    return true;
}


bool OrderMaps::AddOrderToDB (sql::Statement *stmt, OutStandingOrder *outstandingorder)
{
    /*
        ibrokers.orders (Table)
        ========================
        idorders         int(10) unsigned PK
        orderid          int(11)
        iddeals          int(10) unsigned
        buyorsell        text
        ticker           text
        price            double
        username         text
        accountnumber    text
        batchsize        int(11)
        createtimestamp  timestamp
    */
    try
    {
        CString exec;
        for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
        {
            if (! AddOrderToDB (stmt, outstandingorder, buyprice))
                return false;
        }
    }
    catch (std::exception &e) 
    {
        _parent->Log ("AddOrderToDB - exception thrown");
        _parent->Log (e.what ());
        _parent->_stats.IncrementStat ("AddOrderToDB", e.what (), "", "");
        return false;
    }
    catch (...) 
    {
        _parent->Log ("AddOrderToDB - exception thrown");
        _parent->_stats.IncrementStat ("AddOrderToDB", "exception ...", "", "");
        return false;
    }
    return true;
}


bool OrderMaps::PlaceBuyOrder (sql::Statement *stmt, OrderId orderid, BuyPricePair *buyprice, int batchsize, double buycost)
{
    OutStandingOrder *outstandingorder = GetOutStandingOrder (orderid);
    bool havecreated = (outstandingorder == 0);
    if (havecreated)
    {            
        outstandingorder = new OutStandingOrder (buyprice->_ticker, batchsize, buycost, true);
        outstandingorder->_orderid = orderid;
        outstandingorder->SetOrderType (OutStandingOrder::BUY);
    }
    if (AddOrderToDB (stmt, outstandingorder, buyprice))
    {
        outstandingorder->_buypricepairs.push_back (buyprice);
        if (havecreated)
        {
            _orderid_to_outstandingorder [orderid] = outstandingorder;
        }
        return true;
    }
    return false;
}


bool OrderMaps::RemoveOrderId_FromDB (sql::Statement *stmt, OrderId orderid)
{
    /*
        ibrokers.orders (Table)
        ========================
        idorders         int(10) unsigned PK
        orderid          int(11)
        iddeals          int(10) unsigned
        buyorsell        text
        ticker           text
        price            double
        username         text
        accountnumber    text
        batchsize        int(11)
        createtimestamp  timestamp
    */
    try
    {
        bool createstatement = (stmt == 0);
        if (createstatement)
            stmt = _parent->_dbcon->createStatement ();
        CString exec;
        exec.Format ("delete from orders where orderid = %ld and username = '%s' and accountnumber = '%s'",
                     orderid, _parent->_username.c_str (), _parent->_accountnumber.c_str ());
        stmt->executeUpdate ((LPCTSTR) exec);
        if (createstatement)
            delete stmt;
    }
    catch (std::exception &e) 
    {
        _parent->Log ("RemoveOrderId_FromDB - exception thrown");
        _parent->Log (e.what ());
        _parent->_stats.IncrementStat ("RemoveOrderId_FromDB", e.what (), "", "");
        return false;
    }
    catch (...) 
    {
        _parent->Log ("RemoveOrderId_FromDB - exception thrown");
        _parent->_stats.IncrementStat ("RemoveOrderId_FromDB", "exception ...", "", "");
        return false;
    }
    return true;
}


void OrderMaps::RemoveOrderId_FromMemory (OrderId orderid)
{
    OutStandingOrder *outstandingid = GetOutStandingOrder (orderid);
    delete outstandingid;
    _orderid_to_outstandingorder.erase (orderid);
}


bool OrderMaps::PlaceSellOrder (sql::Statement *stmt, OrderId orderid, SellPricePair *sellprice, int batchsize)
{
    OutStandingOrder *outstandingorder = GetOutStandingOrder (orderid);
    bool havecreated = (outstandingorder == 0);
    if (havecreated)
    {            
        outstandingorder = new OutStandingOrder (sellprice->_buypricepair->_ticker, batchsize, 0, true);
        outstandingorder->_orderid = orderid;
        outstandingorder->SetOrderType (OutStandingOrder::SELL);
    }
    if (AddOrderToDB (stmt, outstandingorder, sellprice->_buypricepair))
    {
        outstandingorder->_buypricepairs.push_back (sellprice->_buypricepair);
        if (havecreated)
        {
            _orderid_to_outstandingorder [orderid] = outstandingorder;
        }
        return true;
    }
    return false;
}


void OrderMaps::GetBuySellCounts (int &numberofbuys, int &numberofsells)
{
    numberofbuys = 0;
    numberofsells = 0;
    for (OrderIdIter iter = _orderid_to_outstandingorder.begin ();  iter !=  _orderid_to_outstandingorder.end ();  ++iter)
    {
        OutStandingOrder *outstandingorder = iter->second;
        OutStandingOrder::OrderType ordertype = outstandingorder->GetOrderType ();
        if (ordertype == OutStandingOrder::BUY)
        {
            numberofbuys++;
        }
        else if (ordertype == OutStandingOrder::SELL)
        {
            numberofsells++;
        }
    }
}
#pragma endregion

/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "ib_ptraderdlg.h"
#include <memory>



double IB_PTraderDlg::GetCommissionPerTrade (double commission, int numberoftrades)
{
    if (OutStandingOrder::IsCommissionValid (commission))
    {
        return commission / (double) numberoftrades;
    }
    return 0;
}


void IB_PTraderDlg::Record_BuyTrade (OutStandingOrder *outstandingorder, OrderId orderId, const IBString &status, 
                                     int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice,
                                     int clientId, const IBString &whyHeld)
{
    /*
        ibrokers.trades (Table)
        ========================
        idtrades         int(10) unsigned PK
        ticker           text
        buyorsell        text
        fillcount        int(11)
        avgfillprice     double
        lastfillprice    double
        totalvalue       double
        commission       double
        mincommission    double
        maxcommission    double
        commissioncurrency text
        buyidtradesrow   int(10) unsigned
        iddealsrowid     int(10) unsigned
        pgenbuyprice     double
        pgensellprice    double
        grossprofit      double
        netprofit        double
        sectype          text
        exchange         text
        primaryexchange  text
        currency         text
        username         text
        accountnumber    text
        createtimestamp  timestamp
        batchsize        int(11)
        mintrend         double
        maxtrend         double
        actualtrend      double
    */
    try
    {
        int numberoftrades = (int) outstandingorder->_buypricepairs.size ();
        Contract contract;
        MakeContract (outstandingorder->_ticker, contract, avgFillPrice);
        bool roundedup, stampdutyadded;
        double mincom, maxcom;
        if (! CalculateCommission (outstandingorder->GetOrderType (), contract, contract.strike, filled, mincom, maxcom, roundedup, stampdutyadded))
        {
            mincom = 0;
            maxcom = 0;
        }
        for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
        {
            std::auto_ptr <sql::PreparedStatement> pstmt (_dbcon->prepareStatement (
"insert into trades (ticker,buyorsell,fillcount,avgfillprice,lastfillprice,totalvalue,commission,mincommission,maxcommission,commissioncurrency,buyidtradesrow,iddealsrowid,pgenbuyprice,pgensellprice,"
"grossprofit,netprofit,sectype,exchange,primaryexchange,currency,username,accountnumber,batchsize,mintrend,maxtrend,actualtrend) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
                                                                     ));       
            const int BASE = 1;
            pstmt->setString (BASE, buyprice->_dbrow->_ticker);                                                         // ticker
            pstmt->setString (BASE + 1, OutStandingOrder::ToString (outstandingorder->GetOrderType ()));                // buyorsell
            
            pstmt->setInt (BASE + 2, buyprice->_oi);                                                                    // fillcount

            pstmt->setDouble (BASE + 3, avgFillPrice);                                                                  // avgfillprice
            
            pstmt->setDouble (BASE + 4, lastFillPrice);                                                                 // lastfillprice
     
            double totalcost = avgFillPrice * buyprice->_oi;
            pstmt->setDouble (BASE + 5, totalcost);                                                                     // totalvalue

            double commission = GetCommissionPerTrade (outstandingorder->_orderstate.commission, numberoftrades);
            pstmt->setDouble (BASE + 6, commission);                                                                    // commission

            double mincommission = GetCommissionPerTrade (mincom, numberoftrades);
            pstmt->setDouble (BASE + 7, mincommission);                                                                 // mincommission

            double maxcommission = GetCommissionPerTrade (maxcom, numberoftrades);
            pstmt->setDouble (BASE + 8, maxcommission);                                                                 // maxcommission

            pstmt->setString (BASE + 9, (LPCTSTR) outstandingorder->_orderstate.commissionCurrency);                    // commissioncurrency
            pstmt->setUInt (BASE + 10, 0);                                                                              // buyidtradesrow
            pstmt->setUInt (BASE + 11, buyprice->_dbrow->_iddeals);                                                     // iddealsrowid
                     
            pstmt->setDouble (BASE + 12, buyprice->_price);                                                             // pgenbuyprice
            pstmt->setDouble (BASE + 13, buyprice->_sellpricepair->_price);                                             // pgensellprice
            pstmt->setDouble (BASE + 14, 0);                                                                            // grossprofit
            pstmt->setDouble (BASE + 15, 0);                                                                            // netprofit
            pstmt->setString (BASE + 16, buyprice->_dbrow->_sectype);                                                   // sectype
            pstmt->setString (BASE + 17, buyprice->_dbrow->_exchange);                                                  // exchange
            pstmt->setString (BASE + 18, buyprice->_dbrow->_primaryexchange);                                           // primaryexchange
            pstmt->setString (BASE + 19, buyprice->_dbrow->_currency);                                                  // currency
            pstmt->setString (BASE + 20, _username);                                                                    // username
            pstmt->setString (BASE + 21, _accountnumber);                                                               // accountnumber

            pstmt->setInt (BASE + 22, outstandingorder->_batchsize);                                                    // batchsize

            pstmt->setDouble (BASE + 23, 0);                                                                            // mintrend
            pstmt->setDouble (BASE + 24, 0);                                                                            // maxtrend
            pstmt->setDouble (BASE + 25, 0);                                                                            // actualtrend

            pstmt->executeUpdate ();
        }
    }
    catch (std::exception &e) 
    {
        Log ("Record_BuyTrade - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("Record_BuyTrade", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("Record_BuyTrade - exception thrown");
        _stats.IncrementStat ("Record_BuyTrade", "exception ...", "", "");
    }
}


void IB_PTraderDlg::Record_SellTrade (OutStandingOrder *outstandingorder, OrderId orderId, const IBString &status, 
                                      int filled, int remaining, double avgFillPrice, int permId, int parentId, double lastFillPrice,
                                      int clientId, const IBString &whyHeld)
{
    /*
        ibrokers.trades (Table)
        ========================
        idtrades         int(10) unsigned PK
        ticker           text
        buyorsell        text
        fillcount        int(11)
        avgfillprice     double
        lastfillprice    double
        totalvalue       double
        commission       double
        mincommission    double
        maxcommission    double
        commissioncurrency text
        buyidtradesrow   int(10) unsigned
        iddealsrowid     int(10) unsigned
        pgenbuyprice     double
        pgensellprice    double
        grossprofit      double
        netprofit        double
        sectype          text
        exchange         text
        primaryexchange  text
        currency         text
        username         text
        accountnumber    text
        createtimestamp  timestamp
        batchsize        int(11)
        mintrend         double
        maxtrend         double
        actualtrend      double
    */
    try
    {
        CString format;
        int numberoftrades = (int) outstandingorder->_buypricepairs.size ();
        Contract contract;
        MakeContract (outstandingorder->_ticker, contract, avgFillPrice);
        bool roundedup, stampdutyadded;
        double mincom, maxcom;
        if (! CalculateCommission (outstandingorder->GetOrderType (), contract, contract.strike, filled, mincom, maxcom, roundedup, stampdutyadded))
        {
            mincom = 0;
            maxcom = 0;
        }
        for each (BuyPricePair *buyprice in outstandingorder->_buypricepairs)
        {
            format.Format ("%lu", buyprice->_dbrow->_iddeals);
            std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
            std::string sqlquery = std::string (
"select idtrades,iddealsrowid,totalvalue,commission,pgenbuyprice from trades where ticker = '" + buyprice->_dbrow->_ticker + "' and "
"buyorsell = '" + OutStandingOrder::ToString (OutStandingOrder::BUY) + "' and "
"iddealsrowid = " + ((LPCTSTR) format) + " and "
" sectype = '" + buyprice->_dbrow->_sectype + "' and "
" exchange = '" + buyprice->_dbrow->_exchange + "' and "
" primaryexchange = '" + buyprice->_dbrow->_primaryexchange + "' and "
" currency = '" + buyprice->_dbrow->_currency + "' and "
" username = '" + _username + "' and "
" accountnumber = '" + _accountnumber + 
"' order by idtrades desc limit 1"
                                               );   
            std::auto_ptr <sql::ResultSet> resultset (stmt->executeQuery (sqlquery));
            if (resultset->rowsCount () == 1 && resultset->next ())
            {
                uint32_t buytraderowid = resultset->getUInt (1);
                uint32_t iddealsrowid = resultset->getUInt (2);

                if (iddealsrowid != buyprice->_dbrow->_iddeals)
                {
                    Log ("Record_SellTrade FAIL:-");
                    format.Format ("Mismatch buy trade/sell trade IDDEALSROWID (%lu/%lu) with orderid %ld", iddealsrowid, buyprice->_dbrow->_iddeals, outstandingorder->_orderid);
                    Log (format);
                    _stats.IncrementStat ("Record_SellTrade", (LPCTSTR) format, "", "");
                    continue;
                }

                double buy_totalcost = (double) resultset->getDouble (3);
                double buy_commission = (double) resultset->getDouble (4);
                double pgenbuyprice = (double) resultset->getDouble (5);

                std::auto_ptr <sql::PreparedStatement> pstmt (_dbcon->prepareStatement (
"insert into trades (ticker,buyorsell,fillcount,avgfillprice,lastfillprice,totalvalue,commission,mincommission,maxcommission,commissioncurrency,buyidtradesrow,iddealsrowid,pgenbuyprice,pgensellprice,"
"grossprofit,netprofit,sectype,exchange,primaryexchange,currency,username,accountnumber,batchsize,mintrend,maxtrend,actualtrend) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
                                                                         ));     

                const int BASE = 1;
                pstmt->setString (BASE, buyprice->_dbrow->_ticker);                                                         // ticker
                pstmt->setString (BASE + 1, OutStandingOrder::ToString (outstandingorder->GetOrderType ()));                // buyorsell

                int currentfill = buyprice->_oi;
                pstmt->setInt (BASE + 2, currentfill);                                                                      // fillcount

                pstmt->setDouble (BASE + 3, avgFillPrice);                                                                  // avgfillprice
                pstmt->setDouble (BASE + 4, lastFillPrice);                                                                 // lastfillprice

                double sell_totalcost = avgFillPrice * currentfill;
                pstmt->setDouble (BASE + 5, sell_totalcost);                                                                // totalvalue

                double sellcommission = GetCommissionPerTrade (outstandingorder->_orderstate.commission, numberoftrades);
                pstmt->setDouble (BASE + 6, sellcommission);                                                                // commission

                double mincommission = GetCommissionPerTrade (mincom, numberoftrades);
                pstmt->setDouble (BASE + 7, mincommission);                                                                 // mincommission

                double maxcommission = GetCommissionPerTrade (maxcom, numberoftrades);
                pstmt->setDouble (BASE + 8, maxcommission);                                                                 // maxcommission

                pstmt->setString (BASE + 9, (LPCTSTR) outstandingorder->_orderstate.commissionCurrency);                    // commissioncurrency
                pstmt->setUInt (BASE + 10, buytraderowid);                                                                  // buyidtradesrow
                pstmt->setUInt (BASE + 11, iddealsrowid);                                                                   // iddealsrowid
                pstmt->setDouble (BASE + 12, pgenbuyprice);                                                                 // pgenbuyprice
                pstmt->setDouble (BASE + 13, buyprice->_sellpricepair->_price);                                             // pgensellprice

                double gross_profit = sell_totalcost - buy_totalcost;
                double net_profit = (sell_totalcost - buy_totalcost) - sellcommission - buy_commission;

                if (outstandingorder->_ticker)
                    outstandingorder->_ticker->_netprofits += net_profit;

                pstmt->setDouble (BASE + 14, gross_profit);                                                                 // grossprofit

                pstmt->setDouble (BASE + 15, net_profit);                                                                   // netprofit
                pstmt->setString (BASE + 16, buyprice->_dbrow->_sectype);                                                   // sectype
                pstmt->setString (BASE + 17, buyprice->_dbrow->_exchange);                                                  // exchange
                pstmt->setString (BASE + 18, buyprice->_dbrow->_primaryexchange);                                           // primaryexchange
                pstmt->setString (BASE + 19, buyprice->_dbrow->_currency);                                                  // currency
                pstmt->setString (BASE + 20, _username);                                                                    // username
                pstmt->setString (BASE + 21, _accountnumber);                                                               // accountnumber
                                            
                pstmt->setInt (BASE + 22, outstandingorder->_batchsize);                                                    // batchsize

                if (buyprice->_sellpricepair->_trending_hitcount > 0)
                {
                    pstmt->setDouble (BASE + 23, buyprice->_sellpricepair->_min_trend_over);                                // mintrend
                    pstmt->setDouble (BASE + 24, buyprice->_sellpricepair->_max_trend_over);                                // maxtrend
                    pstmt->setDouble (BASE + 25, buyprice->_sellpricepair->_actual_sellgap);                                // actualtrend
                }
                else
                {
                    pstmt->setDouble (BASE + 23, 0);                                                                       // mintrend
                    pstmt->setDouble (BASE + 24, 0);                                                                       // maxtrend
                    pstmt->setDouble (BASE + 25, 0);                                                                       // actualtrend
                }

                pstmt->executeUpdate ();
            }
            else
            {
                Log ("Record_SellTrade FAIL:-");
                format.Format ("Could not locate buy trade for sell with orderid %ld", outstandingorder->_orderid);
                Log (format);
                _stats.IncrementStat ("Record_SellTrade", (LPCTSTR) format, "", "");
            }
        }
    }
    catch (std::exception &e) 
    {
        Log ("Record_SellTrade - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("Record_SellTrade", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("Record_SellTrade - exception thrown");
        _stats.IncrementStat ("Record_SellTrade", "exception ...", "", "");
    }
}

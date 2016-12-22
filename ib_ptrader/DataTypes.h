/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include "BSearch.h"
#include "MarketStatus.h"
#include "CommonDefs.h"
#include <list>
#include <map>



typedef unsigned long IdDeals;


class PGenDatabaseRows
{
public:
    class PGenDatabaseRow
    {
        /*
            ibrokers.deals (Table)
            =======================
            iddeals             int(10) unsigned PK
            tickerid            int(11)
            ticker              text
            oi                  int(11)
            buyprice            double
            numberbuyshares     int(11)
            sellprice           double
            numbersellshares    int(11)
            sectype             text
            exchange            text
            primaryexchange     text
            currency            text
            localsymbol         text
            secidtype           text
            secid               text
            genericticks        text
            displayname         text
            username            text
            accountnumber       text
            hitcount            int(11)
            lastavgfillbuyprice double
            lastbuycommission   double
            lastbuydate         datetime
            updatetimestamp     timestamp
        */

        // POD. Easier to keep public, rather than unnecessary get/set methods.
        // Each instance of this class represents 1 row in the deals table.
        // Everyone else needs easy access to this data.
        public:
            IdDeals _iddeals;
            TickerId _tickerid;
            std::string _ticker;
            long _oi;
            double _buyprice;
            long _numberbuyshares;
            double _sellprice;
            long _numbersellshares;
            std::string _sectype;
            std::string _exchange;
            std::string _primaryexchange;
            std::string _currency;
            std::string _localsymbol;
            std::string _secidtype;
            std::string _secid;
            std::string _genericticks;
            std::string _displayname;
            long _hitcount;
            double _lastavgbuyfill;
            double _lastbuycommission;
            std::string _lastbuydate;

        private:
            bool _haveparsedlastbuydate; 
            bool _parsedlastbuydate_valid; 
            CTime _parsedlastbuydate;    

        public:
            static void InitialiseField (const XMLCh *value, std::string &field);
            static void InitialiseField (const XMLCh *value, long &field);
            static void InitialiseField (const XMLCh *value, double &field);

            std::string GetKey ();
            CTime &GetLastBuyDate (bool &valid);

        public:
            PGenDatabaseRow ();
            PGenDatabaseRow (long oi);
    };

private:
    typedef std::map <std::string, PGenDatabaseRows::PGenDatabaseRow *>::iterator DBRowsIter;
    std::map <std::string, PGenDatabaseRow *> _dbrows; // <rowkey, dbrow>

    bool _deepdelete;

    friend class IB_PTraderDlg;
    friend class Tickers;

public:
    static void Delete (PGenDatabaseRows * &dbrows, bool deepdelete);
    static PGenDatabaseRows *Merge (PGenDatabaseRows *master, PGenDatabaseRows *slave, PGenDatabaseRows *delta);

    int GetNumberOfRows () { return (int) _dbrows.size (); }

    PGenDatabaseRow *AddDBRow (PGenDatabaseRow *dbrow);
    PGenDatabaseRow *AddDBRow (const XMLCh *ticker, const XMLCh *tickerid,
                               const XMLCh *buyprice, const XMLCh *numberbuyshares, 
                               const XMLCh *sellprice, const XMLCh *numbersellshares,
                               const XMLCh *sectype, const XMLCh *exchange, const XMLCh *primaryexchange,
                               const XMLCh *currency, const XMLCh *localsymbol, const XMLCh *secidtype,
                               const XMLCh *secid, const XMLCh *genericticks, const XMLCh *displayname,
                               long hitcount);
    PGenDatabaseRow *AddDBRow (IdDeals iddeals,
                               TickerId tickerid, 
                               std::string ticker,
                               long oi,
                               double buyprice, 
                               long numberbuyshares, 
                               double sellprice, 
                               long numbersellshares,
                               std::string sectype,
                               std::string exchange,
                               std::string primaryexchange,
                               std::string currency,
                               std::string localsymbol,
                               std::string secidtype,
                               std::string secid,
                               std::string genericticks,
                               std::string displayname,
                               long hitcount,
                               double lastavgbuyfill,
                               double lastbuycommission,
                               std::string lastbuydate);

    PGenDatabaseRows ();

private:
    ~PGenDatabaseRows ();
};

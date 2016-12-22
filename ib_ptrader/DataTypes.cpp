/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"



#pragma region "PGenDatabaseRows"
#pragma region "PGenDatabaseRow"
void PGenDatabaseRows::PGenDatabaseRow::InitialiseField (const XMLCh *value, std::string &field)
{
    field = xercesc::XMLString::transcode (value);
}


void PGenDatabaseRows::PGenDatabaseRow::InitialiseField (const XMLCh *value, long &field)
{
    std::string valuefield = xercesc::XMLString::transcode (value); 
    field = ::atol (valuefield.c_str ());
}


void PGenDatabaseRows::PGenDatabaseRow::InitialiseField (const XMLCh *value, double &field)
{
    std::string valuefield = xercesc::XMLString::transcode (value);
    field = ::atof (valuefield.c_str ());
}


std::string PGenDatabaseRows::PGenDatabaseRow::GetKey ()
{
    CString format;
    format.Format ("%ld_%s_%s_%0.2f_%ld_%0.2f_%ld", _tickerid, _ticker.c_str (), _primaryexchange.c_str (), _buyprice, _numberbuyshares, _sellprice, _numbersellshares);
    return (LPCTSTR) format;
}


CTime &PGenDatabaseRows::PGenDatabaseRow::GetLastBuyDate (bool &valid)
{
    if (! _haveparsedlastbuydate)
    {
        std::string datepart = _lastbuydate.substr (0, 10);
        if (datepart.length () == 10)
        {
            std::string yearpart = datepart.substr (0, 4);
            std::string monthpart = datepart.substr (5, 2);
            std::string daypart = datepart.substr (8, 2);

            int year = ::atoi (yearpart.c_str ());
            int month = ::atoi (monthpart.c_str ());
            int day = ::atoi (daypart.c_str ());

            _parsedlastbuydate = CTime (year, month, day, 0, 0, 0);
            _parsedlastbuydate_valid = true;
        }
        else
        { 
            _parsedlastbuydate_valid = false;     
        }
        _haveparsedlastbuydate = true;
    }
    valid = _parsedlastbuydate_valid;
    return _parsedlastbuydate;
}


PGenDatabaseRows::PGenDatabaseRow::PGenDatabaseRow () : _oi (0), _haveparsedlastbuydate (false), _lastbuydate (""),
                                                        _lastavgbuyfill (0), _lastbuycommission (0)
{
}


PGenDatabaseRows::PGenDatabaseRow::PGenDatabaseRow (long oi) : _oi (oi), _haveparsedlastbuydate (false), _lastbuydate (""),
                                                               _lastavgbuyfill (0), _lastbuycommission (0)
{
}
#pragma endregion


#pragma endregion "PGenDatabaseRows"
PGenDatabaseRows::PGenDatabaseRow *PGenDatabaseRows::AddDBRow (PGenDatabaseRow *dbrow)
{
    _dbrows [dbrow->GetKey ()] = dbrow;
    return dbrow;
}


void PGenDatabaseRows::Delete (PGenDatabaseRows * &dbrows, bool deepdelete)
{
    dbrows->_deepdelete = deepdelete;
    delete dbrows;
    dbrows = 0;
}


PGenDatabaseRows *PGenDatabaseRows::Merge (PGenDatabaseRows *master, PGenDatabaseRows *slave, PGenDatabaseRows *delta)
{
    PGenDatabaseRows *result = new PGenDatabaseRows ();
    if (master)
    {
        for (DBRowsIter iter = master->_dbrows.begin ();  iter != master->_dbrows.end ();  ++iter)
        {
            result->AddDBRow (iter->second);
        }
    }
    if (slave)
    {
        for (DBRowsIter iter = slave->_dbrows.begin ();  iter != slave->_dbrows.end ();  ++iter)
        {
            std::string hashkey = iter->second->GetKey ();
            DBRowsIter entryiter = result->_dbrows.find (hashkey);
            if (entryiter == result->_dbrows.end ())
            {
                result->AddDBRow (iter->second);
                delta->AddDBRow (iter->second);
            }
        }
    }
    return result;
}


PGenDatabaseRows::PGenDatabaseRow *PGenDatabaseRows::AddDBRow (const XMLCh *ticker,
                                                               const XMLCh *tickerid,
                                                               const XMLCh *buyprice,
                                                               const XMLCh *numberbuyshares,
                                                               const XMLCh *sellprice,
                                                               const XMLCh *numbersellshares,
                                                               const XMLCh *sectype, 
                                                               const XMLCh *exchange, 
                                                               const XMLCh *primaryexchange,
                                                               const XMLCh *currency, 
                                                               const XMLCh *localsymbol, 
                                                               const XMLCh *secidtype,
                                                               const XMLCh *secid, 
                                                               const XMLCh *genericticks, 
                                                               const XMLCh *displayname,
                                                               long hitcount)
{
    PGenDatabaseRow *dbrow = new PGenDatabaseRow ();

    dbrow->_iddeals = 0;
    dbrow->InitialiseField (ticker, dbrow->_ticker);
    dbrow->InitialiseField (tickerid, dbrow->_tickerid);
    dbrow->InitialiseField (buyprice, dbrow->_buyprice);
    dbrow->InitialiseField (numberbuyshares, dbrow->_numberbuyshares);
    dbrow->InitialiseField (sellprice, dbrow->_sellprice);
    dbrow->InitialiseField (numbersellshares, dbrow->_numbersellshares);
    dbrow->InitialiseField (sectype, dbrow->_sectype);
    dbrow->InitialiseField (exchange, dbrow->_exchange);
    dbrow->InitialiseField (primaryexchange, dbrow->_primaryexchange);
    dbrow->InitialiseField (currency, dbrow->_currency);
    dbrow->InitialiseField (localsymbol, dbrow->_localsymbol);
    dbrow->InitialiseField (secidtype, dbrow->_secidtype);
    dbrow->InitialiseField (secid, dbrow->_secid);
    dbrow->InitialiseField (genericticks, dbrow->_genericticks);
    dbrow->InitialiseField (displayname, dbrow->_displayname);
    dbrow->_hitcount = hitcount;

    return AddDBRow (dbrow);
}


PGenDatabaseRows::PGenDatabaseRow *PGenDatabaseRows::AddDBRow (IdDeals iddeals,
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
                                                               std::string lastbuydate)
{
    PGenDatabaseRow *dbrow = new PGenDatabaseRow (oi);

    dbrow->_iddeals = iddeals;
    dbrow->_tickerid = tickerid;
    dbrow->_ticker = ticker;
    dbrow->_buyprice = buyprice;
    dbrow->_numberbuyshares = numberbuyshares;
    dbrow->_sellprice = sellprice;
    dbrow->_numbersellshares = numbersellshares;
    dbrow->_sectype = sectype;
    dbrow->_exchange = exchange;
    dbrow->_primaryexchange = primaryexchange;
    dbrow->_currency = currency;
    dbrow->_localsymbol = localsymbol;
    dbrow->_secidtype = secidtype;
    dbrow->_secid = secid;
    dbrow->_genericticks = genericticks;
    dbrow->_displayname = displayname;
    dbrow->_hitcount = hitcount;
    dbrow->_lastavgbuyfill = lastavgbuyfill;
    dbrow->_lastbuycommission = lastbuycommission;
    dbrow->_lastbuydate = lastbuydate;

    return AddDBRow (dbrow);
}


PGenDatabaseRows::PGenDatabaseRows () : _deepdelete (true)
{
}


PGenDatabaseRows::~PGenDatabaseRows ()
{
    try
    {
        if (_deepdelete)
        {
            for (PGenDatabaseRows::DBRowsIter iter = _dbrows.begin ();  iter != _dbrows.end ();  ++iter)
            {
                delete iter->second;
            }
        }
    }
    catch (...)
    {
    }
}
#pragma endregion
#pragma endregion
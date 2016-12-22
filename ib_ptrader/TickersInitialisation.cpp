/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "mysql_connection.h"
#include <memory>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>



PGenDatabaseRows *IB_PTraderDlg::ReadDBRows_FromXML ()
{
    PGenDatabaseRows *result = new PGenDatabaseRows ();

    Log ("Loading from XML pgenfiles from DB");
    std::list <std::string> pgenfilenames;
    try
    {
        /*
            ibrokers.pgens (Table)
            =======================
            idpgens          int(10) unsigned PK
            pgenxml          text
            username         text
            accountnumber    text
        */
        std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
        std::string sqlquery = std::string ("select pgenxml from pgens where username= '") + _username + std::string ("' and accountnumber = '") + 
                               _accountnumber + std::string ("'");
        std::auto_ptr <sql::ResultSet> resultset (stmt->executeQuery (sqlquery));
        while (resultset->next ()) 
        {
            sql::SQLString pgenxml = resultset->getString (1);
            pgenfilenames.push_back (pgenxml.asStdString ());
        }
    }
    catch (std::exception &e) 
    {
        Log ("ReadDBRows_FromXML - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("ReadDBRows_FromXML", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("ReadDBRows_FromXML - exception thrown");
        _stats.IncrementStat ("ReadDBRows_FromXML", "exception ...", "", "");
    }

    // XML Parse
    Log ("Parsing XML");
    try 
    {
        xercesc::XMLPlatformUtils::Initialize ();
    }
    catch (const xercesc::XMLException &e) 
    {
        Log ("ReadDBRows_FromXML - exception thrown");
        Log (e.getMessage ());
        _stats.IncrementStat ("ReadDBRows_FromXML", e.getMessage (), "", "");
    }
    catch (std::exception &e) 
    {
        Log ("ReadDBRows_FromXML - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("ReadDBRows_FromXML", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("ReadDBRows_FromXML - exception thrown");
        _stats.IncrementStat ("ReadDBRows_FromXML", "exception ...", "", "");
    }

    XMLCh *ORDER = xercesc::XMLString::transcode ("order");
    XMLCh *TICKER = xercesc::XMLString::transcode ("ticker");
    XMLCh *TICKERID = xercesc::XMLString::transcode ("tickerid");
    XMLCh *BUYPRICE = xercesc::XMLString::transcode ("buyprice");
    XMLCh *NUMBUYSHARES = xercesc::XMLString::transcode ("numberbuyshares");
    XMLCh *SELLPRICE = xercesc::XMLString::transcode ("sellprice");
    XMLCh *NUMSELLSHARES = xercesc::XMLString::transcode ("numbersellshares");

    XMLCh *SECTYPE = xercesc::XMLString::transcode ("sectype");
    XMLCh *EXCHANGE = xercesc::XMLString::transcode ("exchange");
    XMLCh *PRIMARYEXCHANGE = xercesc::XMLString::transcode ("primaryexchange");
    XMLCh *CURRENCY = xercesc::XMLString::transcode ("currency");
    XMLCh *LOCALSYMBOL = xercesc::XMLString::transcode ("localsymbol");
    XMLCh *SECIDTYPE = xercesc::XMLString::transcode ("secidtype");
    XMLCh *SECID = xercesc::XMLString::transcode ("secid");
    XMLCh *GENERICTICKS = xercesc::XMLString::transcode ("genericticks");
    XMLCh *DISPLAYNAME = xercesc::XMLString::transcode ("displayname");

    for (std::list <std::string>::const_iterator iter = pgenfilenames.begin ();  iter != pgenfilenames.end ();  ++iter)
    {
        std::auto_ptr <xercesc::XercesDOMParser> parser (new xercesc::XercesDOMParser ());
        parser->setValidationScheme (xercesc::XercesDOMParser::Val_Never);
        parser->setDoNamespaces (false);
        parser->setDoSchema (false);
        parser->setLoadExternalDTD (false);

        try 
        {
            std::string filename = *iter;
            parser->parse (filename.c_str ());
            xercesc::DOMDocument *xmldoc = parser->getDocument ();
            xercesc::DOMElement *root = xmldoc->getDocumentElement ();
            if (root)
            {
                xercesc::DOMNodeList *children = root->getChildNodes ();

                const XMLSize_t nodecount = children->getLength ();
                for (XMLSize_t xx = 0;  xx < nodecount;  ++xx)
                {
                    xercesc::DOMNode *currentnode = children->item (xx);
                    if (currentnode->getNodeType () && currentnode->getNodeType () == xercesc::DOMNode::ELEMENT_NODE)
                    {
                        xercesc::DOMElement *currentelement = dynamic_cast <xercesc::DOMElement *> (currentnode);
                        if (currentelement)
                        {
                            const XMLCh *tagname = currentelement->getTagName ();
                            if (xercesc::XMLString::equals (tagname, ORDER))
                            {
                                const XMLCh *ticker = currentelement->getAttribute (TICKER);
                                const XMLCh *tickerid = currentelement->getAttribute (TICKERID);
                                const XMLCh *buyprice = currentelement->getAttribute (BUYPRICE);
                                const XMLCh *numberbuyshares = currentelement->getAttribute (NUMBUYSHARES);
                                const XMLCh *sellprice = currentelement->getAttribute (SELLPRICE);
                                const XMLCh *numbersellshares = currentelement->getAttribute (NUMSELLSHARES);
                                const XMLCh *sectype = currentelement->getAttribute (SECTYPE);
                                const XMLCh *exchange = currentelement->getAttribute (EXCHANGE);
                                const XMLCh *primaryexchange = currentelement->getAttribute (PRIMARYEXCHANGE);
                                const XMLCh *currency = currentelement->getAttribute (CURRENCY);
                                const XMLCh *localsymbol = currentelement->getAttribute (LOCALSYMBOL);
                                const XMLCh *secidtype = currentelement->getAttribute (SECIDTYPE);
                                const XMLCh *secid = currentelement->getAttribute (SECID);
                                const XMLCh *genericticks = currentelement->getAttribute (GENERICTICKS);
                                const XMLCh *displayname = currentelement->getAttribute (DISPLAYNAME);
    
                                result->AddDBRow (ticker, tickerid, buyprice, numberbuyshares,
                                                  sellprice, numbersellshares, sectype, exchange, 
                                                  primaryexchange, currency, localsymbol, secidtype, secid, genericticks, displayname,
                                                  0);
                            }
                        }
                        else
                        {
                            _stats.IncrementStat ("ReadDBRows_FromXML", "dynamic_cast failure", "", "");
                        }
                    }
                } 
            }
            else
            {
                Log ("Error parsing XML ");
                Log (filename.c_str ());
            }
        }
        catch (const xercesc::XMLException &e) 
        {
            Log ("ReadDBRows_FromXML - exception thrown");
            Log (e.getMessage ());
            _stats.IncrementStat ("ReadDBRows_FromXML", e.getMessage (), "", "");
        }
        catch (const xercesc::DOMException &e) 
        {
            Log ("ReadDBRows_FromXML - exception thrown");
            Log (e.msg);
            _stats.IncrementStat ("ReadDBRows_FromXML", e.msg, "", "");
        }
        catch (...) 
        {
            Log ("ReadDBRows_FromXML - exception thrown");
            _stats.IncrementStat ("ReadDBRows_FromXML", "exception ...", "", "");
        }
    }

    xercesc::XMLString::release (&ORDER);
    xercesc::XMLString::release (&TICKER);
    xercesc::XMLString::release (&TICKERID);
    xercesc::XMLString::release (&BUYPRICE);
    xercesc::XMLString::release (&NUMBUYSHARES);
    xercesc::XMLString::release (&SELLPRICE);
    xercesc::XMLString::release (&NUMSELLSHARES);
    xercesc::XMLString::release (&SECTYPE);
    xercesc::XMLString::release (&EXCHANGE);
    xercesc::XMLString::release (&PRIMARYEXCHANGE);
    xercesc::XMLString::release (&CURRENCY);
    xercesc::XMLString::release (&LOCALSYMBOL);
    xercesc::XMLString::release (&SECIDTYPE);
    xercesc::XMLString::release (&SECID);
    xercesc::XMLString::release (&GENERICTICKS);
    xercesc::XMLString::release (&DISPLAYNAME);

    xercesc::XMLPlatformUtils::Terminate ();

    return result;
}


bool IB_PTraderDlg::UpdateOI_OnDB (sql::Statement *stmt, PGenDatabaseRows::PGenDatabaseRow *dbrow, long newoi, bool updatehitcount)
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
    try
    {
        CString selector;
        selector.Format ("update deals set oi = %ld", newoi);
        CString selector1;
        if (updatehitcount)
        {
            selector1 += ", hitcount = hitcount + 1";
        }
        CString selector2;
        selector2.Format (" where iddeals = %lu and username = '%s' and accountnumber = '%s'",
                          dbrow->_iddeals, _username.c_str (), _accountnumber.c_str ());
        CString exec = selector + selector1 + selector2;
        stmt->executeUpdate ((LPCTSTR) exec);
    }
    catch (std::exception &e) 
    {
        Log ("UpdateOI_OnDB - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("UpdateOI_OnDB", e.what (), "", "");
        return false;
    }
    catch (...) 
    {
        Log ("UpdateOI_OnDB - exception thrown");
        _stats.IncrementStat ("UpdateOI_OnDB", "exception ...", "", "");
        return false;
    }
    return true;
}


bool IB_PTraderDlg::Update_AvgFillPrice_LastBuyDate_OnDB (sql::Statement *stmt, PGenDatabaseRows::PGenDatabaseRow *dbrow, double avgFillPrice, double avgcommission)
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
    try
    {
        CTime now = CTime::GetCurrentTime ();
        CString selector;
        selector.Format ("update deals set lastavgfillbuyprice = %g, lastbuycommission = %g, lastbuydate = '%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d'", 
                         avgFillPrice, avgcommission, now.GetYear (), now.GetMonth (), now.GetDay (), now.GetHour (), now.GetMinute (), now.GetSecond ());
        CString selector1;
        selector1.Format (" where iddeals = %lu and username = '%s' and accountnumber = '%s'",
                          dbrow->_iddeals, _username.c_str (), _accountnumber.c_str ());
        CString exec = selector + selector1;
        stmt->executeUpdate ((LPCTSTR) exec);
    }
    catch (std::exception &e) 
    {
        Log ("Update_AvgFillPrice_LastBuyDate_OnDB - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("Update_AvgFillPrice_LastBuyDate_OnDB", e.what (), "", "");
        return false;
    }
    catch (...) 
    {
        Log ("Update_AvgFillPrice_LastBuyDate_OnDB - exception thrown");
        _stats.IncrementStat ("Update_AvgFillPrice_LastBuyDate_OnDB", "exception ...", "", "");
        return false;
    }
    return true;
}


void IB_PTraderDlg::WriteDBRows_ToDB (PGenDatabaseRows *dbrows, bool emptydbfirst)
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
    try
    {
        if (emptydbfirst)
        {
            std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
            stmt->execute ("delete from deals");
        }

        if (! dbrows || dbrows->GetNumberOfRows () == 0)
            return;

        for (std::map <std::string, PGenDatabaseRows::PGenDatabaseRow *>::const_iterator iter = dbrows->_dbrows.begin ();  iter != dbrows->_dbrows.end ();  ++iter)
        {
            std::auto_ptr <sql::PreparedStatement> pstmt (_dbcon->prepareStatement (
"insert into deals (tickerid,ticker,oi,buyprice,numberbuyshares,sellprice,numbersellshares,sectype,"
"exchange,primaryexchange,currency,localsymbol,secidtype,secid,genericticks,displayname,username,accountnumber,"
"hitcount,lastavgfillbuyprice,lastbuycommission,lastbuydate) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
            const int BASE = 1;
            pstmt->setInt (BASE, iter->second->_tickerid);
            pstmt->setString (BASE + 1, iter->second->_ticker);
            pstmt->setInt (BASE + 2, iter->second->_oi);
            pstmt->setDouble (BASE + 3, iter->second->_buyprice);
            pstmt->setInt (BASE + 4, iter->second->_numberbuyshares);
            pstmt->setDouble (BASE + 5, iter->second->_sellprice);
            pstmt->setInt (BASE + 6, iter->second->_numbersellshares);
            pstmt->setString (BASE + 7, iter->second->_sectype);
            pstmt->setString (BASE + 8, iter->second->_exchange);
            pstmt->setString (BASE + 9, iter->second->_primaryexchange);
            pstmt->setString (BASE + 10, iter->second->_currency);
            pstmt->setString (BASE + 11, iter->second->_localsymbol);
            pstmt->setString (BASE + 12, iter->second->_secidtype);
            pstmt->setString (BASE + 13, iter->second->_secid);
            pstmt->setString (BASE + 14, iter->second->_genericticks);
            pstmt->setString (BASE + 15, iter->second->_displayname);
            pstmt->setString (BASE + 16, _username);
            pstmt->setString (BASE + 17, _accountnumber);
            pstmt->setInt (BASE + 18, iter->second->_hitcount);
            if (iter->second->_lastavgbuyfill > 0)
                pstmt->setDouble (BASE + 19, iter->second->_lastavgbuyfill);
            else
                pstmt->setNull (BASE + 19, 0);
            if (iter->second->_lastbuycommission > 0)
                pstmt->setDouble (BASE + 20, iter->second->_lastbuycommission);
            else
                pstmt->setNull (BASE + 20, 0);
            if (iter->second->_lastbuydate != "")
                pstmt->setDateTime (BASE + 21, iter->second->_lastbuydate);
            else
                pstmt->setNull (BASE + 21, 0);

            pstmt->executeUpdate ();
        }
    }
    catch (std::exception &e) 
    {
        Log ("WriteDBRows_ToDB - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("WriteDBRows_ToDB", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("WriteDBRows_ToDB - exception thrown");
        _stats.IncrementStat ("WriteDBRows_ToDB", "exception ...", "", "");
    }
}


PGenDatabaseRows *IB_PTraderDlg::ReadDBRows_FromDB ()
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
    PGenDatabaseRows *result = new PGenDatabaseRows ();
    try
    {
        std::auto_ptr <sql::Statement> stmt (_dbcon->createStatement ());
        std::auto_ptr <sql::ResultSet> resultset (stmt->executeQuery (
"select iddeals,tickerid,ticker,oi,buyprice,numberbuyshares,sellprice,numbersellshares,sectype,exchange,primaryexchange,currency,localsymbol,"
"secidtype,secid,genericticks,displayname,hitcount,lastavgfillbuyprice,lastbuycommission,lastbuydate from deals where username= '" + _username + 
std::string ("' and accountnumber = '") + _accountnumber + std::string ("'")));
        while (resultset->next ()) 
        {
            const int BASE = 1;
            IdDeals iddeals = resultset->getUInt (BASE);
            TickerId tickerid = resultset->getInt (BASE + 1);
            sql::SQLString ticker = resultset->getString (BASE + 2);
            long oi = resultset->getInt (BASE + 3);
            double buyprice = (double) resultset->getDouble (BASE + 4);
            long numberbuyshares = resultset->getInt (BASE + 5);
            double sellprice = (double) resultset->getDouble (BASE + 6);
            long numbersellshares = resultset->getInt (BASE + 7);
            sql::SQLString sectype = resultset->getString (BASE + 8);
            sql::SQLString exchange = resultset->getString (BASE + 9);
            sql::SQLString primaryexchange = resultset->getString (BASE + 10);
            sql::SQLString currency = resultset->getString (BASE + 11);
            sql::SQLString localsymbol = resultset->getString (BASE + 12);
            sql::SQLString secidtype = resultset->getString (BASE + 13);
            sql::SQLString secid = resultset->getString (BASE + 14);
            sql::SQLString genericticks = resultset->getString (BASE + 15);
            sql::SQLString displayname = resultset->getString (BASE + 16);
            long hitcount = resultset->getInt (BASE + 17);
            double lastavgfillbuyprice;
            if (resultset->isNull (BASE + 18))
                lastavgfillbuyprice = 0;
            else
                lastavgfillbuyprice = (double) resultset->getDouble (BASE + 18);
            double lastbuycommission;
            if (resultset->isNull (BASE + 19))
                lastbuycommission = 0;
            else
                lastbuycommission = (double) resultset->getDouble (BASE + 19);
            sql::SQLString lastbuydate;
            if (resultset->isNull (BASE + 20))
                lastbuydate = "";
            else
                lastbuydate = resultset->getString (BASE + 20);

            result->AddDBRow (iddeals, tickerid, ticker.asStdString (), oi,
                              buyprice, numberbuyshares, sellprice, numbersellshares,
                              sectype.asStdString (), exchange.asStdString (), primaryexchange.asStdString (), 
                              currency.asStdString (), localsymbol.asStdString (), secidtype.asStdString (), 
                              secid.asStdString (), genericticks.asStdString (),
                              displayname.asStdString (), hitcount, lastavgfillbuyprice, lastbuycommission,
                              lastbuydate.asStdString ());
        }
    }
    catch (std::exception &e) 
    {
        Log ("ReadDBRows_FromDB - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("ReadDBRows_FromDB", e.what (), "", "");
    }
    catch (...) 
    {
        Log ("ReadDBRows_FromDB - exception thrown");
        _stats.IncrementStat ("ReadDBRows_FromDB", "exception ...", "", "");
    }
    return result;
}


void IB_PTraderDlg::InitialiseTickers (TickersInitialisation initialisation)
{
    BeginWaitCursor ();
    PGenDatabaseRows *init = 0;
    CString format;
    switch (initialisation)
    {
        case Both:
        {
            PGenDatabaseRows *db = ReadDBRows_FromDB ();
            PGenDatabaseRows *xml = ReadDBRows_FromXML ();
            PGenDatabaseRows *delta = new PGenDatabaseRows ();
            Log ("TickersInitialisation::Both");
            Log ("Heap Fragmentation may have occurred - restart with DB option");
            _stats.IncrementStat ("InitialiseTickers", "Heap Fragmentation - Both", "", "");
            init = PGenDatabaseRows::Merge (db, xml, delta);
            format.Format ("%d new rows added", delta != 0 ? delta->GetNumberOfRows () : 0);
            Log (format);
            if (delta->GetNumberOfRows () > 0)
            {
                Log ("Added Delta Tickers:-");
                std::map <std::string, bool> showntickers;
                for (PGenDatabaseRows::DBRowsIter iter = delta->_dbrows.begin ();  iter != delta->_dbrows.end ();  ++iter)
                {   
                    std::map <std::string, bool>::iterator showniter = showntickers.find (iter->second->_ticker);
                    if (showniter == showntickers.end ())
                    {
                        showntickers [iter->second->_ticker] = true;
                        Log (iter->second->_ticker.c_str ());
                    }
                }
            }
            WriteDBRows_ToDB (delta, false);
            PGenDatabaseRows::Delete (db, true);
            PGenDatabaseRows::Delete (xml, true);
            PGenDatabaseRows::Delete (delta, false);
            PGenDatabaseRows::Delete (init, false);
            break;
        }
        case DB:
        {
            break;
        }
        case PGen:
        {
            init = ReadDBRows_FromXML ();
            Log ("TickersInitialisation::PGen");
            Log ("Heap Fragmentation may have occurred - restart with DB option");
            _stats.IncrementStat ("InitialiseTickers", "Heap Fragmentation - PGen", "", "");
            format.Format ("%d rows added from XML", init != 0 ? init->GetNumberOfRows () : 0);
            Log (format);
            WriteDBRows_ToDB (init, true);
            PGenDatabaseRows::Delete (init, true);
            break;
        }
    }

    init = ReadDBRows_FromDB ();
    format.Format ("%d rows read from DB", init != 0 ? init->GetNumberOfRows () : 0);
    Log (format);
    _tickers = new Tickers (init, this);
    CTime now = CTime::GetCurrentTime ();
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        Ticker *ticker = iter->second;
        SetTickerActive (ticker);
        SetAllowEarlyBuyTicker (ticker);
        MarketDetails *marketdetails = GetMarketDetails (ticker);
        ticker->_marketstatus = MarketDetails::GetMarketStatus (marketdetails, now);
        ticker->DetermineBranches ();
        if (ticker->_buys.GetDataCount () == 1 && ticker->_activestate != Ticker::DISABLED)
        {
            BuyPricePair *buypricepair = ticker->_buys.GetData (0);
            if (buypricepair->_price == 0)
            {
                ticker->_tickerhasnoprices = true;
                ticker->_activestate = Ticker::InActive;
                format.Format ("%s has no active prices", ticker->_ticker.c_str ());
                Log (format);
            }
        }
    }
    _ordermaps.ReadOutStandingOrdersFromDB_AndWireUp ();
    EndWaitCursor ();
}

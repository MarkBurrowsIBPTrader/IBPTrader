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
#include <memory>



void IB_PTraderDlg::AddTickerToRecord (std::string ticker, std::string primaryexch, std::string sectype, Ticker::PriceRecordType pricerecordtype)
{
    Ticker *foundticker = _tickers ? _tickers->GetTickerByContract (ticker, primaryexch, sectype) : 0;
    if (foundticker)
    {
        if (_tickers_to_record == 0)
            _tickers_to_record = new std::list <Ticker *> ();
        _tickers_to_record->push_back (foundticker);
        foundticker->_pricerecordtype = pricerecordtype;
        if (foundticker->_pricerecordtype == Ticker::LivePrices)
        {
            foundticker->_livepriceset = new LivePricesSet (this, foundticker, true);
        }
        Log (CString (ticker.c_str ()) + " (" + CString (primaryexch.c_str ()) + ") set to record prices: " + Ticker::ToString (foundticker->_pricerecordtype, false));
        _stats.IncrementStat ("AddTickerToRecord", std::string ("RecordingPrice_") + std::string ((LPCTSTR) Ticker::ToString (foundticker->_pricerecordtype, false)), ticker, foundticker->_firstdbrow->_currency);
    }
    else
    {
        Log (CString (ticker.c_str ()) + " NOT SET to record prices");
        _stats.IncrementStat ("AddTickerToRecord", "TickerNotFound", ticker, "");
    }
}


#if ! PUBLIC_BUILD
bool IB_PTraderDlg::RecordPrice_ToDB (Ticker *ticker)
{
    try
    {
        /*
            ibrokers.recordedprices (Table)
            ================================
            idrecordedprices int(10) unsigned PK
            ticker           text
            primaryexchange  text
            currency         text
            bid              double
            offer            double
            volume           int(11)
            createtimestamp  timestamp
        */
        std::auto_ptr <sql::PreparedStatement> pstmt (_dbcon->prepareStatement (
                                "insert into recordedprices (ticker,primaryexchange,currency,bid,offer,volume) VALUES (?,?,?,?,?,?)"
                                                                 ));
        const int BASE = 1;
        pstmt->setString (BASE, ticker->_ticker);                                       // ticker
        pstmt->setString (BASE + 1, ticker->_firstdbrow->_primaryexchange);             // primaryexchange
        pstmt->setString (BASE + 2, ticker->_firstdbrow->_currency);                    // currency
        pstmt->setDouble (BASE + 3, ticker->_currentbid);                               // bid
        pstmt->setDouble (BASE + 4, ticker->_currentoffer);                             // offer
        pstmt->setInt (BASE + 5, ticker->_volume);                                      // volume

        pstmt->executeUpdate ();

        _stats.IncrementStat ("RecordPrice_ToDB", "RecordPrice", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    catch (std::exception &e) 
    {
        Log ("RecordPrice_ToDB - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("RecordPrice_ToDB", e.what (), ticker->_ticker, ticker->_firstdbrow->_currency);
        return false;
    }
    catch (...) 
    {
        Log ("RecordPrice_ToDB - exception thrown");
        _stats.IncrementStat ("RecordPrice_ToDB", "exception ...", ticker->_ticker, ticker->_firstdbrow->_currency);
        return false;
    }
    return true;
}
#endif


void IB_PTraderDlg::Write_ToFile (CFile &file, CString &str)
{
    LPCTSTR buf = (LPCTSTR) str;
    UINT len = lstrlen (buf);
    file.Write (buf, len);
}


void IB_PTraderDlg::Write_NewLineToFile (CFile &file)
{
    file.Write ("\r\n", 2);
}


#if ! PUBLIC_BUILD
bool IB_PTraderDlg::RecordPrice_ToFile (Ticker *ticker, CTime &now)
{
    try
    {
        CString filename = _currentworkingdir + CString ("\\") +
                           std::string (ticker->_ticker + "_" +  ticker->_firstdbrow->_primaryexchange + "_" + ticker->_firstdbrow->_currency + ".txt").c_str ();
        CFile file (filename, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite);
        file.SeekToEnd ();

        CString format;
        format.Format ("%g", ticker->_currentbid);
        Write_ToFile (file, format);

        format.Format (",%g", ticker->_currentoffer);
        Write_ToFile (file, format);

        format.Format (",%d,", ticker->_volume);
        Write_ToFile (file, format);

        format = now.Format ("%c");
        Write_ToFile (file, format);     

        Write_NewLineToFile (file);
        file.Close ();

        _stats.IncrementStat ("RecordPrice_ToFile", "RecordPrice", ticker->_ticker, ticker->_firstdbrow->_currency);
    }
    catch (...)
    {
        Log ("RecordPrice_ToFile - exception thrown");
        _stats.IncrementStat ("RecordPrice_ToFile", "exception ...", ticker->_ticker, ticker->_firstdbrow->_currency);
        return false;
    }
    return true;
}
#endif


void IB_PTraderDlg::RecordPrices (CTime &now)
{
    if (_tickers_to_record)
    {
        int recordcount = 0;
        for (RecordTickersIter iter = _tickers_to_record->begin ();  iter != _tickers_to_record->end ();  ++iter)
        {
            Ticker *ticker = *iter;
            if (ticker->MinMaxSetBidOffer ())
            {
                MarketDetails *marketdetails = GetMarketDetails (ticker);
                if (marketdetails)
                {
                    ticker->_marketstatus = MarketDetails::GetMarketStatus (marketdetails, now);
                    if (MarketDetails::IsMarketOpen (ticker->_marketstatus))
                    {
                        switch (ticker->_pricerecordtype)
                        {
#if ! PUBLIC_BUILD
                            case Ticker::RecordToDB:
                            {
                                if (RecordPrice_ToDB (ticker))
                                    recordcount++;
                                break;
                            }
                            case Ticker::RecordToFile:
                            {
                                if (RecordPrice_ToFile (ticker, now))
                                    recordcount++;
                                break;
                            }
#endif
                            case Ticker::LivePrices:
                            {
                                ticker->_livepriceset->NewLivePrice ();
                                break;
                            }
                        }
                    }
                }
            }
        }
        SetWindowTitle (recordcount, &now);
    }
}

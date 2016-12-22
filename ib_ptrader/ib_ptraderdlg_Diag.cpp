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
#include "ib_datadumpdlg.h"
#include <memory>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



void IB_PTraderDlg::DumpNewLineToFile (DumpFlags dumpflags, CFile *file)
{
    if (file)
        file->Write ("\r\n", 2);
}


void IB_PTraderDlg::DumpLineToFile (DumpFlags dumpflags, CFile *file, CString &str)
{
    LPCTSTR buf = (LPCTSTR) str;
    if (file)
    {
        UINT len = lstrlen (buf);
        file->Write (buf, len);
    }
    else
    {
        Log (buf);
    }
}


void IB_PTraderDlg::DumpLineToFile (DumpFlags dumpflags, CFile *file, LPCTSTR buf)
{
    if (file)
    {
        UINT len = lstrlen (buf);
        file->Write (buf, len);
    }
    else
    {
        Log (buf);
    }
}


void IB_PTraderDlg::DumpLineToFile (DumpFlags dumpflags, CFile *file, std::string str)
{
    DumpLineToFile (dumpflags, file, str.c_str ());
}


void IB_PTraderDlg::DumpLineToFile (DumpFlags dumpflags, CFile *file, long value)
{
    CString str;
    str.Format ("%ld", value);
    DumpLineToFile (dumpflags, file, str);
}


void IB_PTraderDlg::DumpLineToFile (DumpFlags dumpflags, CFile *file, LPCTSTR prefixtext, unsigned long value, LPCSTR postfixtext)
{
    CString str;
    str.Format ("%s%lu%s", prefixtext, value, postfixtext);
    DumpLineToFile (dumpflags, file, str);
}


void IB_PTraderDlg::DumpLineToFile (DumpFlags dumpflags, CFile *file, unsigned long value)
{
    CString str;
    str.Format ("%lu", value);
    DumpLineToFile (dumpflags, file, str);
}


void IB_PTraderDlg::DumpLineToFile (DumpFlags dumpflags, CFile *file, double value)
{
    CString str;
    str.Format ("%g", value);
    DumpLineToFile (dumpflags, file, str);
}


void IB_PTraderDlg::DumpSep (DumpFlags dumpflags, CFile *file)
{
    DumpLineToFile (dumpflags, file, "*********************************************");
}


void IB_PTraderDlg::DumpListBoxContents (LPCTSTR filename, CListBox &listbox)
{
    DumpFlags dumpflags = ToFile;
    std::auto_ptr <CFile> file (new CFile (filename, CFile::modeCreate | CFile::modeReadWrite));
    int numberitems = listbox.GetCount ();
    for (int i = 0;  i < numberitems;  i++)
    {
        CString str;
        listbox.GetText (i, str);
        DumpLineToFile (dumpflags, file.get (), str);
        DumpNewLineToFile (dumpflags, file.get ());
    }
    file->Close ();
}


void IB_PTraderDlg::OnBnClickedDumpquotes ()
{
    DumpFlags dumpflags = ToFile;
    CString filename = GetFileName ("quoteslistbox");
    std::auto_ptr <CFile> file (new CFile (filename, CFile::modeCreate | CFile::modeReadWrite));
    int numberitems = _mainlistbox.GetCount ();
    for (int i = 0;  i < numberitems;  i++)
    {
        Ticker *ticker = (Ticker *) _mainlistbox.GetItemData (i);
        CString str = ticker->_currentlistboxline;
        if (str.GetLength () == 0)
            str = CString ("<Not Set> - ") + ticker->_ticker.c_str ();
        DumpLineToFile (dumpflags, file.get (), str);
        DumpNewLineToFile (dumpflags, file.get ());
    }
    file->Close ();
    OnBnClickedScrollmessages ();
}


void IB_PTraderDlg::OnBnClickedListdump ()
{
    CString filename = GetFileName ("mainlistbox");
    DumpListBoxContents (filename, _statusmessageslistbox);
    OnBnClickedScrollmessages ();
}

void IB_PTraderDlg::DumpData (DumpFlags dumpflags)
{
    std::auto_ptr <CFile> file (0);
    if ((dumpflags & ToFile) != 0)
    {
        CString filename = GetFileName ("internaldata");
        file.reset (new CFile (filename, CFile::modeCreate | CFile::modeReadWrite));
        OnBnClickedScrollmessages ();
    }
    long count;
    if (_shares && ((dumpflags & Shares) != 0))
    {
        DumpLineToFile (dumpflags, file.get (), "Shares:-");
        DumpNewLineToFile (dumpflags, file.get ());
        count = 1;
        for (SharesIter iter = _shares->begin ();  iter != _shares->end ();  ++iter)
        {
            Share *share = iter->second;
            DumpLineToFile (dumpflags, file.get (), count);
            DumpLineToFile (dumpflags, file.get (), ") TickerId=");
            DumpLineToFile (dumpflags, file.get (), share->_tickerid);
            DumpLineToFile (dumpflags, file.get (), ", (");
            DumpLineToFile (dumpflags, file.get (), share->_ticker);
            DumpLineToFile (dumpflags, file.get (), "), CurrentOI=");
            DumpLineToFile (dumpflags, file.get (), share->_currentoi);
            DumpLineToFile (dumpflags, file.get (), ", MaxOI=");
            DumpLineToFile (dumpflags, file.get (), share->_maxoi);
            DumpLineToFile (dumpflags, file.get (), ", MaxValue=");
            DumpLineToFile (dumpflags, file.get (), share->_maxvalue);
            if (share->_tickerptr)
            {
                DumpLineToFile (dumpflags, file.get (), ", Bid_MinMaxesSet=");
                DumpLineToFile (dumpflags, file.get (), share->_tickerptr->_minmaxes_set_bid ? "True" : "False");
                DumpLineToFile (dumpflags, file.get (), ", Offer_MinMaxesSet=");
                DumpLineToFile (dumpflags, file.get (), share->_tickerptr->_minmaxes_set_offer ? "True" : "False");
            }
            DumpNewLineToFile (dumpflags, file.get ());
            count++;
        }
    }

    if ((dumpflags & Orders) != 0)
    {
        DumpSep (dumpflags, file.get ());
        DumpNewLineToFile (dumpflags, file.get ());
        DumpLineToFile (dumpflags, file.get (), "Outstanding Orders:-");
        DumpNewLineToFile (dumpflags, file.get ());
        count = 1;
        for (OrderMaps::OrderIdIter iter = _ordermaps._orderid_to_outstandingorder.begin ();  iter !=  _ordermaps._orderid_to_outstandingorder.end ();  ++iter)
        {
            DumpLineToFile (dumpflags, file.get (), count);
            DumpLineToFile (dumpflags, file.get (), ") OrderId=");
            DumpLineToFile (dumpflags, file.get (), iter->second->_orderid);
            DumpLineToFile (dumpflags, file.get (), ", ");
            DumpLineToFile (dumpflags, file.get (), OutStandingOrder::ToString (iter->second->GetOrderType ()));
            DumpNewLineToFile (dumpflags, file.get ());       
            bool fp = true;
            for each (BuyPricePair *buyprice in iter->second->_buypricepairs)
            {
                if (fp)
                {
                    fp = false;
                }
                else
                {
                    DumpLineToFile (dumpflags, file.get (), ", ");
                }
                DumpLineToFile (dumpflags, file.get (), "IdDeals=");
                DumpLineToFile (dumpflags, file.get (), buyprice->_dbrow->_iddeals);
            }
            DumpNewLineToFile (dumpflags, file.get ()); 
            count++;
        }
    }
    
    if (((dumpflags & BuyTickers) != 0) || ((dumpflags & SellTickers) != 0))
    {
        DumpSep (dumpflags, file.get ());
        DumpNewLineToFile (dumpflags, file.get ());
        DumpLineToFile (dumpflags, file.get (), "Tickers:-");
        DumpNewLineToFile (dumpflags, file.get ());
        if (_tickers)
        {
            CString erredbuyorsellindex;
            for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
            {
                Ticker *ticker = iter->second;
                DumpLineToFile (dumpflags, file.get (), ticker->_ticker);
                DumpNewLineToFile (dumpflags, file.get ());
            
                CString format;
                if (((dumpflags & BuyTickers) != 0))
                {
                    count = 1;
                    DumpLineToFile (dumpflags, file.get (), "Buys:-");
                    DumpNewLineToFile (dumpflags, file.get ());
                    int size = ticker->_buys.GetDataCount ();
                    int expectedindex = 0;
                    for (int i = 0;  i < size;  i++)
                    {
                        BuyPricePair *buyprice = ticker->_buys.GetData (i);
                        DumpLineToFile (dumpflags, file.get (), count);
                        DumpLineToFile (dumpflags, file.get (), ") ");
                        DumpLineToFile (dumpflags, file.get (), buyprice->_dbrow->_iddeals);
                        DumpLineToFile (dumpflags, file.get (), ", ");
                        DumpLineToFile (dumpflags, file.get (), PricePair::ToString (buyprice->GetStatus ()));
                        DumpLineToFile (dumpflags, file.get (), ", ");
                        DumpLineToFile (dumpflags, file.get (), buyprice->_price);
                        DumpLineToFile (dumpflags, file.get (), " (");
                        DumpLineToFile (dumpflags, file.get (), buyprice->_size);
                        DumpLineToFile (dumpflags, file.get (), "), OI=");
                        DumpLineToFile (dumpflags, file.get (), buyprice->_oi);
                        DumpLineToFile (dumpflags, file.get (), ", Index=");
                        DumpLineToFile (dumpflags, file.get (), (long) buyprice->_pairindex);
                        if (buyprice->_pairindex != expectedindex++)
                        {
                            if (erredbuyorsellindex.GetLength () == 0)
                                erredbuyorsellindex = ticker->_ticker.c_str ();
                        }
                        if (buyprice->_lastbuysset)
                        {
                            format.Format (", LASTBUYSET (BT=%s, %s), Fill=%0.2f, Com=%0.2f",
                                           buyprice->_boughttoday ? "true" : "false",
                                           buyprice->_dbrow->_lastbuydate.c_str (),
                                           buyprice->_lastavgfill, 
                                           buyprice->_lastbuycommission);
                            DumpLineToFile (dumpflags, file.get (), format);
                        }
                        DumpNewLineToFile (dumpflags, file.get ());
                        count++;
                    }   
                }

                if ((dumpflags & SellTickers) != 0)
                {
                    count = 1;
                    DumpLineToFile (dumpflags, file.get (), "Sells:-");
                    DumpNewLineToFile (dumpflags, file.get ());
                    int size = ticker->_sells.GetDataCount ();
                    int expectedindex = 0;
                    for (int i = 0;  i < size;  i++)
                    {
                        SellPricePair *sellprice = ticker->_sells.GetData (i);
                        DumpLineToFile (dumpflags, file.get (), count);
                        DumpLineToFile (dumpflags, file.get (), ") ");
                        DumpLineToFile (dumpflags, file.get (), sellprice->_buypricepair->_dbrow->_iddeals);
                        DumpLineToFile (dumpflags, file.get (), ", ");
                        DumpLineToFile (dumpflags, file.get (), PricePair::ToString (sellprice->GetStatus ()));
                        DumpLineToFile (dumpflags, file.get (), ", ");
                        DumpLineToFile (dumpflags, file.get (), sellprice->_price);
                        DumpLineToFile (dumpflags, file.get (), " (");
                        DumpLineToFile (dumpflags, file.get (), sellprice->_size);
                        DumpLineToFile (dumpflags, file.get (), ")");
                        DumpLineToFile (dumpflags, file.get (), ", Index=");
                        DumpLineToFile (dumpflags, file.get (), (long) sellprice->_pairindex);
                        if (sellprice->_pairindex != expectedindex++)
                        {
                            if (erredbuyorsellindex.GetLength () == 0)
                                erredbuyorsellindex = ticker->_ticker.c_str ();
                        }
                        DumpNewLineToFile (dumpflags, file.get ());
                        count++;
                    }
                }
            }
            if (erredbuyorsellindex.GetLength () > 0)
            {
                DumpLineToFile (dumpflags, file.get (), CString ("Errored on PairIndex for ") + erredbuyorsellindex);
                DumpNewLineToFile (dumpflags, file.get ());
            }
        }
    }

    if (file.get ())
    {
        file->Close ();
    }
}


void IB_PTraderDlg::OnBnClickedInternaldata ()
{
    BeginWaitCursor ();
    DumpData ((DumpFlags) (ToFile | Shares | Orders | BuyTickers | SellTickers));
    EndWaitCursor (); 
}

/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "LivePriceLogger.h"
#include "Ticker.h"
#include "ChartViewDlg.h"



#pragma region "LivePricePoint"
void LivePricePoint::Dump (CFile &file)
{
    CString format;
    format.Format ("%d,%g,%g,%d,%d,%d,%d,%d,%d,%d", 
                   _index,
                   _bid, _offer, _volume, 
                   _hour, _min, _sec, _day, _month, _year);
    IB_PTraderDlg::Write_ToFile (file, format);    
}


void LivePricePoint::Read (CString &line)
{
    ::sscanf_s ((LPCTSTR) line, "%d,%lf,%lf,%d,%d,%d,%d,%d,%d,%d", &_index, &_bid, &_offer, &_volume, 
                &_hour, &_min, &_sec, &_day, &_month, &_year);
}


LivePricePoint::LivePricePoint ()
{
}


LivePricePoint::LivePricePoint (int hour, int min, int sec, int day, int month, int year) :
                                                 _hour (hour), _min (min), _sec (sec), _day (day), _month (month), _year (year)
{
}
#pragma endregion


#pragma region "LivePrices"
LivePricePoint *LivePrices::GetNewLivePricePoint ()
{
    CTime now = CTime::GetCurrentTime ();
    LivePricePoint *livepricepoint = new LivePricePoint (now.GetHour (),
                                                         now.GetMinute (),
                                                         now.GetSecond (),
                                                         now.GetDay (),
                                                         now.GetMonth (),
                                                         now.GetYear ());
    _livepricepoints.push_back (livepricepoint);
    return livepricepoint;
}


void LivePrices::CleanUp ()
{
    for (std::vector <LivePricePoint *>::iterator iter = _livepricepoints.begin ();  iter != _livepricepoints.end ();  ++iter)
    {
        delete *iter;
    }
    _livepricepoints.clear ();
}


LivePricePoint *LivePrices::GetTheLivePricePoint ()
{
    size_t size = _livepricepoints.size ();
    return size > 0 ? _livepricepoints [size - 1] : 0;
}


void LivePrices::Get_HLOC (double &high, double &low, double &open, double &close)
{
    const bool USE_OFFER = true;

    high = DBL_MIN;
    low = DBL_MAX;
    for each (LivePricePoint *pricepoint in _livepricepoints)
    {
        double value = pricepoint->GetValue (USE_OFFER);
        if (value > high)
            high = value;
        if (value < low)
            low = value;
    }
    open = _livepricepoints [0]->GetValue (USE_OFFER);
    close = GetTheLivePricePoint ()->GetValue (USE_OFFER);
}


void LivePrices::Get_HLOC (std::list <LivePrices *> &liveprices, double &high, double &low, double &open, double &close)
{
    const bool USE_OFFER = true;

    high = DBL_MIN;
    low = DBL_MAX;
    LivePricePoint *firstpricepoint = 0, *lastpricepoint = 0;
    for each (LivePrices *livepriceslist in liveprices)
    {
        for each (LivePricePoint *pricepoint in livepriceslist->_livepricepoints)
        {
            double value = pricepoint->GetValue (USE_OFFER);
            if (value > high)
                high = value;
            if (value < low)
                low = value;
            if (firstpricepoint == 0)
                firstpricepoint = pricepoint;
            lastpricepoint = pricepoint;
        }
    }
    open = firstpricepoint ? firstpricepoint->GetValue (USE_OFFER) : 0;
    close = lastpricepoint ? lastpricepoint->GetValue (USE_OFFER) : 0;
}


bool LivePrices::DataToDump ()
{
    return _livepricepoints.size () > 0;
}


void LivePrices::Dump (CFile &file, int index)
{
    for each (LivePricePoint *pricepoint in _livepricepoints)
    {
        pricepoint->Dump (file);
        IB_PTraderDlg::Write_NewLineToFile (file);
    }
}


void LivePricesSet::CleanUp ()
{
    for (std::vector <LivePrices *>::iterator iter = _liveprices.begin ();  iter != _liveprices.end ();  ++iter)
    {
        (*iter)->CleanUp ();
        delete *iter;
    }
    _liveprices.clear ();
}
#pragma endregion


#pragma region "LivePricesSet"
LivePrices *LivePricesSet::PriceChange ()
{
    if (_parentticker->MinMaxSetBidOffer ())  
    {
        LivePrices *currentliveprices;
        size_t size = _liveprices.size ();
        if (size == 0)
        {
            currentliveprices = new LivePrices ();
            _liveprices.push_back (currentliveprices);
        }
        else
        {
            currentliveprices = _liveprices [size - 1];
        }

        LivePricePoint *livepricepoint = currentliveprices->GetNewLivePricePoint ();
        livepricepoint->_index = (int) size + 1;
        livepricepoint->_bid = _parentticker->CurrentBid ();
        livepricepoint->_offer = _parentticker->CurrentOffer ();
        livepricepoint->_volume = _parentticker->Volume ();
        return currentliveprices;
    }
    return 0;
}


CString LivePricesSet::GetLivePricesLogFile (int day, int month, int year)
{
    CString filename;
    filename.Format ("%s\\%s_%s_%d_%d_%d.txt",
                     (LPCTSTR) _parent->CurrentWorkingDir (),
                     _parentticker->TickerName ().c_str (), _parent->UserName ().c_str (),
                     day, month, year);
    return filename;
}


CString LivePricesSet::GetLivePricesLogFile (const CTime &time)
{
    return GetLivePricesLogFile (time.GetDay (), time.GetMonth (), time.GetYear ());
}


void TalkToChartDialog (LivePricesSet *livepricesset, LivePrices *currentliveprices, CChartViewDlg::ChartPeriod chartperiod, size_t size, bool newpriceadded)
{
    CChartViewDlg *dlg = livepricesset->Parent ()->GetChartViewDlg (livepricesset->ParentTicker (), CChartViewDlg::Live, chartperiod); 
    if (dlg != 0)
    {   
        if (newpriceadded)
        {   
            dlg->LiveUpdate (currentliveprices);
        }
        else
        {
            while (--size >= 0)
            {
                if ((*livepricesset) [size - 1]->DataToDump ())
                {
                    dlg->LiveUpdate ((*livepricesset) [size - 1]);
                    break;
                }
            }
        }
    }
}


void LivePricesSet::NewLivePrice ()
{
    size_t size = _liveprices.size ();
    if (size > 0)
    {
        LivePrices *currentliveprices = _liveprices [size - 1];
        try
        {
            bool newpriceadded = currentliveprices->DataToDump ();
            if (newpriceadded)
            {
                // Update Log file
                CString filename = GetLivePricesLogFile (CTime::GetCurrentTime ());
                CFile file (filename, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite);
                file.SeekToEnd ();
                currentliveprices->Dump (file, (int) size);
                file.Close ();
            }
            // Update Live Chart
            if (_parentticker->GetLivePricesSet ())
            {
                TalkToChartDialog (this, currentliveprices, CChartViewDlg::Sec30, size, newpriceadded);
                TalkToChartDialog (this, currentliveprices, CChartViewDlg::Min1, size, newpriceadded);
                TalkToChartDialog (this, currentliveprices, CChartViewDlg::Min2point5, size, newpriceadded);
            }
        }
        catch (std::exception &e) 
        {
            _parent->Log ("NewLivePrice - exception thrown");
            _parent->Log (e.what ());
            _parent->GetStats ().IncrementStat ("NewLivePrice", e.what (), _parentticker->TickerName (), _parentticker->FirstDBRow ()->_currency);
        }
        catch (...)
        {
            _parent->Log ("NewLivePrice - exception thrown");
            _parent->GetStats ().IncrementStat ("NewLivePrice", "exception ...", _parentticker->TickerName (), _parentticker->FirstDBRow ()->_currency);
        }
        currentliveprices = new LivePrices ();
        _liveprices.push_back (currentliveprices);
    }
}


void LivePricesSet::LoadDataSet (int day, int month, int year)
{
    try
    {
        CString filename = GetLivePricesLogFile (day, month, year);
        CFileException fileexcep;
        CStdioFile stdfile;
        if (stdfile.Open (filename, CFile::modeRead | CFile::modeNoTruncate, &fileexcep))
        {
            int lastindex = 0;
            CString line;
            while (stdfile.ReadString (line))
            {
                LivePricePoint *pricepoint = new LivePricePoint ();
                pricepoint->Read (line);
                LivePrices *liveprices = 0;
                if (pricepoint->_index == lastindex)
                {
                    size_t size = _liveprices.size ();
                    if (size == 0)
                    {
                        liveprices = new LivePrices ();
                        _liveprices.push_back (liveprices);
                    }
                    else
                    {
                        liveprices = _liveprices [size - 1];
                    }
                }
                else
                {
                    for (int i = lastindex + 1;  i <= pricepoint->_index;  i++)
                    {
                        liveprices = new LivePrices ();
                        _liveprices.push_back (liveprices);
                    }
                }
                liveprices->_livepricepoints.push_back (pricepoint);
                lastindex = pricepoint->_index;
            }
        }
        stdfile.Close ();
    }
    catch (std::exception &e) 
    {
        _parent->GetStats ().IncrementStat ("LoadDataSet", e.what (), _parentticker->TickerName (), _parentticker->FirstDBRow ()->_currency);
    }
    catch (...)
    {
        _parent->GetStats ().IncrementStat ("LoadDataSet", "exception ...", _parentticker->TickerName (), _parentticker->FirstDBRow ()->_currency);
    }
}


void LivePricesSet::LoadDataSet (const CTime &time)
{
    LoadDataSet (time.GetDay (), time.GetMonth (), time.GetYear ());
}


int LivePricesSet::StartIter ()
{
    _lastiterliveprices = 0;
    return 0;
}


LivePrices *LivePricesSet::GetNextIter (int &iter)
{
    int datasize = (int) _liveprices.size ();
    while (datasize > iter)
    {
        LivePrices *liveprice = _liveprices [iter++];
        if (liveprice->DataToDump ())
            return liveprice;
    }
    return 0;
}


LivePrices *LivePricesSet::GetNextIterNoSkip (int &iter)
{
    int datasize = (int) _liveprices.size ();
    while (datasize > iter)
    {
        LivePrices *liveprice = _liveprices [iter++];
        if (liveprice->DataToDump ())
        {
            _lastiterliveprices = liveprice;
            return liveprice;
        }
        if (_lastiterliveprices)
            return _lastiterliveprices;
    }
    return 0;
}


LivePricesSet::LivePricesSet (IB_PTraderDlg *parent, Ticker *parentticker, bool loadtodaysdataset) : _parent (parent), _parentticker (parentticker)
{
    if (loadtodaysdataset)
    {
        LoadDataSet (CTime::GetCurrentTime ());
    }
}
#pragma endregion

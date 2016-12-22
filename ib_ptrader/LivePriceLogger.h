/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include <map>
#include <vector>



class LivePricePoint
{
private:
    int _index;
    int _hour, _min, _sec, _day, _month, _year;
    double _bid, _offer;
    int _volume;

    friend class LivePricesSet;
    friend class CChartViewDlg;

public:
    inline double GetValue (bool offer) { return offer ? _offer : _bid; }

    void Dump (CFile &file);
    void Read (CString &line);

    LivePricePoint ();
    LivePricePoint (int hour, int min, int sec, int day, int month, int year);
};


class LivePrices
{
private:
    std::vector <LivePricePoint *> _livepricepoints;

    friend class TechnicalIndicator;
    friend class LivePricesSet;

public:
    void CleanUp ();

    LivePricePoint *GetTheLivePricePoint ();
    void Get_HLOC (double &high, double &low, double &open, double &close);

    static void Get_HLOC (std::list <LivePrices *> &liveprices, double &high, double &low, double &open, double &close);

    LivePricePoint *GetNewLivePricePoint ();
    bool DataToDump ();
    void Dump (CFile &file, int index);
};


class Ticker;


class LivePricesSet
{
private:
    CString GetLivePricesLogFile (int day, int month, int year);
    CString GetLivePricesLogFile (const CTime &time);

private:
    IB_PTraderDlg *_parent;
    Ticker *_parentticker;

public:
    inline IB_PTraderDlg *Parent () { return _parent; }
    inline Ticker *ParentTicker () { return _parentticker; }

private:
    std::vector <LivePrices *> _liveprices;

    friend class TechnicalIndicator;

public:
    LivePrices *operator [] (size_t i) { return _liveprices [i]; }

    void CleanUp ();

    LivePrices *PriceChange ();
    void NewLivePrice ();

    void LoadDataSet (int day, int month, int year);
    void LoadDataSet (const CTime &time);

    LivePrices *_lastiterliveprices;
    int StartIter ();
    LivePrices *GetNextIter (int &iter);
    LivePrices *GetNextIterNoSkip (int &iter);
    
    LivePricesSet (IB_PTraderDlg *parent, Ticker *parentticker, bool loadtodaysdataset);
};

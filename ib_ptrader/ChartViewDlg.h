/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include "chartviewer.h"
#include "LivePriceLogger.h"


// CChartViewDlg dialog

class CChartViewDlg : public CDialog
{
    DECLARE_DYNAMIC(CChartViewDlg)

private:
    IB_PTraderDlg *_parent;

public:
    CChartViewDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CChartViewDlg();

// Dialog Data
    enum { IDD = IDD_CHARTVIEW_DLG };

protected:
    HICON m_hIcon;

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    CChartViewer m_ChartViewer;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

private:
    void drawChart(CChartViewer *viewer);
    void trackFinance(MultiChart *m, int mouseX);

public:
    enum ChartKind
    {
        Live, Historic
    };

    enum ChartPeriod
    {
        Sec30, Min1, Min2point5
    };

    static CString ToString (ChartKind chartkind)
    {
        switch (chartkind)
        {
            case Live: { return "Live"; }
            case Historic: { return "Historic"; }
        }
        return "?";
    }

    static CString ToDisplayString (ChartPeriod chartperiod)
    {
        switch (chartperiod)
        {
            case Sec30: { return "30 Sec"; }
            case Min1: { return "1 Min"; }
            case Min2point5: { return "2.5 Min"; }
        }
        return "?";
    }

    int GetCountDown ()
    {
        switch (_chartperiod)
        {
            case Sec30:
                return 1;
            case Min1:
                return 2;
            case Min2point5:
                return 5;        
        }
        return 1;
    }

private:
    ChartKind _chartkind;
    ChartPeriod _chartperiod;

    int _receiveprice;
    std::list <LivePrices *> _receiveliveprices;
    bool ProceedWithPrice (LivePrices *liveprices);

    int _day, _month, _year;
    double _minvalue, _maxvalue;

    Ticker *_owningticker;

public:
    void SetChartKind (ChartKind chartkind, ChartPeriod chartperiod, Ticker *owningticker, int day = 0, int month = 0, int year = 0);

private:
    LivePricesSet *_livepricesset;
    bool _deletelivepricesset;

public:
    void SetLivePrices (LivePricesSet *livepricesset, bool drawchart, bool deletelivepricesset);

    static CString GetWindowTitle (IB_PTraderDlg *parent, Ticker *ticker, ChartKind chartkind, ChartPeriod chartperiod, int day = 0, int month = 0, int year = 0);

    #pragma region "DataPointsForGraph"
private:
    class DataPointsForGraph
    {
    public:
        const int MAX_DOUBLEARRAY_CAPACITY;

        enum DataPointType
        {
            Highs, Lows, Opens, Closes, Times, Volumes
        };

        typedef std::map <DataPointType, DoubleArray *>::iterator DataPointIter;
        std::map <DataPointType, DoubleArray *> _datapoints;

        DoubleArray *GetDoubleArray (DataPointType datapointtype);
        DoubleArray *AddData (DataPointType datapointtype, double newvalue);

        void CleanUp ();

        DataPointsForGraph ();
        ~DataPointsForGraph ();
    };

    DataPointsForGraph _datapointsforgraph;

public:
    void LiveUpdate (LivePrices *mostrecentiveprice);
    #pragma endregion

protected:
    virtual void PostNcDestroy();
    virtual void OnCancel();
};

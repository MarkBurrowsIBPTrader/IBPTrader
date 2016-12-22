/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "ChartViewDlg.h"
#include "FinanceChart.h"
#include <memory>
#include <sstream>



// CChartViewDlg dialog

IMPLEMENT_DYNAMIC(CChartViewDlg, CDialog)

CChartViewDlg::CChartViewDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CChartViewDlg::IDD, pParent), _parent ((IB_PTraderDlg *) pParent), _owningticker (0), _livepricesset (0), _receiveprice (0),
                                                _minvalue (DBL_MAX), _maxvalue (DBL_MIN)

{
}


CChartViewDlg::~CChartViewDlg ()
{
}


void CChartViewDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHARTBITMAP, m_ChartViewer);
}


BEGIN_MESSAGE_MAP(CChartViewDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CChartViewDlg message handlers

BOOL CChartViewDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    SetWindowText (GetWindowTitle (_parent, _owningticker, _chartkind, _chartperiod, _day, _month, _year));

    drawChart (&m_ChartViewer);

    CRect winSize;
    m_ChartViewer.GetWindowRect(winSize);
    m_ChartViewer.MoveWindow(5, 5, winSize.Width(), winSize.Height());

    CalcWindowRect(&winSize, CWnd::adjustBorder);
    SetWindowPos(&wndTop, 0, 0, winSize.Width() + 10, winSize.Height() + 10, SWP_NOMOVE);
 
    return TRUE;  // return TRUE  unless you set the focus to a control
}


void CChartViewDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}


HCURSOR CChartViewDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


BOOL CChartViewDlg::OnEraseBkgnd(CDC* pDC)
{
    //Setbrushtodesiredbackgroundcolor
    CBrush backBrush(RGB(255,255,255));

    //Saveoldbrush
    CBrush *pOldBrush=pDC->SelectObject(&backBrush);

    //Erasetheareaneededwiththegivenbackgroundcolor
    CRect rect;
    pDC->GetClipBox(&rect);
    pDC->PatBlt(rect.left,rect.top,rect.Width(),rect.Height(),PATCOPY);

    //Restoreoldbrushandexit
    pDC->SelectObject(pOldBrush);
    return TRUE;
}


bool CChartViewDlg::ProceedWithPrice (LivePrices *liveprices)
{
    _receiveliveprices.push_back (liveprices);
    _receiveprice++;
    int thresold =  GetCountDown ();
    if (_receiveprice >= thresold)
    {
        _receiveprice = 0;
        return true;
    }
    return false;
}


void CChartViewDlg::SetChartKind (ChartKind chartkind, ChartPeriod chartperiod, Ticker *owningticker, int day, int month, int year)
{
    _chartkind = chartkind;
    _chartperiod = chartperiod;
    _owningticker = owningticker;
    _day = day;
    _month = month;
    _year = year;
}


#define WANT_ACCOUNT_DETAILS 1

CString CChartViewDlg::GetWindowTitle (IB_PTraderDlg *parent, Ticker *ticker, ChartKind chartkind, ChartPeriod chartperiod, int day, int month, int year)
{
    CString title = ticker->TickerName ().c_str () + CString (" ") + ToDisplayString (chartperiod) + CString (" ") + 
                    ToString (chartkind) + CString (" Chart Data");
    if (chartkind == Historic)
    {
        CString format;
        format.Format (" on %d/%d/%d", day, month, year);
        title += format;
    }
#if WANT_ACCOUNT_DETAILS
    title += " (" + CString (parent->UserName ().c_str ()) + ")";
#endif
    return title;
}


void CChartViewDlg::LiveUpdate (LivePrices *mostrecentiveprice)
{
    if (! ProceedWithPrice (mostrecentiveprice))
    {
        return;
    }

    double high, low, open, close;
    LivePrices::Get_HLOC (_receiveliveprices, high, low, open, close);
    _receiveliveprices.clear ();
    _datapointsforgraph.AddData (DataPointsForGraph::Highs, high);
    _datapointsforgraph.AddData (DataPointsForGraph::Lows, low);
    _datapointsforgraph.AddData (DataPointsForGraph::Opens, open);
    _datapointsforgraph.AddData (DataPointsForGraph::Closes, close);

    if (close < _minvalue)
        _minvalue = close;
    if (close > _maxvalue)
        _maxvalue = close;

    LivePricePoint *thepricepoint = mostrecentiveprice->GetTheLivePricePoint ();
    double time = Chart::chartTime (thepricepoint->_year, thepricepoint->_month, thepricepoint->_day,
                                    thepricepoint->_hour, thepricepoint->_min, thepricepoint->_sec);
    _datapointsforgraph.AddData (DataPointsForGraph::Times, time);
    _datapointsforgraph.AddData (DataPointsForGraph::Volumes, (double) thepricepoint->_volume);

    drawChart (&m_ChartViewer);
}


DoubleArray ToDoubleArray (std::list <double> &list)
{
    int size = (int) list.size ();
    double *thearray = new double [size];
    int index = 0;
    for each (double d in list)
    {
        thearray [index++] = d;
    }
    return DoubleArray (thearray, size);
}


void FillDoubleArray (std::list <double> &list, DoubleArray *doublearray)
{
    int index = 0;
    for each (double d in list)
    {
        double *buffer = const_cast <double *> (doublearray->data);
        buffer [index++] = d;
    }
    doublearray->len = index;
}



void CChartViewDlg::SetLivePrices (LivePricesSet *livepricesset, bool drawchart, bool deletelivepricesset)
{
    _livepricesset = livepricesset;
    _deletelivepricesset = deletelivepricesset;
    std::list <double> high_list, low_list, open_list, close_list, timestamp_list, volume_list;
    int iter = _livepricesset->StartIter ();
    LivePrices *liveprices;
    while ((liveprices = _livepricesset->GetNextIterNoSkip (iter)) != 0)
    {
        if (! ProceedWithPrice (liveprices))
        {
            continue;
        }
        double high, low, open, close;
        LivePrices::Get_HLOC (_receiveliveprices, high, low, open, close);
        _receiveliveprices.clear ();
        high_list.push_back (high);
        low_list.push_back (low);
        open_list.push_back (open);
        close_list.push_back (close);

        if (close < _minvalue)
            _minvalue = close;
        if (close > _maxvalue)
            _maxvalue = close;

        LivePricePoint *thepricepoint = liveprices->GetTheLivePricePoint ();
        double time = Chart::chartTime (thepricepoint->_year, thepricepoint->_month, thepricepoint->_day,
                                        thepricepoint->_hour, thepricepoint->_min, thepricepoint->_sec);
        timestamp_list.push_back (time);
        volume_list.push_back ((double) thepricepoint->_volume);
    }

    size_t expectedlen = high_list.size ();
    if (low_list.size () == expectedlen && 
        open_list.size () == expectedlen && 
        close_list.size () == expectedlen && 
        timestamp_list.size () == expectedlen && 
        volume_list.size () == expectedlen)
    {
        FillDoubleArray (high_list, _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Highs));
        FillDoubleArray (low_list, _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Lows));
        FillDoubleArray (open_list, _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Opens));
        FillDoubleArray (close_list, _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Closes));
        FillDoubleArray (timestamp_list, _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Times));
        FillDoubleArray (volume_list, _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Volumes));
    }

    if (drawchart)
        drawChart (&m_ChartViewer);
}


void CChartViewDlg::drawChart (CChartViewer *viewer)
{
    // Create a FinanceChart object of width 1300 pixels
    std::auto_ptr <FinanceChart> c (new FinanceChart (1300));

    DoubleArray *times = _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Times);
    DoubleArray *closes = _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Closes);

    // Add a title to the chart
    CString title, format;
    switch (_chartkind)
    {
        case Live:
        {
            CTime now = CTime::GetCurrentTime ();
            title.Format ("Live %s at %0.2d:%0.2d:%0.2d, %d data points", 
                           _owningticker->TickerName ().c_str (), now.GetHour (), now.GetMinute (), now.GetSecond (),
                           times->len);
            bool fp = true;
            for (int i = 0;  i < 5;  i++)
            {
                int index = closes->len - i - 1;
                if (index >= 0)
                {
                    if (fp)
                    {
                        title += " <";
                        fp = false;
                    }
                    else
                    {
                        title += ", ";
                    }
                    format.Format ("%0.2f", closes->data [index]);
                    title += format;
                }
            }
            if (! fp)
                title += ">";
            break;
        }
        case Historic:
        {
            title = CString ("Historic ") + _owningticker->TickerName ().c_str ();
            break;
        }
        default:
        {
            title = "?";
            break;
        }
    }

    format.Format (", Min=%0.2f, Max=%0.2f", _minvalue, _maxvalue);
    if (format.GetLength () < 30)
        title += format;
    c->addTitle ((LPCTSTR) title);

    // Disable default legend box, as we are using dynamic legend
    c->setLegendStyle ("normal", 8, Chart::Transparent, Chart::Transparent);

    // Set the data into the finance chart object
    DoubleArray *highs = _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Highs);
    DoubleArray *lows = _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Lows);
    DoubleArray *opens = _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Opens);
    DoubleArray *volumes = _datapointsforgraph.GetDoubleArray (DataPointsForGraph::Volumes);

    c->setData (*times, *highs, *lows, *opens, *closes, *volumes, 0);

    c->addFastStochastic (75, 14, 3, 0x006060, 0x606000);

    c->addSlowStochastic (75, 14, 3, 0x006060, 0x606000);

    c->addMainChart (240);

    c->addSimpleMovingAvg (10, 0x663300);

    // Add a 20 period simple moving average to the main chart, using purple color
    c->addSimpleMovingAvg (20, 0x9900ff);

    // Add candlestick symbols to the main chart, using green/red for up/down days
    c->addCandleStick (0x00ff00, 0xff0000);

    // Add 20 days bollinger band to the main chart, using light blue (9999ff) as the border and
    // semi-transparent blue (c06666ff) as the fill color
    c->addBollingerBand (20, 2, 0x9999ff, 0xc06666ff);

    // Add a 75 pixels volume bars sub-chart to the bottom of the main chart, using green/red/grey for
    // up/down/flat days
    c->addVolBars (75, 0x99ff99, 0xff9999, 0x808080);

    // Append a 14-days RSI indicator chart (75 pixels high) after the main chart. The main RSI line
    // is purple (800080). Set threshold region to +/- 20 (that is, RSI = 50 +/- 25). The upper/lower
    // threshold regions will be filled with red (ff0000)/blue (0000ff).
    c->addRSI (75, 14, 0x800080, 20, 0xff0000, 0x0000ff);

    // Append a MACD(26, 12) indicator chart (75 pixels high) after the main chart, using 9 days for
    // computing divergence.
    //c->addMACD(75, 26, 12, 9, 0x0000ff, 0xff00ff, 0x008000);

    //c->addTRIX(75, 15, 0x0000ff);

    c->addCCI (100, 20, 0x800080, 200, 0xff00ff, 0x008000);

    c->addWilliamR (100, 14, 200, 0xff00ff, 0x0080009, 0xff9999); 

    trackFinance (c.get (), ((XYChart *) c->getChart (0))->getPlotArea ()->getRightX ());
    
    viewer->setChart (c.get ());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2012 Advanced Software Engineering Limited
//
// You may use and modify the code in this file in your application, provided the code and
// its modifications are used only in conjunction with ChartDirector. Usage of this software
// is subjected to the terms and condition of the ChartDirector license.
///////////////////////////////////////////////////////////////////////////////////////////////////
void CChartViewDlg::trackFinance (MultiChart *m, int mouseX)
{
    // Clear the current dynamic layer and get the DrawArea object to draw on it.
    DrawArea *d = m->initDynamicLayer ();

    // It is possible for a FinanceChart to be empty, so we need to check for it.
    if (m->getChartCount () == 0)
        return ;

    // Get the data x-value that is nearest to the mouse
    int xValue = (int) (((XYChart *) m->getChart (0))->getNearestXValue (mouseX));

    // Iterate the XY charts (main price chart and indicator charts) in the FinanceChart
    XYChart *c = 0;
    for(int i = 0; i < m->getChartCount(); ++i) {
        c = (XYChart *)m->getChart(i);

        // Variables to hold the legend entries
        std::ostringstream ohlcLegend;
        std::vector<std::string> legendEntries;

        // Iterate through all layers to find the highest data point
        for(int j = 0; j < c->getLayerCount(); ++j) {
            Layer *layer = c->getLayerByZ(j);
            int xIndex = layer->getXIndexOf(xValue);
            int dataSetCount = layer->getDataSetCount();

            // In a FinanceChart, only layers showing OHLC data can have 4 data sets
            if (dataSetCount == 4) {
                double highValue = layer->getDataSet(0)->getValue(xIndex);
                double lowValue = layer->getDataSet(1)->getValue(xIndex);
                double openValue = layer->getDataSet(2)->getValue(xIndex);
                double closeValue = layer->getDataSet(3)->getValue(xIndex);

                if (closeValue != Chart::NoValue) {
                    // Build the OHLC legend
					ohlcLegend << "      <*block*>";
					ohlcLegend << "Open: " << c->formatValue(openValue, "{value|P4}");
					ohlcLegend << ", High: " << c->formatValue(highValue, "{value|P4}"); 
					ohlcLegend << ", Low: " << c->formatValue(lowValue, "{value|P4}"); 
					ohlcLegend << ", Close: " << c->formatValue(closeValue, "{value|P4}");

                    // We also draw an upward or downward triangle for up and down days and the %
                    // change
                    double lastCloseValue = layer->getDataSet(3)->getValue(xIndex - 1);
                    if (lastCloseValue != Chart::NoValue) {
                        double change = closeValue - lastCloseValue;
                        double percent = change * 100 / closeValue;
                        std::string symbol = (change >= 0) ?
                            "<*font,color=008800*><*img=@triangle,width=8,color=008800*>" :
                            "<*font,color=CC0000*><*img=@invertedtriangle,width=8,color=CC0000*>";

                        ohlcLegend << "  " << symbol << " " << c->formatValue(change, "{value|P4}");
						ohlcLegend << " (" << c->formatValue(percent, "{value|2}") << "%)<*/font*>";
                    }

					ohlcLegend << "<*/*>";
                }
            } else {
                // Iterate through all the data sets in the layer
                for(int k = 0; k < layer->getDataSetCount(); ++k) {
                    DataSet *dataSet = layer->getDataSetByZ(k);

                    std::string name = dataSet->getDataName();
                    double value = dataSet->getValue(xIndex);
                    if ((0 != name.size()) && (value != Chart::NoValue)) {

                        // In a FinanceChart, the data set name consists of the indicator name and its
                        // latest value. It is like "Vol: 123M" or "RSI (14): 55.34". As we are
                        // generating the values dynamically, we need to extract the indictor name
                        // out, and also the volume unit (if any).

						// The volume unit
						std::string unitChar;

                        // The indicator name is the part of the name up to the colon character.
						int delimiterPosition = (int)name.find(':');
                        if (name.npos != delimiterPosition) {
							
							// The unit, if any, is the trailing non-digit character(s).
							int lastDigitPos = (int)name.find_last_of("0123456789");
							if ((name.npos != lastDigitPos) && (lastDigitPos + 1 < (int)name.size()) &&
								(lastDigitPos > delimiterPosition))
								unitChar = name.substr(lastDigitPos + 1);

							name.resize(delimiterPosition);
                        }

                        // In a FinanceChart, if there are two data sets, it must be representing a
                        // range.
                        if (dataSetCount == 2) {
                            // We show both values in the range in a single legend entry
                            value = layer->getDataSet(0)->getValue(xIndex);
                            double value2 = layer->getDataSet(1)->getValue(xIndex);
                            name = name + ": " + c->formatValue(min(value, value2), "{value|P3}");
							name = name + " - " + c->formatValue(max(value, value2), "{value|P3}");
                        } else {
                            // In a FinanceChart, only the layer for volume bars has 3 data sets for
                            // up/down/flat days
                            if (dataSetCount == 3) {
                                // The actual volume is the sum of the 3 data sets.
                                value = layer->getDataSet(0)->getValue(xIndex) + layer->getDataSet(1
                                    )->getValue(xIndex) + layer->getDataSet(2)->getValue(xIndex);
                            }

                            // Create the legend entry
                            name = name + ": " + c->formatValue(value, "{value|P3}") + unitChar;
                        }

                        // Build the legend entry, consist of a colored square box and the name (with
                        // the data value in it).
						std::ostringstream legendEntry;
						legendEntry << "<*block*><*img=@square,width=8,edgeColor=000000,color=" 
							<< std::hex << dataSet->getDataColor() << "*> " << name << "<*/*>";
                        legendEntries.push_back(legendEntry.str());
                    }
                }
            }
        }

        // Get the plot area position relative to the entire FinanceChart
        PlotArea *plotArea = c->getPlotArea();
        int plotAreaLeftX = plotArea->getLeftX() + c->getAbsOffsetX();
        int plotAreaTopY = plotArea->getTopY() + c->getAbsOffsetY();

		// The legend begins with the date label, then the ohlcLegend (if any), and then the
		// entries for the indicators.
		std::ostringstream legendText;
		legendText << "<*block,valign=top,maxWidth=" << (plotArea->getWidth() - 5) 
			<< "*><*font=arialbd.ttf*>[" << c->xAxis()->getFormattedLabel(xValue, "mmm dd, yyyy")
			<< "]<*/font*>" << ohlcLegend.str();
		for (int i = ((int)legendEntries.size()) - 1; i >= 0; --i) {
			legendText << "      " << legendEntries[i];
		}
		legendText << "<*/*>";

        // Draw a vertical track line at the x-position
        d->vline(plotAreaTopY, plotAreaTopY + plotArea->getHeight(), c->getXCoor(xValue) +
            c->getAbsOffsetX(), d->dashLineColor(0x000000, 0x0101));

        // Display the legend on the top of the plot area
        TTFText *t = d->text(legendText.str().c_str(), "arial.ttf", 8);
        t->draw(plotAreaLeftX + 5, plotAreaTopY + 3, 0x000000, Chart::TopLeft);
		t->destroy();
    }
}


void CChartViewDlg::PostNcDestroy()
{
    // TODO: Add your specialized code here and/or call the base class
    CDialog::PostNcDestroy ();
    delete this;
}


void CChartViewDlg::OnCancel()
{
    // TODO: Add your specialized code here and/or call the base class
    if (_deletelivepricesset)
    {
        _livepricesset->CleanUp ();
        delete _livepricesset;
    }
    _datapointsforgraph.CleanUp ();
    DestroyWindow ();
}

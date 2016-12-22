/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "TechnicalAnalysis.h"
#include "Ticker.h"



void IB_PTraderDlg::SetTickersTA (std::string ticker, std::string exchange, std::string sectype, TechnicalIndicator::Indicators TAindicator)
{
    Ticker *foundticker = _tickers ? _tickers->GetTickerByContract (ticker, exchange, sectype) : 0;
    if (foundticker)
    {
        if (foundticker->_livepriceset == 0)
        {
            Log (CString (ticker.c_str ()) + " NOT SET for TA (live prices not set)");
            _stats.IncrementStat ("SetTickersTA", "LivePricesNotSet", ticker, "");
            return;
        }
        if (foundticker->_technicalindicators == 0)
        {
            foundticker->_technicalindicators = new TechnicalAnalysisSet (foundticker);
        }
        TechnicalIndicator *technicalindicator = 0;
        switch (TAindicator)
        {
            case TechnicalIndicator::RSI_14:
                technicalindicator = new RSI (TAindicator, foundticker, 14);
                break;
            case TechnicalIndicator::StochRSI14:
                technicalindicator = new StochRSI (TAindicator, foundticker, 14);
                break;
            case TechnicalIndicator::TSI:
                technicalindicator = new TrueSI (TAindicator, foundticker);
                break;
            case TechnicalIndicator::Stochs14_7_7:
                technicalindicator = new Stochs (TAindicator, foundticker, 14, 7, 7);
                break;
            case TechnicalIndicator::Stochs14_3_3:
                technicalindicator = new Stochs (TAindicator, foundticker, 14, 3, 3);
                break;
#if ! PUBLIC_BUILD
            case TechnicalIndicator::DailyStochs:
                technicalindicator = new DailyStochs (TAindicator, foundticker, "c:\\data\\traderstats.txt");
                break;
#endif
            case TechnicalIndicator::Williams_45:
                technicalindicator = new Williams (TAindicator, foundticker, 45);
                break;
        }
        if (technicalindicator)
        {
            foundticker->_technicalindicators->AddIndicator (technicalindicator);
        }
        else
        {
            _stats.IncrementStat ("SetTickersTA", "TechnicalIndicatorNotFound", ticker, "");
        }
    }
    else
    {
        Log (CString (ticker.c_str ()) + " NOT SET for TA");
        _stats.IncrementStat ("SetTickersTA", "TickerNotFound", ticker, "");
    }
}

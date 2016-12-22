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
#include <math.h>



#pragma region "TechnicalIndicator"
double TechnicalIndicator::LatestValue (bool &defaultvalue)
{
    size_t size = _results.size ();
    if (size == 0)
    {
        defaultvalue = true;
        return 100;
    }
    double result = _results [size - 1];
    if (result == 0 || result >= 100)
    {
        defaultvalue = true;
    }
    else
    {
        defaultvalue = false;
    }
    return result;
}


void TechnicalIndicator::AddLatestValue (double value)
{
    _results.push_back (value);
    bool defaultvalue;
    value = LatestValue (defaultvalue);
    if (! defaultvalue)
    {
        if (value < _minvalue)
        {
            _minvalueset = true;
            _minvalue = value;
        }
        if (value > _maxvalue)
        {
            _maxvalueset = true;
            _maxvalue = value;
        }
    }
}


void TechnicalIndicator::GetOffers (std::vector <double> &offers, int numberofprices)
{
    int iter;
    if (numberofprices == -1)
    {
        iter = _ticker->GetLivePricesSet ()->StartIter ();
        LivePrices *liveprices;
        while ((liveprices = _ticker->GetLivePricesSet ()->GetNextIter (iter)) != 0)
        {
            for each (LivePricePoint *pricepoint in liveprices->_livepricepoints)
            {
                offers.insert (offers.begin (), pricepoint->GetValue (true));
            }
        }
    }
    else
    {
        iter = (int) _ticker->GetLivePricesSet ()->_liveprices.size () - 1;
        while (iter >= 0)
        {
            LivePrices *liveprices = _ticker->GetLivePricesSet ()->_liveprices [iter];
            for each (LivePricePoint *pricepoint in liveprices->_livepricepoints)
            {
                offers.insert (offers.begin (), pricepoint->GetValue (true));
            }
            if ((int) offers.size () >= numberofprices)
                return;            
            iter--;
        }
    }
}


bool TechnicalIndicator::DisplayIndicator ()
{
    return true;
}


TechnicalIndicator::BuyIndicator TechnicalIndicator::BuyAction ()
{
    return TechnicalIndicator::NoAction;
}


void TechnicalIndicator::Calculate (TechnicalAnalysisSet *set)
{
}


TechnicalIndicator::TechnicalIndicator (Indicators technicalindicator, std::string name, Ticker *ticker) : _technicalindicator (technicalindicator), 
                                                                                                           _name (name), _ticker (ticker),
                                                                                                           _minvalueset (false), _minvalue (DBL_MAX), 
                                                                                                           _maxvalueset (false), _maxvalue (DBL_MIN)

{
}
#pragma endregion


#pragma region "RSI"
TechnicalIndicator::BuyIndicator RSI::BuyAction ()
{
    size_t size = Results ().size ();
    if (size < 20)
        return TechnicalIndicator::NotEnoughData;
    bool defaultvalue;
    double latestvalue = LatestValue (defaultvalue);
    if (latestvalue < 30)
        return TechnicalIndicator::Buy;
    return TechnicalIndicator::NoAction;
}


void RSI::GetGainsLosses (std::vector <double> &data, int numberofdays, double &gains, double &losses)
{
    gains = 0;
    losses = 0;
    int upperbound = numberofdays >= (int) data.size () ? (int) (data.size () - 1) : numberofdays;
    for (int i = 0;  i < upperbound;  i++)
    {
        double today = data [i];
        double previousday = data [i + 1];
        if (today > previousday)
        {
            gains += (today - previousday);
        }
        else if (today < previousday)
        {
            losses += (previousday - today);
        }
    }
}


double RSI::GetRS (double gains, double losses)
{
    if (losses == 0)
        return 100;
    return gains / losses;
}


void RSI::GetRange (std::vector <double> &pairs, int index, int count, std::vector <double> &result)
{
    for (int i = 0;  i < count;  i++)
    {
        result.push_back (pairs [index++]);
    }
}


void RSI::GetRSs (std::vector <double> &pairs, int PERIOD, std::vector <double> &rsset)
{
    int startindex = (int) (pairs.size () - PERIOD - 1);
    if (startindex < 0)
    {
        startindex = 0;
        PERIOD = (int) (pairs.size () - 1);
    }
    std::vector <double> firstset;
    GetRange (pairs, startindex, PERIOD + 1, firstset);
    double gains, losses;
    GetGainsLosses (firstset, (int) firstset.size (), gains, losses);
    if (PERIOD > 0)
        gains /= PERIOD;
    else
        gains = 0;
    if (PERIOD > 0)
        losses /= PERIOD;
    else
        losses = 0;
    double rs = GetRS (gains, losses);
    rsset.push_back (rs);
    for (int i = startindex - 1;  i >= 0;  i--)
    {
        double today = pairs [i];
        double previousday = pairs [i + 1];

        double currentgain, currentloss;
        if (today > previousday)
        {
            currentgain = today - previousday;
            currentloss = 0;
        }
        else if (today < previousday)
        {
            currentloss = previousday - today;
            currentgain = 0;
        }
        else
        {
            currentgain = 0;
            currentloss = 0;
        }
        gains = ((gains * (PERIOD - 1)) + currentgain) / PERIOD;
        losses = ((losses * (PERIOD - 1)) + currentloss) / PERIOD;
        rs = GetRS (gains, losses);
        rsset.push_back (rs);
    }
}


double RSI::CalculateRSIFromRS (double rs)
{
    return 100 - (100 / (1 + rs));
}


void RSI::Calculate (TechnicalAnalysisSet *set)
{
    std::vector <double> offers;
    GetOffers (offers, _period + 5);
    std::vector <double> rsset;
    GetRSs (offers, _period, rsset);
    double rsi = CalculateRSIFromRS (rsset [rsset.size () - 1]); 
    AddLatestValue (rsi);
}


RSI::RSI (Indicators technicalindicator, Ticker *ticker, int period) : TechnicalIndicator (technicalindicator, "RSI", ticker), _period (period)
{
}
#pragma endregion


#pragma region "StochRSI"
TechnicalIndicator::BuyIndicator StochRSI::BuyAction ()
{
    size_t size = Results ().size ();
    if (size < 20)
        return TechnicalIndicator::NotEnoughData;
    bool defaultvalue;
    double latestvalue = LatestValue (defaultvalue);
    if (latestvalue < 0.2)
        return TechnicalIndicator::Buy;
    return TechnicalIndicator::NoAction;
}


void StochRSI::GetMinMax (std::vector <double> &data, int period, double &min, double &max)
{
    min = DBL_MAX;
    max = DBL_MIN;
    int datalen = (int) data.size ();
    if (datalen < period)
        return;
    int index = datalen - 1;
    for (int i = 0;  i < period;  i++)
    {
        double value = data [index--];
        if (value < min)
            min = value;
        if (value > max)
            max = value;
    }
}


void StochRSI::Calculate (TechnicalAnalysisSet *set)
{
    TechnicalIndicator *ti = set->GetIndicator (TechnicalIndicator::RSI_14);
    if (ti)
    {
        double min, max;
        GetMinMax (ti->Results (), _period, min, max);
        bool defaultvalue;
        double rsi = ti->LatestValue (defaultvalue);
        double stochrsi = (rsi - min) / (max - min);
        AddLatestValue (stochrsi);
    }
}


StochRSI::StochRSI (Indicators technicalindicator, Ticker *ticker, int period) : TechnicalIndicator (technicalindicator, "StochRSI", ticker), _period (period)
{
}
#pragma endregion


#pragma region "TrueSI"
TechnicalIndicator::BuyIndicator TrueSI::BuyAction ()
{
    size_t size = Results ().size ();
    if (size < 20)
        return TechnicalIndicator::NotEnoughData;
    bool defaultvalue;
    double latestvalue = LatestValue (defaultvalue);
    if (latestvalue < 0)
        return TechnicalIndicator::Buy;
    return TechnicalIndicator::NoAction;
}


double TechnicalIndicator::GetSMA (std::vector <double> &data, int start, int numberofdays)
{
    double sum = 0;
    int upperbound = __min ((int) data.size (), numberofdays);
    for (int i = 0;  i < upperbound;  i++)
    {
        sum += data [i];
    }
    return sum / numberofdays;
}


int TechnicalIndicator::GetEMAListStart (double datacount, double emaperiod)
{
    if (emaperiod > datacount)
    {
        return 0;
    }
    else
    {
        return (int) (datacount - emaperiod);
    }
}


double TechnicalIndicator::CalculateEMA (double close, double yesterdayclose, double k)
{
    return (close * k) + (yesterdayclose * (1 - k));
}


void TechnicalIndicator::GetEMAList (std::vector <double> &data, double emaperiod, std::vector <double> &emas)
{
    double k = 2 / (emaperiod + 1);
    int datasize = (int) data.size ();
    int start = GetEMAListStart ((double) datasize, emaperiod);
    double yema = GetSMA (data, start, (int) emaperiod);
    for (int i = start - 1;  i >= 0;  i--)
    {
        double ema = CalculateEMA (data [i], yema, k);
        emas.insert (emas.begin (), ema);
        yema = ema;
    }
    if (emas.size () == 0)
    {
        emas.push_back (yema);
    }
}


double TechnicalIndicator::GetEMA (std::vector <double> &data, double emaperiod)
{
    std::vector <double> emas;
    GetEMAList (data, emaperiod, emas);
    return emas [0];
}


double TrueSI::CalculateTSI (std::vector <double> &data, int start, double r, double s, bool &calculated)
{
    calculated = false;
    std::vector <double> mtms;
    int datasize = (int) data.size ();
    for (int i = start;  i < datasize - 1;  i++)
    {
        double mtm = data [i] - data [i + 1];
        mtms.push_back (mtm);
    }
    std::vector <double> ema_mtm;
    GetEMAList (mtms, r, ema_mtm);

    std::vector <double> abs_mtms;
    for (int i = start;  i < datasize - 1;  i++)
    {
        double abs_mtm = ::abs (data [i] - data [i + 1]);
        abs_mtms.push_back (abs_mtm);
    }
    std::vector <double> ema_absmtm;
    GetEMAList (abs_mtms, r, ema_absmtm);

    double s_for_mtm = GetEMA (ema_mtm, s); 
    double s_for_absmtm = GetEMA (ema_absmtm, s);
    if (s_for_absmtm != 0)
    {
        double tsi = 100 * (s_for_mtm / s_for_absmtm);
        calculated = true;
        return tsi;
    }
    return 0;
}


void TrueSI::Calculate (TechnicalAnalysisSet *set)
{
    std::vector <double> offers;
    GetOffers (offers, 30);
    bool calculated;
    double tsi = CalculateTSI (offers, 0, 13, 25, calculated);
    if (calculated)
    {
        AddLatestValue (tsi);
    }
}


TrueSI::TrueSI (Indicators technicalindicator, Ticker *ticker) : TechnicalIndicator (technicalindicator, "TSI", ticker)
{
}
#pragma endregion


#pragma region "Stochs"
TechnicalIndicator::BuyIndicator Stochs::BuyAction ()
{
    size_t size = Results ().size ();
    if (size < 20)
        return TechnicalIndicator::NotEnoughData;

    bool defaultvalue;
    double latestvalue = LatestValue (defaultvalue);

    double K;
    double D = ::modf (latestvalue, &K);
    D *= 100;

    if ((K > 0 && K <= 20) && (D > 0 && D <= 20))
        return TechnicalIndicator::Buy;

    return TechnicalIndicator::NoAction;
}


void Stochs::SmoothOut (std::vector <double> &data, int smoothperiod, std::vector <double> &results)
{
    results.clear ();
    int countdown = (int) data.size () - smoothperiod;
    for (int i = 0;  i <= countdown;  i++)
    {
        double r = GetSMA (data, i, smoothperiod);
        results.push_back (r);
    }
}


bool Stochs::Get_LowestLow_HighestHigh (std::vector <double> &data, int startindex, int numberofdays, double &lowestlow, double &highesthigh)
{
    bool loset = false, hiset = false;
    lowestlow = DBL_MAX;
    highesthigh = DBL_MIN;
    int maxdays;
    if ((startindex + numberofdays) > (int) data.size ())
    {
        maxdays = (int) data.size ();
    }
    else
    {
        maxdays = startindex + numberofdays;
    }
    for (int i = startindex;  i < maxdays;  i++)
    {
        double value = data [i];
        if (value >= 0)
        {
            if (value < lowestlow)
            {
                lowestlow = value;
                loset = true;
            }
            if (value > highesthigh)
            {
                highesthigh = value;
                hiset = true;
            }
        }
    }
    return loset && hiset;
}


void Stochs::CalculateStochs (std::vector <double> &data, int startindex, int period, int smoothperiod, std::vector <double> &Ks, std::vector <double> &Ds)
{
    Ks.clear ();
    Ds.clear ();
    int countdown = period;
    while (countdown-- >= 0)
    {
        double lowestlow, highesthigh;
        if (Get_LowestLow_HighestHigh (data, startindex, period, lowestlow, highesthigh))
        {
            double rhs = (highesthigh - lowestlow);
            if (rhs > 0)
            {
                double K = (data [startindex] - lowestlow) / rhs;
                K *= 100;
                Ks.push_back (K);
            }
        }
        startindex++;
    }
    SmoothOut (data, smoothperiod, Ds);
}


void Stochs::Calculate (TechnicalAnalysisSet *set)
{
    const int LOOKBACKS = 
    20
    ;

    std::vector <double> offers;
    GetOffers (offers, _first_param + LOOKBACKS);

    std::vector <double> fullKs;
    for (int i = 0;  i < LOOKBACKS;  i++)
    {
        std::vector <double> Ks, Ds;
        CalculateStochs (offers, i, _first_param, _second_param, Ks, Ds);
        if ((int) Ks.size () > _x_smooth)
        {
            double fullK = GetSMA (Ks, 0, _x_smooth);
            fullKs.push_back (fullK);
        }
    }
    std::vector <double> fullDs;
    SmoothOut (fullKs, _x_smooth, fullDs);

    if (fullKs.size () > 0 && fullDs.size () > 0)
    {
        double Kvalue = fullKs [0];
        double Kint;
        ::modf (Kvalue, &Kint);

        double Dvalue = fullDs [0];
        double Dint;
        ::modf (Dvalue, &Dint);

        double result = Kint + (Dint / 100);
        
        AddLatestValue (result);
    }
}


Stochs::Stochs (Indicators technicalindicator, Ticker *ticker, int first_param, int second_param, int x_smooth) :
                                                                   TechnicalIndicator (technicalindicator, "Stochs", ticker),
                                                                   _first_param (first_param), _second_param (second_param), _x_smooth (x_smooth)

{
}
#pragma endregion


#pragma region "DailyStochs"
#if ! PUBLIC_BUILD
bool DailyStochs::DisplayIndicator ()
{
    return false;
}


TechnicalIndicator::BuyIndicator DailyStochs::BuyAction ()
{
    return TechnicalIndicator::Buy;
}


void DailyStochs::Calculate (TechnicalAnalysisSet *set)
{
}


double DailyStochs::GetValue (CString &text)
{
    CString value = text.Right (text.GetLength () - 4);
    return ::atof ((LPCTSTR) value);
}


DailyStochs::DailyStochs (Indicators technicalindicator, Ticker *ticker, std::string filename) : 
                                                                    TechnicalIndicator (technicalindicator, "DailyStochs", ticker),
                                                                    _filename (filename), _fileread (false),
                                                                    _filereaderror (false), _K (0), _D (0), _weekly_K (0), _weekly_D (0)
{
    _fileread = true;
    try
    {
        CFileException fileexcep;
        CStdioFile stdfile;
        if (stdfile.Open (_filename.c_str (), CFile::modeRead | CFile::modeNoTruncate, &fileexcep))
        {
            CString line;
            while (stdfile.ReadString (line))
            {
                CString token;
                for (int i = 0;  i < 6;  i++)
                {
                    if (AfxExtractSubString (token, line, i, ','))
                    {
                        if (i == 0)
                        {
                            if (ticker->TickerName () != (LPCTSTR) token)
                            {
                                break;
                            }
                        }
                        else if (i == 1)
                        {
                            if (ticker->FirstDBRow ()->_primaryexchange != (LPCTSTR) token)
                            {
                                _filereaderror = true;
                                break;
                            }
                        }
                        else
                        {
                            CString testtoken = token.Left (3);
                            if (testtoken == "d_K")
                            {
                                _K = GetValue (token);
                            }
                            else if (testtoken == "d_D")
                            {
                                _D = GetValue (token);
                            }
                            else if (testtoken == "w_K")
                            {
                                _weekly_K = GetValue (token);
                            }
                            else if (testtoken == "w_D")
                            {
                                _weekly_D = GetValue (token);
                            }
                            else
                            {
                                _filereaderror = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        _filereaderror = true;
                        break;
                    }
                }
            }
            stdfile.Close ();
        }
        else
        {
            _filereaderror = true;
        }
    }
    catch (...)
    {
        _filereaderror = true;
    }
}
#endif
#pragma endregion


#pragma region "Williams"
TechnicalIndicator::BuyIndicator Williams::BuyAction ()
{
    size_t size = Results ().size ();
    if (size < (size_t) _period)
        return TechnicalIndicator::NotEnoughData;
    bool defaultvalue;
    double latestvalue = LatestValue (defaultvalue);
    if (! defaultvalue && latestvalue <= -80)
        return TechnicalIndicator::Buy;
    return TechnicalIndicator::NoAction;
}


void Williams::Calculate (TechnicalAnalysisSet *set)
{
    std::vector <double> offers;
    GetOffers (offers, _period);
    double lowestlow, highesthigh;
    if (Stochs::Get_LowestLow_HighestHigh (offers, 0, (int) offers.size (), lowestlow, highesthigh))
    {
        double div = highesthigh - lowestlow;
        if (div != 0)
        {
            double R = ((highesthigh - offers [0]) / div) * -100;
            AddLatestValue (R);
        }
    }
}


Williams::Williams (Indicators technicalindicator, Ticker *ticker, int period) : TechnicalIndicator (technicalindicator, "R%", ticker), _period (period)
{
}
#pragma endregion


#pragma region "TechnicalAnalysisSet"
void TechnicalAnalysisSet::AddIndicator (TechnicalIndicator *technicalindicator)
{
    _technicalindicators.push_back (technicalindicator);
    _indicatorsbymap [technicalindicator->TechnicalIndicators ()] = technicalindicator;
}


TechnicalIndicator *TechnicalAnalysisSet::GetIndicator (TechnicalIndicator::Indicators indicator)
{
    IndicatorsIter iter = _indicatorsbymap.find (indicator);
    if (iter != _indicatorsbymap.end ())
    {
        return iter->second;
    }
    return 0;
}


void TechnicalAnalysisSet::Calculate ()
{
    if (_ticker->MinMaxSetBidOffer ())
    {
        for each (TechnicalIndicator *ta in _technicalindicators)
        {
            ta->Calculate (this);
        }
    }
}


TechnicalAnalysisSet::TechnicalAnalysisSet (Ticker *ticker) : _ticker (ticker)
{
}
#pragma endregion

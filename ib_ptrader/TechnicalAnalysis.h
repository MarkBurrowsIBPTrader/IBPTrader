/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include <list>
#include <vector>



class Ticker;
class TechnicalAnalysisSet;


class TechnicalIndicator
{
public:
    enum BuyIndicator
    {
        NotEnoughData, Buy, NoAction
    };

    static std::string ToString (BuyIndicator indicator)
    {
        switch (indicator)
        {
            case NotEnoughData: { return "NotEnoughData"; }
            case Buy: { return "Buy"; }
            case NoAction: { return "NoAction"; }
        }
        return "";
    }

    enum Indicators
    {
        RSI_14, StochRSI14, TSI, Stochs14_7_7, Stochs14_3_3, 
#if ! PUBLIC_BUILD
        DailyStochs,
#endif
        Williams_45
    };

    static std::string ToString (Indicators indicator)
    {
        switch (indicator)
        {
            case RSI_14: { return "RSI_14"; }
            case StochRSI14: { return "StochRSI14"; }
            case TSI: { return "TSI"; }
            case Stochs14_7_7: { return "Stochs14_7_7"; }
            case Stochs14_3_3: { return "Stochs14_3_3"; }
#if ! PUBLIC_BUILD
            case DailyStochs: { return "DailyStochs"; }
#endif
            case Williams_45: { return "Williams_45"; }
        }
        return "";
    }

    static CString ToDisplayString (Indicators indicator)
    {
        switch (indicator)
        {
            case RSI_14: { return "RSI"; }
            case StochRSI14: { return "stRSI"; }
            case TSI: { return "TSI"; }
            case Stochs14_7_7: { return "K7_D7"; }
            case Stochs14_3_3: { return "K3_D3"; }
#if ! PUBLIC_BUILD
            case DailyStochs: { return "D_K_D"; }
#endif
            case Williams_45: { return "%R45"; }
        }
        return "";
    }

private:
    Indicators _technicalindicator;
    std::string _name;
    Ticker *_ticker;

public:
    inline Indicators TechnicalIndicators () { return _technicalindicator; }
    inline std::string Name () { return _name; }

private:
    std::vector <double> _results;

private:
    bool _minvalueset;
    double _minvalue;
    bool _maxvalueset;
    double _maxvalue;

    std::string GetMinMaxValue (bool set, double value)
    {
        if (set)
        {
            CString str;
            str.Format ("%g", value);
            return (LPCTSTR) str;
        }
        return "?";
    }

public:
    std::string GetMinValue ()
    {
        return GetMinMaxValue (_minvalueset, _minvalue);
    }

    std::string GetMaxValue ()
    {
        return GetMinMaxValue (_maxvalueset, _maxvalue);
    }

protected:
    double GetSMA (std::vector <double> &data, int start, int numberofdays);
    int GetEMAListStart (double datacount, double emaperiod);
    double CalculateEMA (double close, double yesterdayclose, double k);
    void GetEMAList (std::vector <double> &data, double emaperiod, std::vector <double> &emas);
    double GetEMA (std::vector <double> &data, double emaperiod);

public:
    std::vector <double> &Results ()
    {
        return _results;
    }

    double LatestValue (bool &defaultvalue);
    
protected:
    void AddLatestValue (double value);
    void GetOffers (std::vector <double> &offers, int numberofprices);

public:    
    virtual bool DisplayIndicator ();
    virtual BuyIndicator BuyAction ();
    virtual void Calculate (TechnicalAnalysisSet *set);

    TechnicalIndicator (Indicators technicalindicator, std::string name, Ticker *ticker);
};


class RSI : public TechnicalIndicator
{
private:
    int _period;

public:
    BuyIndicator BuyAction ();

    void GetGainsLosses (std::vector <double> &data, int numberofdays, double &gains, double &losses);
    double GetRS (double gains, double losses);
    void GetRange (std::vector <double> &pairs, int index, int count, std::vector <double> &result);
    void GetRSs (std::vector <double> &pairs, int PERIOD, std::vector <double> &rsset);
    double CalculateRSIFromRS (double rs);

    void Calculate (TechnicalAnalysisSet *set);

    RSI (Indicators technicalindicator, Ticker *ticker, int period);
};


class StochRSI : public TechnicalIndicator
{
private:
    int _period;

public:
    BuyIndicator BuyAction ();

    void GetMinMax (std::vector <double> &data, int period, double &min, double &max);
    void Calculate (TechnicalAnalysisSet *set);

    StochRSI (Indicators technicalindicator, Ticker *ticker, int period);
};


class TrueSI : public TechnicalIndicator
{
public:
    BuyIndicator BuyAction ();

    double CalculateTSI (std::vector <double> &data, int start, double r, double s, bool &calculated);
    void Calculate (TechnicalAnalysisSet *set);

    TrueSI (Indicators technicalindicator, Ticker *ticker);
};


class Stochs : public TechnicalIndicator
{
private:
    int _first_param;
    int _second_param;
    int _x_smooth;

public:
    BuyIndicator BuyAction ();

    void SmoothOut (std::vector <double> &data, int smoothperiod, std::vector <double> &results);
    static bool Get_LowestLow_HighestHigh (std::vector <double> &data, int startindex, int numberofdays, double &lowestlow, double &highesthigh);
    void CalculateStochs (std::vector <double> &data, int startindex, int period, int smoothperiod, std::vector <double> &Ks, std::vector <double> &Ds);
    void Calculate (TechnicalAnalysisSet *set);

    Stochs (Indicators technicalindicator, Ticker *ticker, int first_param, int second_param, int x_smooth);
};


#if ! PUBLIC_BUILD
class DailyStochs : public TechnicalIndicator
{
private:
    std::string _filename;
    bool _fileread;
    bool _filereaderror;
    double _K;
    double _D;
    double _weekly_K;
    double _weekly_D;

public:
    inline bool FileReadError () { return _filereaderror; }
    inline double K () { return _K; }
    inline double D () { return _D; }
    inline double Weekly_K () { return _weekly_K; }
    inline double Weekly_D () { return _weekly_D; }

public:
    bool DisplayIndicator ();
    BuyIndicator BuyAction ();

    void Calculate (TechnicalAnalysisSet *set);

    double GetValue (CString &text);

    DailyStochs (Indicators technicalindicator, Ticker *ticker, std::string filename);
};
#endif


class Williams : public TechnicalIndicator
{
private:
    int _period;

public:
    BuyIndicator BuyAction ();
    void Calculate (TechnicalAnalysisSet *set);

    Williams (Indicators technicalindicator, Ticker *ticker, int period);
};


class TechnicalAnalysisSet
{
private:
    Ticker *_ticker;
    std::list <TechnicalIndicator *> _technicalindicators;

    typedef std::map <TechnicalIndicator::Indicators, TechnicalIndicator *>::iterator IndicatorsIter;
    std::map <TechnicalIndicator::Indicators, TechnicalIndicator *> _indicatorsbymap;

public:
    inline std::list <TechnicalIndicator *> &TechnicalIndicators () { return _technicalindicators; }

public:
    void AddIndicator (TechnicalIndicator *technicalindicator);
    TechnicalIndicator *GetIndicator (TechnicalIndicator::Indicators indicator);

    void Calculate ();

    TechnicalAnalysisSet (Ticker *ticker);
};

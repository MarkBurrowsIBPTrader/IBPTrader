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



std::string IB_PTraderDlg::AccountValue::GetHashKey (const IBString &key, const IBString &currency, const IBString &accountName)
{
    return (LPCSTR) (accountName + "_" + key + "_" + currency);
}


std::string IB_PTraderDlg::AccountValue::GetHashKey (AccountValue *accountvalue)
{
    return GetHashKey (accountvalue->_accountname.c_str (), accountvalue->_key.c_str (), accountvalue->_currency.c_str ());
}


double IB_PTraderDlg::AccountValue::GetAdjustedCashValue ()
{
    double currentvalue = ::atof (_value.c_str ());
    if (_currency == "GBP")
        currentvalue *= 100;
    return currentvalue;
}


void IB_PTraderDlg::AccountValue::Initialise (std::string accountname, std::string key, std::string value, std::string currency)
{
    _accountname = accountname; 
    _key = key; 
    _value = value;
    _currency = currency;
    _hashkey = GetHashKey (this);
    _numberhits++;
    _lasthittime = CTime::GetCurrentTime ();
}


IB_PTraderDlg::AccountValue::AccountValue () : _numberhits (0)
{
}


std::string IB_PTraderDlg::PortfolioPart::GetHashKey (Contract &contract)
{
    return (LPCSTR) (contract.symbol + "_" + contract.secType + "_" + contract.primaryExchange);
}


std::string IB_PTraderDlg::PortfolioPart::GetHashKey (Ticker *ticker)
{
    std::string exchange = ticker->_firstdbrow->_primaryexchange == "NASDAQ.NMS" ? "NASDAQ" : ticker->_firstdbrow->_primaryexchange;
    return ticker->_ticker + "_" + ticker->_firstdbrow->_sectype + "_" + exchange;
}


Contract CloneContract (Contract &contract)
{
    Contract newcontract;  
    newcontract.conId = contract.conId;
    newcontract.symbol = contract.symbol;
    newcontract.secType = contract.secType;
    newcontract.expiry = contract.expiry;
    newcontract.strike = contract.strike;
    newcontract.right = contract.right;
    newcontract.multiplier = contract.multiplier;
    newcontract.exchange = contract.exchange;
    newcontract.primaryExchange = contract.primaryExchange;
    newcontract.currency = contract.currency;
    newcontract.localSymbol = contract.localSymbol;
    newcontract.includeExpired = contract.includeExpired;
    newcontract.secIdType = contract.secIdType;
    newcontract.secId = contract.secId;
    newcontract.comboLegsDescrip = contract.comboLegsDescrip;
    return newcontract;
}


double IB_PTraderDlg::PortfolioPart::GetAdjustedMarketValue ()
{
    if (_contract.currency == "GBP")
        return _marketValue * 100;
    return _marketValue;
}


void IB_PTraderDlg::PortfolioPart::Initialise (Contract &contract, int position, double marketPrice, double marketValue, 
                                               double averageCost, double unrealizedPNL, double realizedPNL)
{
    _contract = CloneContract (contract);
    _position = position;
    _marketPrice = marketPrice;
    _marketValue = marketValue; 
    _averageCost = averageCost;
    _unrealizedPNL = unrealizedPNL;
    _realizedPNL = realizedPNL;
    _numberhits++;
    _lasthittime = CTime::GetCurrentTime ();
}


IB_PTraderDlg::PortfolioPart::PortfolioPart (Contract &contract, std::string accountname,
                                             int position, double marketPrice, double marketValue, 
                                             double averageCost, double unrealizedPNL, double realizedPNL) : _ticker (0),
                                                                                                             _contract (contract),
                                                                                                             _accountname (accountname),
                                                                                                             _numberhits (0),
                                                                                                             _initial_averageCost (averageCost)
{
    Initialise (contract, position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL);
}

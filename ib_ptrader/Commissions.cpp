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



#pragma region "Commissions"
void IB_PTraderDlg::InitialiseZeroCommissions ()
{
    // All these shares are Global X Funds ETFs

    // Commodity Funds
    _zero_usd_commissionshares ["COPX"] = true;
    _zero_usd_commissionshares ["GGGG"] = true;
    _zero_usd_commissionshares ["GLDX"] = true;
    _zero_usd_commissionshares ["LIT"] = true;
    _zero_usd_commissionshares ["SIL"] = true;
    _zero_usd_commissionshares ["JUNR"] = true;
    _zero_usd_commissionshares ["URA"] = true;
    _zero_usd_commissionshares ["SOIL"] = true;

    // Asia Funds
    _zero_usd_commissionshares ["ASEA"] = true;
    _zero_usd_commissionshares ["QQQC"] = true;
    _zero_usd_commissionshares ["CHIE"] = true;
    _zero_usd_commissionshares ["CHII"] = true;
    _zero_usd_commissionshares ["CHIM"] = true;
    _zero_usd_commissionshares ["CHIQ"] = true;
    _zero_usd_commissionshares ["CHIX"] = true;

    // Frontier
    _zero_usd_commissionshares ["NGE"] = true;
    _zero_usd_commissionshares ["EMFM"] = true;
    _zero_usd_commissionshares ["AZIA"] = true;

    // Latin American Funds
    _zero_usd_commissionshares ["AND"] = true;
    _zero_usd_commissionshares ["ARGT"] = true;
    _zero_usd_commissionshares ["BRAF"] = true;
    _zero_usd_commissionshares ["BRAQ"] = true;
    _zero_usd_commissionshares ["BRAZ"] = true;
    _zero_usd_commissionshares ["GXG"] = true;

    // Europe
    _zero_usd_commissionshares ["GREK"] = true;
    _zero_usd_commissionshares ["GXF"] = true;
    _zero_usd_commissionshares ["NORW"] = true;

    // Industry
    _zero_usd_commissionshares ["SOCL"] = true;

    // Income
    _zero_usd_commissionshares ["CNPF"] = true;
    _zero_usd_commissionshares ["SDIV"] = true;
    _zero_usd_commissionshares ["DIV"] = true;
    _zero_usd_commissionshares ["SPFF"] = true;
    _zero_usd_commissionshares ["MLPA"] = true;
    _zero_usd_commissionshares ["MLPJ"] = true;
    _zero_usd_commissionshares ["MLPX"] = true;

    // Asset Allocation
    _zero_usd_commissionshares ["PERM"] = true;

    // Alternatives
    _zero_usd_commissionshares ["GURU"] = true;
}


bool IB_PTraderDlg::IsZeroCommission (std::string ticker, stdext::hash_map <std::string, bool> &zerocommissions)
{
    ZeroCommsIter iter = zerocommissions.find (ticker);
    if (iter != zerocommissions.end ())
        return iter->second;
    return false;
}


double IB_PTraderDlg::GetPercentage (double value, double percentage)
{
    return value * (percentage / 100);
}


#pragma region "USD Commissions"
double IB_PTraderDlg::USD_ExchangeFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue)
{
    double result;
    if (shareprice >= 1)
    {
        result = ordersize * 0.003;
    }
    else
    {
        result = tradevalue * 0.003;
    }
    return result;
}


double IB_PTraderDlg::USD_TransactionFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue)
{
    return ordertype == OutStandingOrder::SELL ? tradevalue * 0.0000221 : 0;
}
#pragma endregion


#pragma region "CAD Commissions"
double IB_PTraderDlg::CAD_ExchangeFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue)
{
    double result;
    if (ordertype == OutStandingOrder::BUY)
    {
        // Remove Liquidity
        if (shareprice < 0.1)
        {
            result = ordersize * 0.000025;
        }
        else if (shareprice < 1)
        {
            result = ordersize * 0.000075;
        }
        else
        {
            result = ordersize * 0.0035;
        }
    }
    else
    {
        // Add Liquidity
        if (shareprice < 0.1)
        {
            result = -(ordersize * 0.000025);
        }
        else if (shareprice < 1)
        {
            result = -(ordersize * 0.000075);
        }
        else
        {
            result = -(ordersize * 0.0031);
        }
    }
    return result;
}


double IB_PTraderDlg::CAD_ClearingFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue)
{
    double result;
    if (contract.primaryExchange == "VENTURE")
    {
        result = ordersize * 0.0001;
        if (result > 2)
            result = 2;
    }
    else
    {
        result = ordersize * 0.00017;
        if (result > 2)
            result = 2;
    }
    return result;
}


double IB_PTraderDlg::CAD_TransactionFees (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double tradevalue)
{
    double result = ordersize * 0.00011;
    if (result > 3.3)
        result = 3.3;
    return result;
}
#pragma endregion


void IB_PTraderDlg::EnsureMinMaxComs (double minallowed, double maxallowed, double &value, bool &roundedup)
{
    if (value < minallowed)
    {
        value = minallowed;
        roundedup = true;
    }
    else if (value > maxallowed)
    {
        value = maxallowed;
    }
}


void IB_PTraderDlg::EnsureMaxCom (double maxallowed, double &value)
{
    if (value > maxallowed)
    {
        value = maxallowed;
    }
}


bool IB_PTraderDlg::CalculateCommission (OutStandingOrder::OrderType ordertype, Contract &contract, double shareprice, long ordersize, double &min, double &max, bool &roundedup, bool &stampdutyadded)
{
    roundedup = false;
    stampdutyadded = false;
    if (contract.secType == "STK")
    {
        double tradevalue = shareprice * ordersize;

        if (contract.currency == "USD")
        {
            #pragma region "USD"    
            if (IsZeroCommission ((LPCTSTR) contract.symbol, _zero_usd_commissionshares))
            {
                min = 0;
                max = 0;
                return true;
            }
      
            double com, mincomperorder, maxcomperorder;
            if (contract.exchange == "SMART")
            {
                com = ordersize * 0.005;
                mincomperorder = 1;
                maxcomperorder = GetPercentage (tradevalue, 0.5);
            }
            else
            {
                if (ordersize <= 500)
                    com = ordersize * 0.013;
                else
                    com = ordersize * 0.008;
                mincomperorder = 1.3;
                maxcomperorder = GetPercentage (tradevalue, 0.5);
            }

            double exchfee = USD_ExchangeFees (ordertype, contract, shareprice, ordersize, tradevalue);
            double transactionfees = USD_TransactionFees (ordertype, contract, shareprice, ordersize, tradevalue);
            double fullcom = com + transactionfees;
            min = fullcom;
            max = fullcom + exchfee;

            EnsureMinMaxComs (mincomperorder, maxcomperorder, min, roundedup);
            EnsureMinMaxComs (mincomperorder, maxcomperorder, max, roundedup);

            return true;
            #pragma endregion
        }
        else if (contract.currency == "CAD")
        {
            #pragma region "CAD"
            double com = ordersize * 0.01;
            double maxcomperorder = GetPercentage (tradevalue, 0.5);

            double exchfees = CAD_ExchangeFees (ordertype, contract, shareprice, ordersize, tradevalue);
            double clearingfees = CAD_ClearingFees (ordertype, contract, shareprice, ordersize, tradevalue);
            double transactionfees = CAD_TransactionFees (ordertype, contract, shareprice, ordersize, tradevalue);
            min = com + clearingfees + transactionfees;
            max = com + exchfees + clearingfees + transactionfees;

            EnsureMinMaxComs (1, maxcomperorder, min, roundedup);
            EnsureMinMaxComs (1, maxcomperorder, max, roundedup);

            return true;
            #pragma endregion
        }
        else if (contract.currency == "GBP")
        {
            #pragma region "GBP"
            double adjustment;
            double tax = GetPercentage (tradevalue, 0.5);
            if (tradevalue > (50000 * 100))
            {
                adjustment = GetPercentage (tradevalue - (50000 * 100), 0.05);
            }
            else
            {
                adjustment = 0;
            }
            min = 600 + adjustment;
            EnsureMaxCom (2900, min);
            min += tax;
            max = min;
            stampdutyadded = true;
            roundedup = true;
            return true;
            #pragma endregion
        }
        else
        {
            _stats.IncrementStat ("CalculateCommission", "CurrencyNotSupported", (LPCTSTR) contract.symbol, (LPCTSTR) contract.currency);
        }
    }
    return false;
}
#pragma endregion

/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "IB_DataDumpDlg.h"


// IB_DataDumpDlg dialog

IMPLEMENT_DYNAMIC(IB_DataDumpDlg, CDialog)

IB_DataDumpDlg::IB_DataDumpDlg(CWnd* pParent /*=NULL*/)
    : CDialog(IB_DataDumpDlg::IDD, pParent), _parent (0)
{
}


IB_DataDumpDlg::~IB_DataDumpDlg()
{
}


void IB_DataDumpDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DATALIST, _datalistbox);
}


BOOL IB_DataDumpDlg::OnInitDialog ()
{
    CDialog::OnInitDialog ();

    if (_parent)
    {
        bool added = false;
        if (_parent->_accountvalues)
        {
            CSingleLock crit (&_parent->_account_critsection);
            crit.Lock ();
            try
            {
                CString str;
                for (IB_PTraderDlg::AccountValuesIter iter = _parent->_accountvalues->begin ();  iter != _parent->_accountvalues->end ();  ++iter)
                {
                    IB_PTraderDlg::AccountValue *accountvalue = iter->second;
                    std::string line = accountvalue->_key + "=" + accountvalue->GetValue () + " (" + (accountvalue->_currency == "" ? "-" : accountvalue->_currency) + ") - ";
                    str.Format ("%ld", accountvalue->_numberhits);
                    line += (LPCSTR) str;
                    line += " hits";
                    if (accountvalue->_numberhits > 0)
                    {
                        str.Format (" (%0.2d:%0.2d:%0.2d)", accountvalue->_lasthittime.GetHour (), accountvalue->_lasthittime.GetMinute (), accountvalue->_lasthittime.GetSecond ());
                        line += (LPCSTR) str;
                    }
                    this->_datalistbox.AddString (line.c_str ());
                    added = true;
                }
            }
            catch (std::exception &e) 
            {
                _parent->_stats.IncrementStat ("IB_DataDumpDlg::OnInitDialog", e.what (), "", "");
            }
            catch (...)
            {
                _parent->_stats.IncrementStat ("IB_DataDumpDlg::OnInitDialog", "exception ...", "", "");
            }
            crit.Unlock ();
        }
        if (_parent->_portfolio)
        {
            CSingleLock crit (&_parent->_portfolio_critsection);
            crit.Lock ();
            try
            {
                if (added)
                    this->_datalistbox.AddString ("-----------------------------------------------");
                for (IB_PTraderDlg::AccountPortfolioPartsIter outeriter = _parent->_portfolio->begin ();  outeriter != _parent->_portfolio->end ();  ++outeriter)
                {
                    std::string accountname = outeriter->first;
                    this->_datalistbox.AddString ((accountname + ":-").c_str ());
                    for (IB_PTraderDlg::PortfolioPartsIter inneriter = outeriter->second->begin ();
                        inneriter != outeriter->second->end ();  ++inneriter)
                    {
                        IB_PTraderDlg::PortfolioPart *part = inneriter->second;
                        if (part->_ticker == 0)
                        {
                            std::string hashkey = IB_PTraderDlg::PortfolioPart::GetHashKey (part->_contract);
                            part->_ticker = _parent->_tickers ? _parent->_tickers->GetTickerByContract (hashkey) : 0;
                        }
                        CString message = part->_contract.symbol + " (" + part->_contract.secType + ") (e=" + part->_contract.exchange + ", pe=" +
                                        part->_contract.primaryExchange + ")";
                        CString str;
                        str.Format (", pos=%d, price=%g, value=%g, avg=%g, uPNL=%g, rPNL=%g", 
                                    part->_position, part->_marketPrice, part->GetMarketValue (), part->_averageCost, 
                                    part->_unrealizedPNL, part->_realizedPNL);
                        message += str;
                        if (part->_ticker)
                        {
                            message += ", Vol=";
                            TickerListBox::GetFormattedNumberString (part->_ticker->Volume (), true, str);
                            message += str;
                            message += ", AvgVol=";
                            TickerListBox::GetFormattedNumberString (part->_ticker->AvgVolume (), true, str);
                            message += str;
                        }
                        str.Format (" - %ld hits", part->_numberhits);
                        message += str;
                        if (part->_numberhits > 0)
                        {
                            str.Format (" (%0.2d:%0.2d:%0.2d)", part->_lasthittime.GetHour (), part->_lasthittime.GetMinute (), part->_lasthittime.GetSecond ());
                            message += str;
                        }
                        this->_datalistbox.AddString ((LPCTSTR) message);                    
                    }
                }
            }
            catch (std::exception &e) 
            {
                _parent->_stats.IncrementStat ("IB_DataDumpDlg::OnInitDialog", e.what (), "", "");
            }
            catch (...)
            {
                _parent->_stats.IncrementStat ("IB_DataDumpDlg::OnInitDialog", "exception ...", "", "");
            }
            crit.Unlock ();
        }
    }

    return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(IB_DataDumpDlg, CDialog)
    ON_BN_CLICKED(IDOK, &IB_DataDumpDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &IB_DataDumpDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// IB_DataDumpDlg message handlers

void IB_DataDumpDlg::OnBnClickedOk ()
{
    OnOK();
}


void IB_DataDumpDlg::DoFileDump (CString filename)
{
    CFile file (filename, CFile::modeCreate | CFile::modeReadWrite);
    int numberitems = this->_datalistbox.GetCount ();
    for (int i = 0;  i < numberitems;  i++)
    {
        CString str;
        this->_datalistbox.GetText (i, str);
        LPCTSTR buf = (LPCTSTR) str;
        UINT len = lstrlen (buf);
        file.Write (buf, len);
        file.Write ("\r\n", 2);
    }
    file.Close ();
}


void IB_DataDumpDlg::OnBnClickedCancel ()
{
    CString str = _parent->GetFileName ("dump");
    DoFileDump (str);
    _parent->OnBnClickedScrollmessages ();
}

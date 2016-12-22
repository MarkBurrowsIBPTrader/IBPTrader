/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "AppSettings.h"
#include "ib_ptraderdlg.h"
#include "ib_datadumpdlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif



IB_PTraderDlg::IB_PTraderDlg (CWnd* pParent)
    : CDialog(IB_PTraderDlg::IDD, pParent), _CLIENTID (-1), _PORTID (0),
                                            _accountvalues (0), _portfolio (0), _contractREQID (0), _shares (0), 
                                            _have_shown_LB_ERR (false), _have_shown_LB_ERRSPACE (false), _tickers (0),
                                            _dbcon (0), _dbschema ("ibrokers"),
#pragma warning (push)
#pragma warning (disable : 4355)
                                            _ordermaps (this), _accounttype (NONE), _appsettings (0), _allowedorders (Stopped), 
                                            _locked (false), _stats (this),
#pragma warning (pop)
#if ! PUBLIC_BUILD
                                            _numberofsellstarts_fails (0), _max_number_of_sellstarts_fails (0),
#endif
                                            _minaccountvaluebalance (DBL_MAX), _maxaccountvaluebalance (DBL_MIN),
                                            _minPnL (DBL_MAX), _maxPnL (DBL_MIN),
                                            _titletextsetfirsttime (true), _accountselection (""), _orignalwindowtitletext (""),
                                            _extrawindowtitletext (""), _tickers_to_record (0)
#if ! PUBLIC_BUILD
                                            , _displayquotesdiagnostics (false)
#endif
{
    _mainlistbox.SetParent (this);
    _pclient = new EClientSocket (this);
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void IB_PTraderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_QUOTESLIST, _mainlistbox);
    DDX_Control(pDX, IDC_MESSAGES, _statusmessageslistbox);
}

BEGIN_MESSAGE_MAP(IB_PTraderDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDOK, &IB_PTraderDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &IB_PTraderDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_DB_START, &IB_PTraderDlg::OnBnClickedDbStart)
    ON_BN_CLICKED(IDC_XML_START, &IB_PTraderDlg::OnBnClickedXmlStart)
    ON_BN_CLICKED(IDC_DATADUMP, &IB_PTraderDlg::OnBnClickedDatadump)
    ON_BN_CLICKED(IDC_LISTDUMP, &IB_PTraderDlg::OnBnClickedListdump)
    ON_BN_CLICKED(IDC_INTERNALDATA, &IB_PTraderDlg::OnBnClickedInternaldata)
    ON_WM_TIMER()
    ON_LBN_DBLCLK(IDC_QUOTESLIST, &IB_PTraderDlg::OnLbnDblclkQuoteslist)
    ON_BN_CLICKED(IDC_KILLBUYORDER, &IB_PTraderDlg::OnBnClickedKillbuyorder)
    ON_BN_CLICKED(IDC_KILLSELLORDER, &IB_PTraderDlg::OnBnClickedKillsellorder)
    ON_BN_CLICKED(IDC_STARTSTOP, &IB_PTraderDlg::OnBnClickedStartstop)
    ON_LBN_SELCHANGE(IDC_MESSAGES, &IB_PTraderDlg::OnLbnSelchangeMessages)
    ON_LBN_DBLCLK(IDC_MESSAGES, &IB_PTraderDlg::OnLbnDblclkMessages)
    ON_BN_CLICKED(IDC_STATSSCREEN, &IB_PTraderDlg::OnBnClickedStatsscreen)
    ON_BN_CLICKED(IDC_STATSFILE, &IB_PTraderDlg::OnBnClickedStatsfile)
    ON_BN_CLICKED(IDC_SELLSTARTS, &IB_PTraderDlg::OnBnClickedSellstarts)
    ON_BN_CLICKED(IDC_SELLSTARTSFILE, &IB_PTraderDlg::OnBnClickedSellstartsfile)
    ON_BN_CLICKED(IDC_TOGGLESELLSPEED, &IB_PTraderDlg::OnBnClickedTogglesellspeed)
    ON_BN_CLICKED(IDC_KILLALLORDERS, &IB_PTraderDlg::OnBnClickedKillallorders)
    ON_BN_CLICKED(IDC_SHOWALLORDERS, &IB_PTraderDlg::OnBnClickedShowallorders)
    ON_BN_CLICKED(IDC_TICKERACTIVE, &IB_PTraderDlg::OnBnClickedTickeractive)
    ON_BN_CLICKED(IDC_SCROLLMESSAGES, &IB_PTraderDlg::OnBnClickedScrollmessages)
    ON_BN_CLICKED(IDC_CLEARMESSAGES, &IB_PTraderDlg::OnBnClickedClearmessages)
    ON_LBN_SELCANCEL(IDC_QUOTESLIST, &IB_PTraderDlg::OnLbnSelcancelQuoteslist)
    ON_LBN_SELCHANGE(IDC_QUOTESLIST, &IB_PTraderDlg::OnLbnSelchangeQuoteslist)
    ON_BN_CLICKED(IDC_GOTO_ACTIVE, &IB_PTraderDlg::OnBnClickedGotoActive)
    ON_BN_CLICKED(IDC_GOTO_INACTIVE, &IB_PTraderDlg::OnBnClickedGotoInactive)
    ON_BN_CLICKED(IDC_GOTO_BUY, &IB_PTraderDlg::OnBnClickedGotoBuy)
    ON_BN_CLICKED(IDC_GOTO_SELL, &IB_PTraderDlg::OnBnClickedGotoSell)
    ON_BN_CLICKED(IDC_DUMPQUOTES, &IB_PTraderDlg::OnBnClickedDumpquotes)
    ON_BN_CLICKED(IDC_UNSELECT_QUOTES, &IB_PTraderDlg::OnBnClickedUnselectQuotes)
    ON_BN_CLICKED(IDC_UNSELECT_MESSAGES, &IB_PTraderDlg::OnBnClickedUnselectMessages)
    ON_BN_CLICKED(IDC_AVG_ACTION, &IB_PTraderDlg::OnBnClickedAvgAction)
    ON_BN_CLICKED(IDC_BATCHBUY, &IB_PTraderDlg::BatchBuyClick)
    ON_BN_CLICKED(IDC_BATCHBUYCHECK, &IB_PTraderDlg::OnBnClickedBatchbuycheck)
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_LIQUIDATE, &IB_PTraderDlg::OnBnClickedLiquidate)
    ON_BN_CLICKED(IDC_ALLLIQUIDATE, &IB_PTraderDlg::OnBnClickedAllliquidate)
END_MESSAGE_MAP()


bool IB_PTraderDlg::Parse_TWS_CmdLine (const CString &str)
{
    int index = 0;
    CString token;
    while (AfxExtractSubString (token, str, index, ','))
    {  
        if (token == "")
            break;
        switch (index)
        {
            case 0:
                _username = token;
                break;
            case 1:
                _accountnumber = token;
                break;
            case 2:
                if (token.CompareNoCase ("live") == 0)
                {
                    _accounttype = Live;
                    _accountselection = "Live";
                }
                else if (token.CompareNoCase ("paper") == 0)
                {
                    _accounttype = Paper;
                    _accountselection = "Paper";
                }
                else
                {
                    return false;
                }                
                break;
            case 3:
                _CLIENTID = ::atoi ((LPCTSTR) token);
                break;
            case 4:
                _PORTID = ::atoi ((LPCTSTR) token);
                break;
            default:
                return false;
        }
        index++;
    }
    return index == 5;
}


bool IB_PTraderDlg::Parse_DB_CmdLine (const CString &str)
{
    int index = 0;
    CString token;
    while (AfxExtractSubString (token, str, index, ','))
    {  
        if (token == "")
            break;
        switch (index)
        {
            case 0:
                _dbschema = token;
                break;
            case 1:
                _dbusername = token;
                break;
            case 2:
                _dbpwd = token;
                break;
            case 3:
                _dbhostname = token;
                break;
            default:
                return false;
        }
        index++;
    }
    return index == 4;
}


BOOL IB_PTraderDlg::OnInitDialog ()
{
    CDialog::OnInitDialog ();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    bool cmdlinecorrectlyformed = false;
    CWinApp *app = AfxGetApp ();
    CString cmdline;
    if (app->m_lpCmdLine)
    {
        bool found_tws = false, found_db = false;
        CString token;
        int cmdlinecount = 0;
        cmdline = app->m_lpCmdLine;
        while (AfxExtractSubString (token, cmdline, cmdlinecount++, ' '))
        {  
            if (token == "")
                break;
            if (token.Left (4).CompareNoCase ("tws=") == 0)
            {
                found_tws = Parse_TWS_CmdLine (token.Right (token.GetLength () - 4));
            }
            else if (token.Left (3).CompareNoCase ("db=") == 0)
            {
                found_db = Parse_DB_CmdLine (token.Right (token.GetLength () - 3));
            }
            else
            {
                break;
            }
        }
        cmdlinecorrectlyformed = found_tws && found_db;
    }

    if (! cmdlinecorrectlyformed)
    {
        Log ("Command Line parameters incorrect, format should be:-");
        Log ("ib_ptrader TWS=twsusername,twsaccountnumber,Live|Paper,clientid,portnumber DB=schemaname,dbusername,dbpwd,dbhostname");
        _locked = true;
        GetDlgItem (IDOK)->EnableWindow (false);
        GetDlgItem (IDC_DB_START)->EnableWindow (false);
        GetDlgItem (IDC_XML_START)->EnableWindow (false);
        GetDlgItem (IDC_STARTSTOP)->EnableWindow (false);
    }

    switch (_accounttype)
    {
        case Paper:
            _appsettings = new PaperSettings (this);
            break;
        case Live:
            _appsettings = new LiveSettings (this);
            break;
    }

    int accountsel = _accounttype == Live ? 3 : 0;
    CComboBox *paccounttype = (CComboBox *) GetDlgItem (IDC_ACCOUNTTYPE);
    paccounttype->SetCurSel (accountsel);
#if PUBLIC_BUILD
    GetDlgItem (IDC_ACCOUNTTYPE)->EnableWindow (false);
    GetDlgItem (IDC_TOGGLESELLSPEED)->EnableWindow (false);
    GetDlgItem (IDC_TOGGLESELLSPEED)->ShowWindow (SW_HIDE);
    GetDlgItem (IDC_SELLSTARTS)->EnableWindow (false);
    GetDlgItem (IDC_SELLSTARTS)->ShowWindow (SW_HIDE);
    GetDlgItem (IDC_SELLSTARTSFILE)->EnableWindow (false);
    GetDlgItem (IDC_SELLSTARTSFILE)->ShowWindow (SW_HIDE);
#endif

    SetStopGoButtonText ();
    SetTickerButtonText (0);
#if ! PUBLIC_BUILD
    SetSellStartsButtonText ();
#endif
    SetBuyAverageButtonText (0);

    TCHAR szDirectory [MAX_PATH + 1];
    ::GetCurrentDirectory (sizeof (szDirectory) - 1, szDirectory);
    _currentworkingdir = szDirectory;

    return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void IB_PTraderDlg::OnPaint ()
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


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR IB_PTraderDlg::OnQueryDragIcon ()
{
    return static_cast <HCURSOR> (m_hIcon);
}


void IB_PTraderDlg::OnBnClickedOk ()
{
    StartUp (Both);
}


void IB_PTraderDlg::OnBnClickedDbStart ()
{
    StartUp (DB);
}


void IB_PTraderDlg::OnBnClickedXmlStart ()
{   
    if (AfxMessageBox ("Read from XML only (delete DB)?", MB_YESNO) == IDYES)
    {
        StartUp (PGen);
    }
}


void IB_PTraderDlg::OnBnClickedCancel ()
{
    int numberofbuys, numberofsells;
    _ordermaps.GetBuySellCounts (numberofbuys, numberofsells);
    if (numberofbuys > 0 || numberofsells > 0)
    {
        CString format;
        format.Format ("%d sell orders, %d, buys orders active - Continue Close?", numberofsells, numberofbuys);
        if (AfxMessageBox ((LPCTSTR) format, MB_YESNO) == IDNO)
        {
            return;
        }
    }
    Log ("Disconnecting from TWS...");
    _pclient->eDisconnect ();
    DisconnectFromDatabase ();
    OnCancel ();
}


void IB_PTraderDlg::OnBnClickedDatadump ()
{
    IB_DataDumpDlg dlg;
    dlg._parent = this;
    dlg.DoModal ();
}


long IB_PTraderDlg::GetEditFieldAsNumber (int editctrlid, bool &valid)
{
    CString text;
    GetDlgItem (editctrlid)->GetWindowText (text);
    text.Trim ();
    int len = text.GetLength ();
    if (len == 0)
    {
        valid = false;
        return 0;
    }
    for (int i = 0;  i < len;  i++)
    {
        char ch = text [i];
        if (! ::isdigit (ch))
        {
            valid = false;
            return 0;
        }
    }
    long value = ::atol ((LPCSTR) text);
    valid = true;
    return value;
}


void IB_PTraderDlg::KillOrder (OutStandingOrder::OrderType expectedordertype)
{
    bool valid;
    OrderId orderid = GetEditFieldAsNumber (IDC_ORDERID, valid);
    if (! valid)
    {
        AfxMessageBox ("Field Should all be digits");
        return;
    }
    OutStandingOrder *outstandingorder = _ordermaps.GetOutStandingOrder (orderid);
    if (! outstandingorder)
    {
        AfxMessageBox ("Order id not found");
        return;
    }
    if (outstandingorder->GetOrderType () != expectedordertype)
    {
        AfxMessageBox ("Different Order Type found");
        return;
    }
    std::string ordertypeasstr = OutStandingOrder::ToString (outstandingorder->GetOrderType ());
    CString str;
    str.Format ("Do you want to delete %s order %lu?", ordertypeasstr.c_str (), orderid);
    if (AfxMessageBox ((LPCSTR) str, MB_YESNO) == IDYES)
    {
        CancelOrder (outstandingorder, orderid);
    }
}


void IB_PTraderDlg::OnBnClickedKillbuyorder ()
{
    KillOrder (OutStandingOrder::BUY);
}


void IB_PTraderDlg::OnBnClickedKillsellorder ()
{
    KillOrder (OutStandingOrder::SELL);
}


void IB_PTraderDlg::OnBnClickedStartstop ()
{
    if (_allowedorders == Allow)
    {
        _allowedorders = Stopped;
        Log ("Stopping");
    }
    else
    {
        _allowedorders = Allow;
        Log ("Go!");
    }
    SetStopGoButtonText ();
    OnBnClickedScrollmessages ();
    OnBnClickedUnselectQuotes ();
    if (_allowedorders == Allow)
    {
        Set_DoBuys_DoSells_OnAllsTickersStateChange ();
    }
}


void IB_PTraderDlg::OnLbnDblclkMessages ()
{
    int cursel = _statusmessageslistbox.GetCurSel ();
    if (cursel != LB_ERR)
    {
        CString text;
        _statusmessageslistbox.GetText (cursel, text);
        AfxMessageBox ((LPCTSTR) text);
    }
    OnBnClickedUnselectMessages ();
}


void IB_PTraderDlg::OnBnClickedStatsscreen ()
{
    _stats.Dump (0);
}


void IB_PTraderDlg::OnBnClickedStatsfile ()
{
    CString filename = GetFileName ("stats");
    CFile file (filename, CFile::modeCreate | CFile::modeReadWrite);
    _stats.Dump (&file);
    file.Close ();
}


#if ! PUBLIC_BUILD
int IB_PTraderDlg::DumpSellStarts (CFile *file, bool wantlogging)
{
    int dumpcount = 0;
    if (_tickers == 0)
        return dumpcount;
    CString format;
    for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
    {
        Ticker *ticker = iter->second;
        if (ticker->_sellsstartpoint)
        {
            BuyPricePair *lowestmatch = 0;
            double lowestmatchprice = DBL_MAX;
            int size = ticker->_buys.GetDataCount ();
            for (int i = 0;  i < size;  i++)
            {
                BuyPricePair *buyprice = ticker->_buys.GetData (i);
                if (buyprice->GetStatus () == PricePair::BOUGHT && buyprice->_sellpricepair->GetStatus () == PricePair::ACTIVE)
                {
                    if (buyprice->_sellpricepair->_price < lowestmatchprice)
                    {
                        lowestmatch = buyprice;
                        lowestmatchprice = buyprice->_sellpricepair->_price;
                    }
                }
            }
            if (lowestmatch)
            {
                if (ticker->_sellsstartpoint != lowestmatch)
                {
                    ticker->_sellsearchspeed = Ticker::Slow;
                    FailedSellStartsIter failedstartsiter = find (_failedsellstarts.begin (), _failedsellstarts.end (), ticker);
                    if (failedstartsiter == _failedsellstarts.end ())
                    {
                        _failedsellstarts.push_back (ticker);
                    }
                    if (wantlogging)
                    {
                        if (dumpcount == 0)
                        {
                            DumpLineToFile (IB_PTraderDlg::None, file, "SellStarts:-");
                            DumpNewLineToFile (IB_PTraderDlg::None, file);
                        }
                        format.Format ("%s is %lu (%g %s -> %g %s, PI=%d,%d) - sell start point is %lu (%g %s, PI=%d,%d)", 
                                       ticker->_ticker.c_str (),
                                       lowestmatch->_dbrow->_iddeals, 
                                       lowestmatch->_price,
                                       PricePair::ToShortString (lowestmatch->GetStatus ()).c_str (),

                                       lowestmatch->_sellpricepair->_price,
                                       PricePair::ToShortString (lowestmatch->_sellpricepair->GetStatus ()).c_str (),

                                       lowestmatch->_pairindex,
                                       lowestmatch->_sellpricepair->_pairindex,

                                       ticker->_sellsstartpoint->_dbrow->_iddeals,
                                       ticker->_sellsstartpoint->_sellpricepair->_price,
                                       PricePair::ToShortString (ticker->_sellsstartpoint->_sellpricepair->GetStatus ()).c_str (),

                                       ticker->_sellsstartpoint->_pairindex,
                                       ticker->_sellsstartpoint->_sellpricepair->_pairindex
                                      );
                        DumpLineToFile (IB_PTraderDlg::None, file, (LPCTSTR) format);
                        DumpNewLineToFile (IB_PTraderDlg::None, file);
                    }
                    dumpcount++;
                }
                else
                {
                    ticker->_sellsearchspeed = Ticker::Fast;
                }
            }
        }
    }
    if (wantlogging)
    {
        if (dumpcount == 0)
        {
            DumpLineToFile (IB_PTraderDlg::None, file, "SellStarts:- Zero Found");
            DumpNewLineToFile (IB_PTraderDlg::None, file);
        }
        else
        {
            format.Format ("%i items", dumpcount);
            DumpLineToFile (IB_PTraderDlg::None, file, (LPCTSTR) format);
            DumpNewLineToFile (IB_PTraderDlg::None, file);
        }
        bool fp = true;
        for each (Ticker *ticker in _failedsellstarts)
        {
            if (fp)
            {
                format = "All FailedSellStarts:- ";
                fp = false;
            }
            else
            {
                format += ", ";
            }
            CString str;
            str.Format ("%s", ticker->_ticker.c_str ());
            format += str;
        }
        if (! fp)
        {
            DumpLineToFile (IB_PTraderDlg::None, file, (LPCTSTR) format);
            DumpNewLineToFile (IB_PTraderDlg::None, file);
        }
        OnBnClickedScrollmessages ();
    }
    return dumpcount;
}
#endif


void IB_PTraderDlg::OnBnClickedSellstarts ()
{
#if ! PUBLIC_BUILD
    DumpSellStarts (0, true);
#endif
}


void IB_PTraderDlg::OnBnClickedSellstartsfile ()
{
#if ! PUBLIC_BUILD
    CString filename = GetFileName ("sellstarts");
    CFile file (filename, CFile::modeCreate | CFile::modeReadWrite);
    DumpSellStarts (&file, true);
    file.Close ();
#endif
}


void IB_PTraderDlg::OnBnClickedTogglesellspeed ()
{
#if ! PUBLIC_BUILD
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        if (ticker->_sellsearchspeed == Ticker::Slow)
        {
            ticker->_sellsearchspeed = Ticker::Fast;
        }
        else
        {
            ticker->_sellsearchspeed = Ticker::Slow;
        }
        SetSellStartsButtonText ();
    }
#endif
}


void IB_PTraderDlg::OnBnClickedKillallorders ()
{
    CString text;
    GetDlgItem (IDC_ORDERID)->GetWindowText (text);
    if (text.GetLength () == 1 && text [0] == 'x')
    {
        if (AfxMessageBox ("KILL all orders?", MB_YESNO) == IDYES)
        {
            std::vector <OutStandingOrder *> orderslist;
            for (OrderMaps::OrderIdIter iter = _ordermaps._orderid_to_outstandingorder.begin ();  iter !=  _ordermaps._orderid_to_outstandingorder.end ();  ++iter)
            {
                orderslist.push_back (iter->second);
            }
            for (size_t i = 0;  i < orderslist.size ();  i++)
            {
                OutStandingOrder *outstandingorder = orderslist [i];
                OutStandingOrder::OrderType ordertype = outstandingorder->GetOrderType ();
                CString str;
                str.Format ("Kill OrderId %ld, %s?", 
                            outstandingorder->_orderid, 
                            OutStandingOrder::ToString (ordertype).c_str ());                
                if (AfxMessageBox ((LPCTSTR) str, MB_YESNO) == IDYES)
                {
                    CancelOrder (outstandingorder, outstandingorder->_orderid);
                }
            }
        }
    }
    else
    {
        AfxMessageBox ("x expected in kill orders field");
    }
    GetDlgItem (IDC_ORDERID)->SetWindowText ("");
}


void IB_PTraderDlg::OnBnClickedShowallorders ()
{
    Log ("----------------------------------------");
    bool shown = false;
    for (OrderMaps::OrderIdIter iter = _ordermaps._orderid_to_outstandingorder.begin ();  iter !=  _ordermaps._orderid_to_outstandingorder.end ();  ++iter)
    {
        OutStandingOrder *outstandingorder = iter->second;
        OutStandingOrder::OrderType ordertype = outstandingorder->GetOrderType ();
        CString str;
        str.Format ("OrderId %ld, %s %s %i:%i:%i", 
                    iter->first, 
                    outstandingorder->_ticker->_ticker.c_str (),
                    OutStandingOrder::ToString (ordertype).c_str (),
                    outstandingorder->_recordedtime.GetHour (),
                    outstandingorder->_recordedtime.GetMinute (),
                    outstandingorder->_recordedtime.GetSecond ());
        for each (BuyPricePair *buypricepair in outstandingorder->_buypricepairs)
        {
            CString format;
            format.Format (", %ld (%s", buypricepair->_oi, PricePair::ToShortString (buypricepair->GetStatus ()).c_str ());
            str += format;
            if (ordertype == OutStandingOrder::SELL)
            {
                format.Format (", %s", PricePair::ToShortString (buypricepair->_sellpricepair->GetStatus ()).c_str ());
                str += format;
            }
            str += ")";
        }
        Log (str);
        shown = true;
    }
    if (! shown)
        Log ("No orders currently active");
    OnBnClickedScrollmessages ();
}


void IB_PTraderDlg::OnLbnSelchangeMessages ()
{
}


void IB_PTraderDlg::OnLbnSelcancelQuoteslist ()
{
}


void IB_PTraderDlg::OnLbnSelchangeQuoteslist ()
{
    Ticker *ticker = GetSelectedTicker ();
    SetTickerButtonText (ticker);
#if ! PUBLIC_BUILD
    SetSellStartsButtonText ();
#endif
    SetBuyAverageButtonText (ticker);   
}


void IB_PTraderDlg::OnBnClickedScrollmessages ()
{
    int numberofitems = _statusmessageslistbox.GetCount ();
    if (numberofitems > 0)
    {
        _statusmessageslistbox.SetCurSel (numberofitems - 1);
        _statusmessageslistbox.SetCurSel (-1);
    }
}


void IB_PTraderDlg::OnBnClickedClearmessages()
{
    _statusmessageslistbox.ResetContent ();
}


void IB_PTraderDlg::Set_DoBuys_DoSells_OnTickerStateChange (Ticker *ticker)
{
    switch (ticker->_activestate)
    {
        case Ticker::Active:
            Sell (ticker);
            Buy (ticker);
            break;
        case Ticker::BuyOnly:
            Buy (ticker);
            break;
        case Ticker::SellOnly:
            Sell (ticker);
            break;
    }        
}


void IB_PTraderDlg::Set_DoBuys_DoSells_OnAllsTickersStateChange ()
{
    if (_tickers && _tickers->GetTickers ())
    {
        for (TickersIter iter = _tickers->GetTickers ()->begin ();  iter != _tickers->GetTickers ()->end ();  ++iter)
        {
            Set_DoBuys_DoSells_OnTickerStateChange (iter->second);
        }
    }
}


void IB_PTraderDlg::SetTickerButtonText (Ticker *ticker)
{
    CButton *batchbuycheck = (CButton *) GetDlgItem (IDC_BATCHBUY);
    CWnd *tickeractive_button = GetDlgItem (IDC_TICKERACTIVE);
    if (ticker == 0 || ticker->_activestate == Ticker::DISABLED || ticker->_tickerhasnoprices)
    {
        tickeractive_button->EnableWindow (false);
        tickeractive_button->SetWindowText ("?");
        GetDlgItem (IDC_GOTO_ACTIVE)->EnableWindow (false);
        GetDlgItem (IDC_GOTO_INACTIVE)->EnableWindow (false);
        GetDlgItem (IDC_GOTO_BUY)->EnableWindow (false);
        GetDlgItem (IDC_GOTO_SELL)->EnableWindow (false);
        GetDlgItem (IDC_BATCHBUYCHECK)->EnableWindow (false);
        GetDlgItem (IDC_LIQUIDATE)->EnableWindow (false);
        GetDlgItem (IDC_ALLLIQUIDATE)->EnableWindow (false);
        batchbuycheck->EnableWindow (false);
        batchbuycheck->SetCheck (false);
        return;
    }
    bool defaultenable = ! _locked;
    tickeractive_button->EnableWindow (defaultenable);
    Ticker::ActiveState nextstate = Ticker::GetNextActiveState (ticker->_activestate);
    CString text = Ticker::ToString (nextstate);
    tickeractive_button->SetWindowText ("Set " + text);
    GetDlgItem (IDC_GOTO_ACTIVE)->EnableWindow (defaultenable);
    GetDlgItem (IDC_GOTO_INACTIVE)->EnableWindow (defaultenable);
    GetDlgItem (IDC_GOTO_BUY)->EnableWindow (defaultenable);
    GetDlgItem (IDC_GOTO_SELL)->EnableWindow (defaultenable);
    GetDlgItem (IDC_LIQUIDATE)->EnableWindow (defaultenable);
    GetDlgItem (IDC_ALLLIQUIDATE)->EnableWindow (defaultenable);

    // Logic for batch buy
    batchbuycheck->SetCheck (ticker->_makebatchbuy);
    bool batchbuyenable = false, batchsellenable = false;
    if (ticker->_share)
    {
        if (ticker->_share->_share_has_buy_daylimit)
        {
            batchbuyenable = true;
        }
        if (ticker->_share->_share_has_sell_daylimit)
        {
            batchsellenable = true;
        }
    }
    GetDlgItem (IDC_BATCHBUYCHECK)->EnableWindow (batchbuyenable || batchsellenable);
    batchbuycheck->EnableWindow (batchbuyenable && defaultenable); 
}


void IB_PTraderDlg::OnBnClickedTickeractive ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        ticker->_activestate = Ticker::GetNextActiveState (ticker->_activestate);
        Set_DoBuys_DoSells_OnTickerStateChange (ticker);
    }
    OnBnClickedUnselectQuotes ();
}


void IB_PTraderDlg::OnBnClickedGotoActive ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        ticker->_activestate = Ticker::Active;
        Set_DoBuys_DoSells_OnTickerStateChange (ticker);
    }
    OnBnClickedUnselectQuotes ();
}


void IB_PTraderDlg::OnBnClickedGotoInactive ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        ticker->_activestate = Ticker::InActive;
        Set_DoBuys_DoSells_OnTickerStateChange (ticker);
    }
    OnBnClickedUnselectQuotes ();
}


void IB_PTraderDlg::OnBnClickedGotoBuy ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        ticker->_activestate = Ticker::BuyOnly;
        Set_DoBuys_DoSells_OnTickerStateChange (ticker);
    }
    OnBnClickedUnselectQuotes ();
}


void IB_PTraderDlg::OnBnClickedGotoSell ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        ticker->_activestate = Ticker::SellOnly;
        Set_DoBuys_DoSells_OnTickerStateChange (ticker);
    }
    OnBnClickedUnselectQuotes ();
}


void IB_PTraderDlg::OnBnClickedUnselectQuotes ()
{
    _mainlistbox.SetCurSel (-1);
    OnLbnSelchangeQuoteslist ();
}


void IB_PTraderDlg::OnBnClickedUnselectMessages ()
{
    _statusmessageslistbox.SetCurSel (-1);
}


void IB_PTraderDlg::OnBnClickedAvgAction ()
{
    Ticker *ticker = GetSelectedTicker ();
    if (ticker)
    {
        if (ticker->_buyaverages == Ticker::Always)
        {
            ticker->_buyaverages = Ticker::OnlyIfBelow;
        }
        else
        {
            ticker->_buyaverages = Ticker::Always;
        }
        Set_DoBuys_DoSells_OnTickerStateChange (ticker);
    }
    SetBuyAverageButtonText (ticker);
}

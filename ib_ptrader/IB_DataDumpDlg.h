/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include "afxwin.h"


// IB_DataDumpDlg dialog

class IB_DataDumpDlg : public CDialog
{
    DECLARE_DYNAMIC(IB_DataDumpDlg)

public:
    IB_DataDumpDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~IB_DataDumpDlg();

// Dialog Data
    enum { IDD = IDD_IB_DATADUMP };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();

private:
    void DoFileDump (CString filename);

public:
    IB_PTraderDlg *_parent;

private:
    CListBox _datalistbox;
};

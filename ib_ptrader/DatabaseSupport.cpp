/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "mysql_connection.h"
#include <cppconn/exception.h>



bool IB_PTraderDlg::ConnectToDatabase ()
{
    try 
    {
        Log ("Connecting to MySQLServer...");
        Log ("User: " + _dbusername + ", Pwd: " + _dbpwd + ", HostName: " + _dbhostname);
        _dbdriver = get_driver_instance ();
        _dbcon = _dbdriver->connect ((LPCTSTR) _dbhostname, (LPCTSTR) _dbusername, (LPCTSTR) _dbpwd);
        _dbcon->setSchema ((LPCTSTR) _dbschema);

        Log ("Connected to " + _dbschema);
    }
    catch (std::exception &e) 
    {
        Log ("ConnectToDatabase - exception thrown");
        Log (e.what ());
        _stats.IncrementStat ("ConnectToDatabase", e.what (), "", "");
        return false;
    }
    catch (...)
    {
        Log ("ConnectToDatabase - exception thrown");
        _stats.IncrementStat ("ConnectToDatabase", "exception ...", "", "");
        return false;
    }
    return _dbcon != 0;
}


void IB_PTraderDlg::DisconnectFromDatabase ()
{
    Log ("Disconnecting from MySQLServer");
    delete _dbcon;
}
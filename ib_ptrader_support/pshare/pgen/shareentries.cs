using System;
using System.Collections.Generic;
using System.Text;
using MySql.Data.MySqlClient;
using MySql.Data.Types;



namespace pshare
{
    partial class pshare
    {
        class ShareEntry
        {
            // POD
            public string _tickerid;
            public string _ticker;
            public string _displayname;
            public string _primaryexchange;
            public string _sectype;
            public string _minoi;
            public string _maxoi;
            public string _maxvalue;
            public string _pgenfilename = null;
            public string _username;
            public string _accountnumber;

            override public string ToString ()
            {
                return "TickerId=" + _tickerid + ", Ticker=" + _ticker + " (" + _primaryexchange + "), " + _displayname + 
                       (_pgenfilename != null ? " (" + _pgenfilename + ")" : "") + ", MinOI=" + _minoi + ", MaxOI=" + _maxoi + " (" + _maxvalue + ") for " + _username;
            }

            public ShareEntry (string ticker, string displayname, string primaryexchange, string sectype, 
                               string minoi, string maxoi, string maxvalue, string username, string accountnumber)
            {
                _ticker = ticker;
                _displayname = displayname;
                _primaryexchange = primaryexchange;
                _sectype = sectype;
                _minoi = minoi.Length > 0 ? minoi : "0";
                _maxoi = maxoi;
                _maxvalue = maxvalue;
                _username = username;
                _accountnumber = accountnumber;
            }
        }

        class ShareEntries
        {
            const long INITIAL_TICKERID = 0;

            private long _currenttickerid = INITIAL_TICKERID;
            internal Dictionary <string, ShareEntry> _shareentries = new Dictionary <string, ShareEntry> ();
            private Dictionary <string, ShareEntry> _shareentries_byticker = new Dictionary <string, ShareEntry> ();
            internal List <ShareEntry> _linearshareentries = new List <ShareEntry> ();
            internal List <ShareEntry> _newshareentries = new List <ShareEntry> ();

            public void Reset ()
            {
                _currenttickerid = INITIAL_TICKERID;
                _shareentries.Clear ();
                _shareentries_byticker.Clear ();
                _linearshareentries.Clear ();
                _newshareentries.Clear ();
            }

            public bool AddShareEntry (ShareEntry entry, bool assigntickerid)
            {
                if (_shareentries_byticker.ContainsKey (entry._ticker))
                {
                    entry._tickerid = _shareentries_byticker [entry._ticker]._tickerid;
                    return true; 
                }
                string tickerid;
                if (assigntickerid)
                {
                    tickerid = _currenttickerid.ToString ();
                    entry._tickerid = tickerid;
                    _currenttickerid++;
                }
                else
                {
                    tickerid = entry._tickerid;
                    _currenttickerid = long.Parse (entry._tickerid) + 1;
                }

                _shareentries [tickerid] = entry;
                _shareentries_byticker [entry._ticker] = entry;
                _linearshareentries.Add (entry);
                if (assigntickerid)
                    _newshareentries.Add (entry);
                return false;
            }

            public bool AddShareEntry (string tickerid, string ticker, string displayname, string primaryexchange, string sectype, string minoi, string maxoi, string maxvalue, string username, string accountnumber)
            {
                return AddShareEntry (new ShareEntry (ticker, displayname, primaryexchange, sectype, minoi, maxoi, maxvalue, username, accountnumber) { _tickerid = tickerid }, false);
            }
        }

        ShareEntries _shareentries = new ShareEntries ();
        
        void AddShares (params ShareEntry [] entries)
        {
            foreach (ShareEntry entry in entries)
            {
                _shareentries.AddShareEntry (entry, true);
            }
        }

        void ReadDatabase_shares (pcmdsupport.ParseAccountsCMDLine account)
        {
            /*
                ibrokers.shares (Table)
                ========================
                idshares: INT UNSIGNED
                tickerid: TEXT
                ticker: TEXT
                displayname: TEXT
                primaryexchange: TEXT
                sectype: TEXT
                minoi: TEXT
                maxoi: TEXT
                maxvalue: TEXT
                username: TEXT
                accountnumber: TEXT
            */
            string username = account.UserName;
            string accountnumber = account.AccountNumber;
            StringBuilder sb = new StringBuilder ("select tickerid, ticker, displayname, primaryexchange, sectype, minoi, maxoi, shares.maxvalue, username, accountnumber from shares where ");
            sb.Append (" username = '");
            sb.Append (username);
            sb.Append ("' and accountnumber = '");
            sb.Append (accountnumber);
            sb.Append ("'");
            MySqlCommand mysqlcommand = new MySqlCommand (sb.ToString (), _mysqlconnection);
            mysqlcommand.CommandTimeout = dbconnection.DBConnectionManager.DATABASE_TIMEOUT;
            MySqlDataReader mysqlreader = mysqlcommand.ExecuteReader ();
            while (mysqlreader.Read ()) 
            {
                _shareentries.AddShareEntry (mysqlreader.GetString (0), mysqlreader.GetString (1), mysqlreader.GetString (2),
                                             mysqlreader.GetString (3), mysqlreader.GetString (4), mysqlreader.GetString (5), 
                                             mysqlreader.GetString (6), mysqlreader.GetString (7), mysqlreader.GetString (8), 
                                             mysqlreader.GetString (9));
            }
            mysqlreader.Close ();
        }

        void UpdateDatabase_shares (pcmdsupport.ParseAccountsCMDLine account)
        {
            /*
                ibrokers.shares (Table)
                ========================
                idshares: INT UNSIGNED
                tickerid: TEXT
                ticker: TEXT
                displayname: TEXT
                primaryexchange: TEXT
                sectype: TEXT
                minoi: TEXT
                maxoi: TEXT
                maxvalue: TEXT
                username: TEXT
                accountnumber: TEXT
            */
            if (_shareentries._newshareentries.Count > 0)
            {
                MySqlTransaction mysqltransaction = null;
                MySqlCommand mysqlcommand = null;
                try
                {
                    mysqltransaction = _mysqlconnection.TheDBConnection.BeginTransaction();
                    mysqlcommand = _mysqlconnection.TheDBConnection.CreateCommand();
                    mysqlcommand.Connection = _mysqlconnection.TheDBConnection;
                    mysqlcommand.Transaction = mysqltransaction;

                    foreach (ShareEntry entry in _shareentries._newshareentries)
                    {
                        mysqlcommand.CommandText = "insert into shares (tickerid, ticker, displayname, primaryexchange, sectype, minoi, maxoi, shares.maxvalue, username, accountnumber) VALUES (?field1, ?field2, ?field3, ?field4, ?field5, ?field6, ?field7, ?field8, ?field9, ?field10)";
                        mysqlcommand.Parameters.Clear ();
                        mysqlcommand.Parameters.AddWithValue ("?field1", entry._tickerid);
                        mysqlcommand.Parameters.AddWithValue ("?field2", entry._ticker);
                        mysqlcommand.Parameters.AddWithValue ("?field3", entry._displayname);
                        mysqlcommand.Parameters.AddWithValue ("?field4", entry._primaryexchange);
                        mysqlcommand.Parameters.AddWithValue ("?field5", entry._sectype);
                        mysqlcommand.Parameters.AddWithValue ("?field6", entry._minoi);
                        mysqlcommand.Parameters.AddWithValue ("?field7", entry._maxoi);
                        mysqlcommand.Parameters.AddWithValue ("?field8", entry._maxvalue);
                        mysqlcommand.Parameters.AddWithValue ("?field9", entry._username);
                        mysqlcommand.Parameters.AddWithValue ("?field10", entry._accountnumber);
                        mysqlcommand.ExecuteNonQuery ();
                    }

                    mysqltransaction.Commit ();
                }   
                catch (Exception e)
                {
                    try
                    {
                        if (mysqltransaction != null)
                            mysqltransaction.Rollback ();
                    }
                    catch
                    {
                    }
                    if (e is System.Threading.ThreadAbortException)
                        throw;
                }                
            }
        }

        void AddShares (pcmdsupport.ParseAccountsCMDLine account, bool updateshares)
        {
            _shareentries.Reset ();

            ReadDatabase_shares (account);

            if (updateshares)
            {
                UpdateDatabase_shares (account);
            }
        }
    }
}

using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Text;
using MySql.Data.MySqlClient;
using MySql.Data.Types;



namespace dbconnection
{
    public class DBConnection
    {
        #region "Declarations"
        private string _connectionstring;
        private MySqlConnection _dbconnection;

        public MySqlConnection TheDBConnection
        {
            get
            {
                return _dbconnection;
            }
        }
        #endregion

        #region "Implicit Conversion"
        static public implicit operator MySqlConnection (DBConnection dbconnection)
        {
            return dbconnection != null ? dbconnection._dbconnection : null;
        }
        #endregion

        #region "Construction"
        public DBConnection (string connectionstring, MySqlConnection dbconnection)
        {
            _connectionstring = connectionstring;
            _dbconnection = dbconnection;
        }
        #endregion
    }

    public class DBConnectionManager
    {
        #region "Constants"
        public const int DATABASE_TIMEOUT = 1000;
        #endregion

        #region "Declarations"
        private Dictionary <string, DBConnection> _dbconnections = new Dictionary <string, DBConnection> ();
        #endregion

        #region "Methods"
        #region "Helpers"
        static public string GetDatabaseConnectionString (string username, string password, string database, string hostname)
        {
            return "datasource=" + hostname + ";username=" + username + ";password=" + password + ";database=" + database;
        }

        static public MySqlConnection GetDatabaseConnection (string connectionname)
        {
            try
            {
                return new MySqlConnection (connectionname);
            }
            catch (System.Threading.ThreadAbortException)
            {
                throw;
            }
            catch (Exception)
            {
            }
            return null;
        }

        static public MySqlConnection GetDatabaseConnection (string connectionname, out Exception exception)
        {
            try
            {
                exception = null;
                return new MySqlConnection (connectionname);
            }
            catch (System.Threading.ThreadAbortException)
            {
                throw;
            }
            catch (Exception e)
            {
                exception = e;
            }
            return null;
        }
        #endregion

        public DBConnection GetDBConnection_ConnectionStr (string dbconnectionstr)
        {
            DBConnection dbconnection;
            if (_dbconnections.TryGetValue (dbconnectionstr, out dbconnection))
                return dbconnection;
            Exception exception;
            MySqlConnection mysqlconnection = GetDatabaseConnection (dbconnectionstr, out exception);
            if (exception != null)
                throw exception;
            mysqlconnection.Open ();
            dbconnection = new DBConnection (dbconnectionstr, mysqlconnection);
            _dbconnections [dbconnectionstr] = dbconnection;
            return dbconnection;
        }

        public DBConnection GetDBConnection (string username, string password, string database, string hostname)
        {
            return GetDBConnection_ConnectionStr (GetDatabaseConnectionString (username, password, database, hostname));
        }

        public DBConnection GetDBConnectionFromCommaSeparated (string commaseparated)
        {
            string [] splits = commaseparated.Split (',');
            if (splits.Length == 4)
            {
                return GetDBConnection (splits [0], splits [1], splits [2], splits [3]);
            }
            return null;
        }

        public void CloseConnections ()
        {
            foreach (KeyValuePair <string, DBConnection> kvp in _dbconnections)
            {
                try
                {
                    kvp.Value.TheDBConnection.Close();
                }
                catch (System.Threading.ThreadAbortException)
                {
                    throw;
                }
                catch
                {
                }
            }
        }
        #endregion
    }
}

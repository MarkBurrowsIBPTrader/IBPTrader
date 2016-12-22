using System;
using System.Collections.Generic;
using System.Text;



namespace pcmdsupport
{
    public class ParseDBCMDLine
    {
        private bool _inerror = true;
        private string _schema, _username, _pwd, _hostname;

        public bool InError { get { return _inerror; } }
        public string Schema { get { return _schema; } }
        public string UserName { get { return _username; } }
        public string Pwd { get { return _pwd; } }
        public string Hostname { get { return _hostname; } }

        static public string SupportedFormatString ()
        {
            return "[dbschema,dbusername,dbpwd,dbhostname]";
        }
        
        public ParseDBCMDLine (string input)
        {
            // format is [dbschema,dbusername,dbpwd,dbhostname]
            if (input.Length > 5 && input [0] == '[' && input [input.Length - 1] == ']')
            {
                input = input.Substring (1, input.Length - 2);
                string [] splits = input.Split (',');
                if (splits.Length == 4)
                {
                    _schema = splits [0];
                    _username = splits [1];
                    _pwd = splits [2];
                    _hostname = splits [3];
                    _inerror = false;
                }
            }
        }
    }

    public class ParseAccountsCMDLine
    {
        private bool _inerror = true;
        private string _username, _accountnumber;

        public bool InError { get { return _inerror; } }
        public string UserName { get { return _username; } }
        public string AccountNumber { get { return _accountnumber; } }

        static public string SupportedFormatString ()
        {
            return "[twsusername,twsaccountnumber]";
        }

        public ParseAccountsCMDLine (string input)
        {
            // format is [twsusername,twsaccountnumber]
            if (input.Length > 3 && input [0] == '[' && input [input.Length - 1] == ']')
            {
                input = input.Substring (1, input.Length - 2);
                string [] splits = input.Split (',');
                if (splits.Length == 2)
                {
                    _username = splits [0];
                    _accountnumber = splits [1];
                    _inerror = false;
                }
            }
        }
    }
}

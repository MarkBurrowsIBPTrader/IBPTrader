/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#pragma once

#include <vector>
#if ! PUBLIC_BUILD
#include <boost/circular_buffer.hpp>
#endif



class Branch
{
private:
    double _lowervalue;
    double _uppervalue;
    int _start;
    int _finish;

    template <class T> friend class BSearch;

public:
    inline bool ValueInRange (double value)
    {
        return value >= _lowervalue && value <= _uppervalue;
    }

    inline bool Greater (double value)
    {
        return value > _uppervalue;
    }

    inline bool LessThan (double value)
    {   
        return value < _lowervalue;
    }
};


enum BSearchStartResult
{
    LINEAR, SUBSET
};


template <class T> class BSearch
{
private:
    Branch *CreateBranch ();

    template <class T> class InternalNode
    {
    public:
        T *_data;
        double _datavalue;
    };
 
    template <class T> static bool CompareNodes (const InternalNode <T> *d1, const InternalNode <T> *d2)
    {
        return d1->_datavalue < d2->_datavalue;
    }

#if ! PUBLIC_BUILD
    class SearchStats
    {
    private:
        boost::circular_buffer <double> _linearfails;
        CTime _lastlinearfailtime;

    public:
        unsigned int _number_searches;
        unsigned int _number_cachehits;
        unsigned int _number_btreematches;
        unsigned int _number_linearmatches;

        void IncrementLinearMatch (double value)
        {
            _number_linearmatches++;
            _linearfails.push_back (value);
            _lastlinearfailtime = CTime::GetCurrentTime ();
        }

        void Format (const CString &prefix, CString &str)
        {
            str.Format ("%s searches %u, Cache Hits %u, BTree %u, Linear %u, Cache %0.2f%%", 
                        (LPCSTR) prefix,
                        _number_searches,
                        _number_cachehits,
                        _number_btreematches,
                        _number_linearmatches,
                        _number_searches == 0 ? 0.0 :
                        ((double) _number_cachehits / (double) _number_searches) * 100.0);
            if (_number_linearmatches > 0 && ! _linearfails.empty ())
            {
                CString suffix;
                str += ", linears (";
                for (size_t i = 0;  i < _linearfails.size ();  i++)
                {
                    if (i > 0)
                        str += ", ";
                    suffix.Format ("%0.2f", _linearfails [i]);
                    str += suffix;
                }
                str += ") - Time ";
                suffix.Format ("%0.2d:%0.2d:%0.2d", _lastlinearfailtime.GetHour (), _lastlinearfailtime.GetMinute (), _lastlinearfailtime.GetSecond ());
                str += suffix;
            }
        }

        SearchStats () : _linearfails (5), _number_searches (0), _number_cachehits (0), 
                         _number_btreematches (0), _number_linearmatches (0) {}
    };

    friend class Stats;
#endif

private:
    std::vector <InternalNode <T> *> _data;
    std::vector <Branch *> _branches;
    Branch *_mru;
#if ! PUBLIC_BUILD
    SearchStats _searchstats;
#endif

    template <class T> class IndexFunctor
    {
    private:
        int _currentindex;
    
    public:
        void operator () (InternalNode <T> *node)
        {
            node->_data->_pairindex = _currentindex++;
        }

        IndexFunctor () : _currentindex (0) {}
    };

public:
    static std::string ToString (BSearchStartResult result)
    {
        switch (result)
        {
            case LINEAR: { return "LINEAR"; }
            case SUBSET: { return "SUBSET"; }
        }
        return "???";
    }

    static std::string ToStringDisplay (BSearchStartResult result)
    {
        switch (result)
        {
            case LINEAR: { return "Lin"; }
            case SUBSET: { return "Sub"; }
        }
        return "???";
    }

private:
    BSearchStartResult GetSearchStart (Branch *branch, int &searchstart, int &searchend, bool searchtoend);

public:
    inline T *GetData (int index) { return _data [index]->_data; };
    int GetDataCount () { return (int) _data.size (); };
    BSearchStartResult GetSearchStart (double value, int &searchstart, int &searchend, bool searchtoend);
    void GetStartEndDataRange (int &start, int &end);
    void Add (T *data, double value);
    void DetermineBranches ();
    
    BSearch () : _mru (0) {}
};


template <class T> BSearchStartResult BSearch <T>::GetSearchStart (Branch *branch, int &searchstart, int &searchend, bool searchtoend)
{
    _mru = branch;
    searchstart = branch->_start;
    if (searchtoend)
        searchend = GetDataCount () - 1;
    else
        searchend = branch->_finish;
    return SUBSET;
}


template <class T> BSearchStartResult BSearch <T>::GetSearchStart (double value, int &searchstart, int &searchend, bool searchtoend)
{
#if ! PUBLIC_BUILD
    _searchstats._number_searches++;
#endif
    if (_mru && _mru->ValueInRange (value))
    {
#if ! PUBLIC_BUILD
        _searchstats._number_cachehits++;
#endif
        return GetSearchStart (_mru, searchstart, searchend, searchtoend);
    }
    int first = 0;
    int last = (int) _branches.size () - 1;
    while (first <= last)
    {
        int mid = (first + last) / 2;
        Branch *branch = _branches [mid];
        if (branch->ValueInRange (value))
        {
#if ! PUBLIC_BUILD
            _searchstats._number_btreematches++;
#endif
            return GetSearchStart (branch, searchstart, searchend, searchtoend);
        }
        else if (branch->Greater (value))
        {
            first = mid + 1;
        }
        else
        {
            last = mid - 1;
        }
    }
#if ! PUBLIC_BUILD
    _searchstats.IncrementLinearMatch (value);
#endif
    searchstart = 0;
    searchend = GetDataCount () - 1;
    return LINEAR;
}


template <class T> void BSearch <T>::GetStartEndDataRange (int &start, int &end)
{
    start = 0;
    end = GetDataCount () - 1;
}


template <class T> void BSearch <T>::Add (T *data, double value)
{
    InternalNode <T> *node = new InternalNode <T> ();
    node->_data = data;
    node->_datavalue = value;
    _data.push_back (node);
}


template <class T> Branch *BSearch <T>::CreateBranch ()
{
    Branch *branch = new Branch ();
    _branches.push_back (branch);
    return branch;
}


template <class T> void BSearch <T>::DetermineBranches ()
{
    std::sort (_data.begin (), _data.end (), BSearch::CompareNodes <T>);
    IndexFunctor <T> functor;
    std::for_each (_data.begin (), _data.end (), functor);

    const int MAX_SEARCH = 25;
    int current = 0;
    int maxsize = GetDataCount ();
    while (current < maxsize)
    {
        Branch *branch = CreateBranch ();
        branch->_start = current;
        current += MAX_SEARCH;
        if (current > maxsize)
            current = maxsize;
        branch->_finish = current - 1;
        branch->_lowervalue = _data [branch->_start]->_datavalue;
        branch->_uppervalue = _data [branch->_finish]->_datavalue;
    }    
}

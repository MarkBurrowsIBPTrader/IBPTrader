/****************************************************************************************************
*
* Copyright (C) 2009 - 2014 Mark Burrows (mark_burrows_999_code@outlook.com). All rights reserved
*
* This file is part of the project ib_ptrader and is subject to the terms and conditions defined in
* the file 'LICENSE.txt', which is part of this source code package
*
*****************************************************************************************************/

#include "stdafx.h"
#include "ChartViewDlg.h"



CChartViewDlg::DataPointsForGraph::DataPointsForGraph () : MAX_DOUBLEARRAY_CAPACITY (1050)
{
}


void CChartViewDlg::DataPointsForGraph::CleanUp ()
{
    for (DataPointIter iter = _datapoints.begin ();  iter != _datapoints.end ();  ++iter)
    {
        DoubleArray *doublearray = iter->second;
        delete [] doublearray->data;
        delete doublearray;
    }
    _datapoints.clear ();
}


CChartViewDlg::DataPointsForGraph::~DataPointsForGraph ()
{
}


DoubleArray *CChartViewDlg::DataPointsForGraph::GetDoubleArray (DataPointType datapointtype)
{
    DataPointIter iter = _datapoints.find (datapointtype);
    if (iter != _datapoints.end ())
    {
        return iter->second;
    }
    double *data = new double [MAX_DOUBLEARRAY_CAPACITY];
    DoubleArray *doublearray = new DoubleArray (data, 0);
    _datapoints [datapointtype] = doublearray;
    return doublearray;
}


DoubleArray *CChartViewDlg::DataPointsForGraph::AddData (DataPointType datapointtype, double newvalue)
{
    DoubleArray *doublearray = GetDoubleArray (datapointtype);
    if (doublearray)
    {
        if (doublearray->len < MAX_DOUBLEARRAY_CAPACITY)
        {
            double *buffer = const_cast <double *> (doublearray->data);
            buffer [doublearray->len++] = newvalue;
        }
    }
    return doublearray;
}
